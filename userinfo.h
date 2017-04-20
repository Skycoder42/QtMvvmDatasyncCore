#ifndef USERINFO_H
#define USERINFO_H

#include <QHostAddress>
#include <QObject>
#include <QNetworkDatagram>

class UserInfo
{
	Q_GADGET

	Q_PROPERTY(QString name MEMBER name)
	Q_PROPERTY(QString address READ address STORED false CONSTANT)

public:
	QString name;
	QNetworkDatagram datagram;

	UserInfo();

	QString address() const;
};

Q_DECLARE_METATYPE(UserInfo)

#endif // USERINFO_H
