#include "changeremotecontrol.h"
#include <QtDataSync/Setup>
#include <coremessage.h>
using namespace QtDataSync;

ChangeRemoteControl::ChangeRemoteControl(const QString &setupName, QObject *parent) :
	Control(parent),
	_authenticator(Setup::authenticatorForSetup<WsAuthenticator>(this, setupName)),
	_url(),
	_changeSecret(false),
	_serverSecret(),
	_resetData(true)
{
	if(_authenticator)
		_url = _authenticator->remoteUrl();
}

bool ChangeRemoteControl::isValid() const
{
	return _authenticator;
}

QUrl ChangeRemoteControl::url() const
{
	return _url;
}

bool ChangeRemoteControl::changeSecret() const
{
	return _changeSecret;
}

QString ChangeRemoteControl::serverSecret() const
{
	return _serverSecret;
}

bool ChangeRemoteControl::resetData() const
{
	return _resetData;
}

void ChangeRemoteControl::accept()
{
	_authenticator->setRemoteUrl(_url);
	if(_changeSecret)
		_authenticator->setServerSecret(_serverSecret);
	_authenticator->resetUserData(_resetData).onResult(parent(), [](){
		CoreMessage::information(tr("Reset Identity"),
								 tr("Identity successfully resetted!"));
	}, [](const QException &e){
		CoreMessage::warning(tr("Reset Identity"),
							 tr("Failed to reset Identity with error: %1")
							 .arg(QString::fromUtf8(e.what())));
	});
}

void ChangeRemoteControl::reset()
{
	_authenticator->setRemoteUrl(QUrl());
	_authenticator->setServerSecret(QString());
	_authenticator->resetUserData(_resetData).onResult(parent(), [](){
		CoreMessage::information(tr("Remote server reset"), tr("The application must be restarted to complete the reset!"), [](){
			qApp->quit();
		});
	}, [](const QException &e){
		CoreMessage::warning(tr("Reset Identity"),
							 tr("Failed to reset Identity with error: %1")
							 .arg(QString::fromUtf8(e.what())));
	});
}

void ChangeRemoteControl::setUrl(const QUrl &url)
{
	if (_url == url)
		return;

	_url = url;
	emit urlChanged(_url);
}

void ChangeRemoteControl::setChangeSecret(bool changeSecret)
{
	if (_changeSecret == changeSecret)
		return;

	_changeSecret = changeSecret;
	emit changeSecretChanged(_changeSecret);
}

void ChangeRemoteControl::setServerSecret(const QString &serverSecret)
{
	if (_serverSecret == serverSecret)
		return;

	_serverSecret = serverSecret;
	emit serverSecretChanged(_serverSecret);
}

void ChangeRemoteControl::setResetData(bool resetData)
{
	if (_resetData == resetData)
		return;

	_resetData = resetData;
	emit resetDataChanged(_resetData);
}
