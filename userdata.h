#ifndef USERDATA_H
#define USERDATA_H

#include <QObject>

class UserData
{
	Q_GADGET

	Q_PROPERTY(bool secured MEMBER secured)
	Q_PROPERTY(QByteArray data MEMBER data)

public:
	UserData();

	bool secured;
	QByteArray data;
};

#endif // USERDATA_H
