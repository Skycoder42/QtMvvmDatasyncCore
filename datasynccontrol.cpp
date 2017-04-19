#include "datasynccontrol.h"
#include <setup.h>
using namespace QtDataSync;

DatasyncControl::DatasyncControl(QObject *parent) :
	DatasyncControl(Setup::DefaultSetup, parent)
{}

DatasyncControl::DatasyncControl(const QString &setupName, QObject *parent) :
	Control(parent),
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
	_colorMap.insert(SyncController::Synced, Qt::green);
	_colorMap.insert(SyncController::SyncedWithErrors, Qt::red);
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
