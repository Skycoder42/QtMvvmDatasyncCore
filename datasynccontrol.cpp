#include "datasynccontrol.h"
#include <coremessage.h>
#include <setup.h>
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

bool DatasyncControl::syncEnabled() const
{
	return _syncController->isSyncEnabled();
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

void DatasyncControl::setSyncEnabled(bool syncEnabled)
{
	_syncController->setSyncEnabled(syncEnabled);
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
