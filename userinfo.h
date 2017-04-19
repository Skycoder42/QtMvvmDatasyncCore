#ifndef USERINFO_H
#define USERINFO_H

#include <QHostAddress>
#include <QObject>

class UserInfo
{
	Q_GADGET

	Q_PROPERTY(QString name MEMBER name CONSTANT)
	Q_PROPERTY(QHostAddress address MEMBER address STORED false CONSTANT)

public:
	QString name;
	QHostAddress address;
};

Q_DECLARE_METATYPE(UserInfo)

#endif // USERINFO_H
