#ifndef USERDATAEXCHANGECONTROL_H
#define USERDATAEXCHANGECONTROL_H

#include <control.h>
#include <QtDataSync/UserDataNetworkExchange>
#include <qgadgetlistmodel.h>

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
	void exportTo(int index);

	void setPort(quint16 port, bool force = false);
	void setDeviceName(QString deviceName);

signals:
	void portChanged(quint16 port);
	void deviceNameChanged(QString deviceName);

private slots:
	void usersChanged(QList<QtDataSync::UserInfo> users);
	void userDataReceived(const QtDataSync::UserInfo &userInfo, bool secured);

private:
	QtDataSync::UserDataNetworkExchange *_exchanger;

	QGadgetListModel<QtDataSync::UserInfo> *_model;
};

#endif // USERDATAEXCHANGECONTROL_H
