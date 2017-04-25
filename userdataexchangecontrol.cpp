#include "userdata.h"
#include "userdataexchangecontrol.h"
#include "userinfodatagram.h"
#include <setup.h>
#include <coremessage.h>
#include <QBuffer>
#include <QCryptographicHash>
#include <QNetworkInterface>
#include <QSettings>
using namespace QtDataSync;

UserDataExchangeControl::UserDataExchangeControl(const QString &setupName, QObject *parent) :
	Control(parent),
	_authenticator(Setup::authenticatorForSetup<Authenticator>(this, setupName)),
	_serializer(new QJsonSerializer(this)),
	_socket(new QUdpSocket(this)),
	_timer(new QTimer(this)),
	_model(new QGadgetListModel<UserInfo>(this)),
	_deviceName()
{
	connect(_socket, &QUdpSocket::readyRead,
			this, &UserDataExchangeControl::newData);

	QSettings settings;
	settings.beginGroup(QStringLiteral("QtMvvm/DataSync"));
	_deviceName = settings.value(QStringLiteral("name"), QSysInfo::machineHostName()).toString();
	setPort((quint16)settings.value(QStringLiteral("port"), 4224).toUInt());
	settings.endGroup();

	connect(_timer, &QTimer::timeout,
			this, &UserDataExchangeControl::timeout);
	_timer->setTimerType(Qt::VeryCoarseTimer);
	_timer->setInterval(2000);
	_timer->start();
	timeout();
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

void UserDataExchangeControl::exportTo(int index)
{
	auto info = _model->gadget(index);
	CoreMessage::getInput(tr("Send user data"),
						  tr("Enter a key to protect your data with before sending (optional):"),
						  QMetaType::QString,
						  [=](QVariant key) {
		if(key.isValid())
			sendData(info, key.toString());
	},
						  QVariant(),
						  {{"echoMode", 2}});
}

void UserDataExchangeControl::setPort(quint16 port)
{
	if (_socket->localPort() == port)
		return;

	_socket->close();
	_model->resetModel({});
	if(_socket->bind(port, QAbstractSocket::DontShareAddress)) {
		emit portChanged(_socket->localPort());
		QSettings settings;
		settings.beginGroup(QStringLiteral("QtMvvm/DataSync"));
		settings.setValue(QStringLiteral("port"), _socket->localPort());
		settings.endGroup();
	} else {
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

	QSettings settings;
	settings.beginGroup(QStringLiteral("QtMvvm/DataSync"));
	settings.setValue(QStringLiteral("name"), deviceName);
	settings.endGroup();
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
		if(!datagram.isValid())
			continue;

		auto isSelf = false;
		foreach(auto addr, QNetworkInterface::allAddresses()) {
			if(addr.isEqual(datagram.senderAddress())) {
				isSelf = true;
				break;
			}
		}
		if(isSelf)
			continue;

		QBuffer buffer;
		buffer.setData(datagram.data());
		buffer.open(QIODevice::ReadOnly);
		auto message = _serializer->deserializeFrom(&buffer, qMetaTypeId<UserInfoDatagram>()).value<UserInfoDatagram>();//TODO real use after update
		switch (message.type) {
		case UserInfoDatagram::UserInfo:
			receiveUserInfo(message, datagram);
			break;
		case UserInfoDatagram::UserData:
			receiveUserData(message);
			break;
		default:
			Q_UNREACHABLE();
			break;
		}
	}
}

void UserDataExchangeControl::sendData(const UserInfo &user, const QString &key)
{
	UserData data;
	data.data = _authenticator->exportUserData();
	if(!key.isEmpty()) {
		data.secured = true;
		auto mask = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha3_512);
		while(mask.size() < data.data.size())
			mask = mask + mask;
		for(auto i = 0; i < data.data.size(); i++)
			data.data[i] = data.data[i] ^ mask[i];
	}
	data.data = data.data.toBase64();

	UserInfoDatagram uiData;
	uiData.type = UserInfoDatagram::UserData;
	uiData.data = _serializer->serialize(data);

	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);
	_serializer->serializeTo(&buffer, uiData);//TODO use real after update

	auto datagram = user.datagram.makeReply(buffer.data());
	//reset interface & sender (because of broadcast
	datagram.setInterfaceIndex(0);
	datagram.setSender(QHostAddress());
	_socket->writeDatagram(datagram);
}

void UserDataExchangeControl::receiveUserInfo(const UserInfoDatagram &message, const QNetworkDatagram &datagram)
{
	auto userInfo = _serializer->deserialize<UserInfo>(message.data);
	if(userInfo.name.isEmpty())
		userInfo.name = tr("<Unnamed>");
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
}

void UserDataExchangeControl::receiveUserData(const UserInfoDatagram &message)
{
	UserData userData = _serializer->deserialize<UserData>(message.data);
	userData.data = QByteArray::fromBase64(userData.data);

	auto importDoneHandler = [](){
		CoreMessage::information(tr("User data import"), tr("Import successfully completed!"));
	};
	auto importErrorHandler = [](const QException &e) {
		CoreMessage::critical(tr("User data import"), tr("Import failed with error: %1").arg(QString::fromUtf8(e.what())));
	};

	if(userData.secured) {
		CoreMessage::getInput(tr("Receive user data"),
							  tr("You received user data! If you want to import it, "
								 "enter the key to decrypt the received data:"),
							  QMetaType::QString,
							  [=](QVariant key) {
			if(!key.isValid())
				return;
			auto mask = QCryptographicHash::hash(key.toString().toUtf8(), QCryptographicHash::Sha3_512);
			while(mask.size() < userData.data.size())
				mask = mask + mask;
			auto data = userData.data;
			for(auto i = 0; i < data.size(); i++)
				data[i] = data[i] ^ mask[i];
			_authenticator->importUserData(data).onResult(this, importDoneHandler, importErrorHandler);
		},
							  QVariant(),
							  {{"echoMode", 2}},
							  tr("Import"));
	} else {
		CoreMessage::question(tr("Receive user data"),
							  tr("Do you want to import the received user data?"),
							  [=](bool ok){
			if(ok)
				_authenticator->importUserData(userData.data).onResult(this, importDoneHandler, importErrorHandler);
		});
	}
}
