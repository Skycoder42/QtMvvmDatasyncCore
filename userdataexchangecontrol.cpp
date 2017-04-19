#include "userdataexchangecontrol.h"
#include "userinfodatagram.h"
#include <setup.h>
#include <coremessage.h>
#include <QBuffer>
using namespace QtDataSync;

UserDataExchangeControl::UserDataExchangeControl(const QString &setupName, QObject *parent) :
	Control(parent),
	_authenticator(Setup::authenticatorForSetup<Authenticator>(this, setupName)),
	_serializer(new QJsonSerializer(this)),
	_socket(new QUdpSocket(this)),
	_timer(new QTimer(this)),
	_model(new QGadgetListModel<UserInfo>(this))
{
	connect(_socket, &QUdpSocket::readyRead,
			this, &UserDataExchangeControl::newData);
	setPort(4224);

	connect(_timer, &QTimer::timeout,
			this, &UserDataExchangeControl::timeout);
	_timer->setTimerType(Qt::VeryCoarseTimer);
	_timer->setInterval(2000);
	_timer->start();
}

quint16 UserDataExchangeControl::port() const
{
	return _socket->localPort();
}

QString UserDataExchangeControl::deviceName() const
{
	return _deviceName;
}

QAbstractListModel *UserDataExchangeControl::model() const
{
	return _model;
}

void UserDataExchangeControl::exportTo(const QModelIndex &index)
{
	auto info = _model->gadget(index);
	CoreMessage::getInput(tr("Send user data"),
						  tr("Enter a key to protect your data with before sending (optional):"),
						  QMetaType::QString,
						  [=](QVariant key) {
		sendData(info, key.toString());
	});
}

void UserDataExchangeControl::setPort(quint16 port)
{
	if (_socket->localPort() == port)
		return;

	_socket->close();
	_model->resetModel({});
	if(_socket->bind(port, QAbstractSocket::DontShareAddress))
		emit portChanged(_socket->localPort());
	else {
		emit portChanged(0);
		CoreMessage::warning(tr("Port binding failed"), tr("Failed to bind to selected port with error: %1").arg(_socket->errorString()));
	}
}

void UserDataExchangeControl::setDeviceName(QString deviceName)
{
	if (_deviceName == deviceName)
		return;

	_deviceName = deviceName;
	emit deviceNameChanged(deviceName);

	timeout();//send name immediatly
	_timer->start();//and restart timer for normal delays
}

void UserDataExchangeControl::timeout()
{
	UserInfo info;
	info.name = _deviceName;
	UserInfoDatagram uiData;
	uiData.type = UserInfoDatagram::UserInfo;
	uiData.data = _serializer->serialize(info);

	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);
	_serializer->serializeTo(&buffer, uiData);//TODO use real after update
	_socket->writeDatagram(buffer.data(), QHostAddress::Broadcast, _socket->localPort());
}

void UserDataExchangeControl::newData()
{
	while(_socket->hasPendingDatagrams()) {
		auto datagram = _socket->receiveDatagram();
		qDebug() << datagram.data();
		if(!datagram.isValid())
			continue;

		QBuffer buffer;
		buffer.setData(datagram.data());
		buffer.open(QIODevice::ReadOnly);
		auto message = _serializer->deserializeFrom(&buffer, qMetaTypeId<UserInfoDatagram>()).value<UserInfoDatagram>();//TODO real use after update
		switch (message.type) {
		case UserInfoDatagram::UserInfo:
		{
			auto userInfo = _serializer->deserialize<UserInfo>(message.data);
			userInfo.datagram = datagram;
			auto wasFound = false;
			for(auto i = 0; i < _model->rowCount(); i++) {
				auto info = _model->gadget(i);
				if(info.address() == userInfo.address()) {
					wasFound = true;
					if(info.name != userInfo.name) {
						_model->removeGadget(i);
						_model->insertGadget(i, userInfo);
					}
					break;
				}
			}
			if(!wasFound)
				_model->addGadget(userInfo);
			break;
		}
		case UserInfoDatagram::UserData:
		{
			//TODO
			break;
		}
		default:
			Q_UNREACHABLE();
			break;
		}
	}
}

void UserDataExchangeControl::sendData(const UserInfo &user, const QString &key)
{
	//TODO export via socket
}
