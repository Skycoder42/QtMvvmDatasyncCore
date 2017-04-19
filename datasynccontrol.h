#ifndef DATASYNCCONTROL_H
#define DATASYNCCONTROL_H

#include <QObject>
#include <QMap>
#include <QColor>
#include <control.h>
#include <synccontroller.h>

class DatasyncControl : public Control
{
	Q_OBJECT

	Q_PROPERTY(bool syncEnabled READ syncEnabled WRITE setSyncEnabled NOTIFY syncEnabledChanged)

	Q_PROPERTY(ColorMap colorMap READ colorMap WRITE setColorMap RESET resetColorMap NOTIFY colorMapChanged)
	Q_PROPERTY(QString statusString READ statusString NOTIFY statusStringChanged)

	Q_PROPERTY(bool showProgress READ showProgress NOTIFY showProgressChanged)
	Q_PROPERTY(double syncProgress READ syncProgress NOTIFY syncProgressChanged)

public:
	typedef QMap<QtDataSync::SyncController::SyncState, QColor> ColorMap;

	explicit DatasyncControl(QObject *parent = nullptr);
	explicit DatasyncControl(const QString &setupName, QObject *parent = nullptr);

	ColorMap colorMap() const;
	QString statusString() const;
	bool showProgress() const;
	double syncProgress() const;

	bool syncEnabled() const;

public slots:
	void sync();
	void resync();

	void setColorMap(ColorMap colorMap);
	void resetColorMap();

	void setSyncEnabled(bool syncEnabled);

signals:
	void syncEnabledChanged(bool syncEnabled);
	void colorMapChanged(ColorMap colorMap);
	void statusStringChanged();
	void showProgressChanged();
	void syncProgressChanged();

private slots:
	void updateProgress(int taskCount);

private:
	QtDataSync::SyncController *_syncController;
	ColorMap _colorMap;
	int _currentMax;
	int _currentValue;
};

#endif // DATASYNCCONTROL_H
