#include "datasynccontrol.h"
#include <QtDataSync/Setup>
#include <QtDataSync/WsAuthenticator>
#include <coremessage.h>
#include "changeremotecontrol.h"
#include "userdataexchangecontrol.h"
using namespace QtDataSync;

DatasyncControl::DatasyncControl(QObject *parent) :
	DatasyncControl(Setup::DefaultSetup, parent)
{}

DatasyncControl::DatasyncControl(const QString &setupName, QObject *parent) :
	Control(parent),
	_setupName(setupName),
	_syncController(new SyncController(setupName, this)),
	_colorMap(),
	_currentMax(0),
	_currentValue(0)
{
	resetColorMap();
	connect(_syncController, &SyncController::syncStateChanged,
			this, &DatasyncControl::statusStringChanged);
	connect(_syncController, &SyncController::syncStateChanged,
			this, &DatasyncControl::showProgressChanged);
	connect(_syncController, &SyncController::syncOperationsChanged,
			this, &DatasyncControl::updateProgress);
	connect(_syncController, &SyncController::authenticationErrorChanged,
			this, &DatasyncControl::authErrorChanged);
}

bool DatasyncControl::syncEnabled() const
{
	return _syncController->isSyncEnabled();
}

DatasyncControl::ColorMap DatasyncControl::colorMap() const
{
	return _colorMap;
}

QString DatasyncControl::statusString() const
{
	auto state = _syncController->syncState();

	QString baseText = QStringLiteral("<font color=\"%1\">%2</font>")
					   .arg(_colorMap.value(state).name());
	switch (state) {
	case SyncController::Loading:
		return baseText.arg(tr("Loading…"));
	case SyncController::Disconnected:
		return baseText.arg(tr("Disconnected"));
	case SyncController::Syncing:
		return baseText.arg(tr("Synchronizing…"));
	case SyncController::Synced:
		return baseText.arg(tr("Synchronized"));
	case SyncController::SyncedWithErrors:
		return baseText.arg(tr("Synced with errors!"));
	default:
		Q_UNREACHABLE();
		return QString();
	}
}

bool DatasyncControl::showProgress() const
{
	return _syncController->syncState() == SyncController::Syncing;
}

double DatasyncControl::syncProgress() const
{
	return (double)_currentValue/(double)_currentMax;
}

QString DatasyncControl::authError() const
{
	auto error = _syncController->authenticationError();
	if(error.isNull())
		return QString();
	else
		return tr("Error: %1").arg(error);
}

bool DatasyncControl::canReset()
{
	auto canReset = false;
	auto auth = Setup::authenticatorForSetup<Authenticator>(this, _setupName);
	if(qobject_cast<WsAuthenticator*>(auth))
		canReset = true;
	auth->deleteLater();
	return canReset;
}

void DatasyncControl::sync()
{
	_syncController->triggerSync();
}

void DatasyncControl::resync()
{
	_syncController->triggerResync();
}

void DatasyncControl::exportUserData(QIODevice *device)
{
	auto auth = Setup::authenticatorForSetup<Authenticator>(this, _setupName);
	auth->exportUserData(device);
	auth->deleteLater();
	device->close();
	device->deleteLater();
}

void DatasyncControl::importUserData(QIODevice *device)
{
	auto auth = Setup::authenticatorForSetup<Authenticator>(this, _setupName);
	auth->importUserData(device).onResult(this, [auth, device](){
		auth->deleteLater();
		device->close();
		device->deleteLater();
		CoreMessage::information(tr("User data import"), tr("Import successfully completed!"));
	}, [auth, device](const QException &e){
		auth->deleteLater();
		device->close();
		device->deleteLater();
		CoreMessage::critical(tr("User data import"), tr("Import failed with error: %1").arg(QString::fromUtf8(e.what())));
	});
}

void DatasyncControl::initExchange()
{
	auto control = new UserDataExchangeControl(_setupName, this);
	control->setDeleteOnClose(true);
	control->show();
}

void DatasyncControl::changeRemote()
{
	auto control = new ChangeRemoteControl(_setupName, this);
	if(control->isValid()) {
		control->setDeleteOnClose(true);
		control->show();
	} else
		control->deleteLater();
}

void DatasyncControl::resetIdentity()
{
	auto auth = Setup::authenticatorForSetup<Authenticator>(this, _setupName);
	auto wsauth = qobject_cast<WsAuthenticator*>(auth);
	if(wsauth) {
		CoreApp::MessageConfig message;
		message.title = tr("Reset Identity");
		message.text = tr("Do you want to keep your local data, or delete everything?<br/>"
						  "<b>Warning:</b> Deleting your data cannot be undone. If no other device is connected "
						  "to this identity, the data is lost permanently!");
		message.type = CoreApp::Question;
		message.positiveAction = tr("Delete Data");
		message.neutralAction = tr("Keep Data");
		message.negativeAction = tr("Cancel");
		auto result = CoreMessage::message(message);
		if(result) {
			result->setAutoDelete(true);
			QObject::connect(result, &MessageResult::anyAction,
							 this, [wsauth, this](MessageResult::ResultType type) {
				GenericTask<void> task;
				switch (type) {
				case MessageResult::PositiveResult:
					task = wsauth->resetUserData(true);
					break;
				case MessageResult::NeutralResult:
					task = wsauth->resetUserData(false);
					break;
				case MessageResult::NegativeResult:
					wsauth->deleteLater();
					return;
				default:
					Q_UNREACHABLE();
					break;
				}

				task.onResult(this, [](){
					CoreMessage::information(tr("Reset Identity"),
											 tr("Identity successfully resetted!"));
				}, [](const QException &e){
					CoreMessage::warning(tr("Reset Identity"),
										 tr("Failed to reset Identity with error: %1")
										 .arg(QString::fromUtf8(e.what())));
				});
			}, Qt::QueuedConnection);
		}
	} else
		auth->deleteLater();
}

void DatasyncControl::setSyncEnabled(bool syncEnabled)
{
	_syncController->setSyncEnabled(syncEnabled);
}

void DatasyncControl::setColorMap(DatasyncControl::ColorMap colorMap)
{
	if (_colorMap == colorMap)
		return;

	_colorMap = colorMap;
	emit colorMapChanged(colorMap);
}

void DatasyncControl::resetColorMap()
{
	_colorMap.clear();
	_colorMap.insert(SyncController::Loading, Qt::darkCyan);
	_colorMap.insert(SyncController::Disconnected, Qt::darkYellow);
	_colorMap.insert(SyncController::Syncing, Qt::darkCyan);
	_colorMap.insert(SyncController::Synced, Qt::darkGreen);
	_colorMap.insert(SyncController::SyncedWithErrors, Qt::darkRed);
}

void DatasyncControl::updateProgress(int taskCount)
{
	if(taskCount == 0) {
		_currentMax = 1;
		_currentValue = 1;
	} else {
		_currentMax = qMax(taskCount, _currentMax);
		_currentValue = _currentMax - taskCount;
	}
	emit syncProgressChanged();
}
