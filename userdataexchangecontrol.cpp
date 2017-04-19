#include "userdataexchangecontrol.h"
#include <setup.h>
using namespace QtDataSync;

UserDataExchangeControl::UserDataExchangeControl(const QString &setupName, QObject *parent) :
	Control(parent),
	_authenticator(Setup::authenticatorForSetup<Authenticator>(this, setupName)),
	_serializer(new QJsonSerializer(this)),
	_socket(new QUdpSocket(this)),
	_timer(new QTimer(this)),
	_model(new QGadgetListModel<UserInfo>(this))
{
	_socket->bind(4224, QAbstractSocket::DontShareAddress);

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

void UserDataExchangeControl::setPort(quint16 port)
{
	if (_socket->localPort() == port)
		return;

	_socket->close();
	_socket->bind(port, QAbstractSocket::DontShareAddress);
}

void UserDataExchangeControl::setDeviceName(QString deviceName)
{
	if (_deviceName == deviceName)
		return;

	_deviceName = deviceName;
	emit deviceNameChanged(deviceName);
}

void UserDataExchangeControl::timeout()
{
	//TODO send info via socket
}
