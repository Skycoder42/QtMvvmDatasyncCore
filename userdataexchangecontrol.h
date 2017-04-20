#ifndef USERDATAEXCHANGECONTROL_H
#define USERDATAEXCHANGECONTROL_H

#include <control.h>
#include <QUdpSocket>
#include <QJsonSerializer>
#include <authenticator.h>
#include <QTimer>
#include <qgadgetlistmodel.h>

#include "userinfo.h"
#include "userinfodatagram.h"

class UserDataExchangeControl : public Control
{
	Q_OBJECT

	Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
	Q_PROPERTY(QString deviceName READ deviceName WRITE setDeviceName NOTIFY deviceNameChanged)

	Q_PROPERTY(QAbstractListModel* model READ model CONSTANT)

public:
	explicit UserDataExchangeControl(const QString &setupName, QObject *parent = nullptr);

	quint16 port() const;
	QString deviceName() const;
	QAbstractListModel *model() const;

public slots:
	void exportTo(const QModelIndex &index);

	void setPort(quint16 port);
	void setDeviceName(QString deviceName);

signals:
	void portChanged(quint16 port);
	void deviceNameChanged(QString deviceName);

private slots:
	void timeout();
	void newData();

private:
	QtDataSync::Authenticator *_authenticator;
	QJsonSerializer *_serializer;
	QUdpSocket *_socket;
	QTimer *_timer;

	QGadgetListModel<UserInfo> *_model;

	QString _deviceName;

	void sendData(const UserInfo &user, const QString &key);
	void receiveUserInfo(const UserInfoDatagram &message, const QNetworkDatagram &datagram);
	void receiveUserData(const UserInfoDatagram &message);
};

#endif // USERDATAEXCHANGECONTROL_H
