#ifndef USERINFODATAGRAM_H
#define USERINFODATAGRAM_H

#include <QObject>
#include <QJsonObject>

class UserInfoDatagram
{
	Q_GADGET

	Q_PROPERTY(DatagramType type MEMBER type)
	Q_PROPERTY(QJsonObject data MEMBER data)

public:
	enum DatagramType {
		UserInfo,
		UserData
	};
	Q_ENUM(DatagramType)

	DatagramType type;
	QJsonObject data;
};

Q_DECLARE_METATYPE(UserInfoDatagram)

#endif // USERINFODATAGRAM_H
