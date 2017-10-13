#ifndef CHANGEREMOTECONTROL_H
#define CHANGEREMOTECONTROL_H

#include <QObject>
#include <QUrl>
#include <QtDataSync/WsAuthenticator>
#include <control.h>

class ChangeRemoteControl : public Control
{
	Q_OBJECT

	Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
	Q_PROPERTY(bool changeSecret READ changeSecret WRITE setChangeSecret NOTIFY changeSecretChanged)
	Q_PROPERTY(QString serverSecret READ serverSecret WRITE setServerSecret NOTIFY serverSecretChanged)
	Q_PROPERTY(bool resetData READ resetData WRITE setResetData NOTIFY resetDataChanged)

public:
	explicit ChangeRemoteControl(const QString &setupName, QObject *parent = nullptr);

	bool isValid() const;

	QUrl url() const;
	bool changeSecret() const;
	QString serverSecret() const;
	bool resetData() const;

public slots:
	void accept();
	void reset();

	void setUrl(const QUrl &url);
	void setChangeSecret(bool changeSecret);
	void setServerSecret(const QString &serverSecret);
	void setResetData(bool resetData);

signals:
	void urlChanged(const QUrl &url);
	void changeSecretChanged(bool changeSecret);
	void serverSecretChanged(const QString &serverSecret);
	void resetDataChanged(bool resetData);

private:
	QtDataSync::WsAuthenticator *_authenticator;

	QUrl _url;
	bool _changeSecret;
	QString _serverSecret;
	bool _resetData;
};

#endif // CHANGEREMOTECONTROL_H
