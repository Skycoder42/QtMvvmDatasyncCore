#include "userdataexchangecontrol.h"
#include <setup.h>
#include <coremessage.h>
using namespace QtDataSync;

UserDataExchangeControl::UserDataExchangeControl(const QString &setupName, QObject *parent) :
	Control(parent),
	_authenticator(Setup::authenticatorForSetup<Authenticator>(this, setupName)),
	_serializer(new QJsonSerializer(this)),
	_socket(new QUdpSocket(this)),
	_timer(new QTimer(this)),
	_model(new QGadgetListModel<UserInfo>(this))
{
	_socket->bind(4224, QAbstractSocket::DontShareAddress);//TODO verify
	//TODO receive data

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
	//TODO send info via socket
}

void UserDataExchangeControl::sendData(const UserInfo &user, const QString &key)
{
	//TODO export via socket
}
