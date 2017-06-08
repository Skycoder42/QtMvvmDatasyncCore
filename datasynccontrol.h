#ifndef DATASYNCCONTROL_H
#define DATASYNCCONTROL_H

#include <QObject>
#include <QMap>
#include <QColor>
#include <control.h>
#include <synccontroller.h>
#include <QIODevice>

class DatasyncControl : public Control
{
	Q_OBJECT

	Q_PROPERTY(bool syncEnabled READ syncEnabled WRITE setSyncEnabled NOTIFY syncEnabledChanged)

	Q_PROPERTY(ColorMap colorMap READ colorMap WRITE setColorMap RESET resetColorMap NOTIFY colorMapChanged)
	Q_PROPERTY(QString statusString READ statusString NOTIFY statusStringChanged)

	Q_PROPERTY(bool showProgress READ showProgress NOTIFY showProgressChanged)
	Q_PROPERTY(double syncProgress READ syncProgress NOTIFY syncProgressChanged)

	Q_PROPERTY(QString authError READ authError NOTIFY authErrorChanged)

	Q_PROPERTY(bool canReset READ canReset CONSTANT)

public:
	typedef QMap<QtDataSync::SyncController::SyncState, QColor> ColorMap;

	explicit DatasyncControl(QObject *parent = nullptr);
	explicit DatasyncControl(const QString &setupName, QObject *parent = nullptr);

	bool syncEnabled() const;
	ColorMap colorMap() const;
	QString statusString() const;
	bool showProgress() const;
	double syncProgress() const;
	QString authError() const;

	bool canReset();

public slots:
	void sync();
	void resync();

	void exportUserData(QIODevice *device);
	void importUserData(QIODevice *device);
	void initExchange();
	void resetIdentity();

	void setSyncEnabled(bool syncEnabled);
	void setColorMap(ColorMap colorMap);
	void resetColorMap();

signals:
	void syncEnabledChanged(bool syncEnabled);
	void colorMapChanged(ColorMap colorMap);
	void statusStringChanged();
	void showProgressChanged();
	void syncProgressChanged();
	void authErrorChanged();

private slots:
	void updateProgress(int taskCount);

private:
	const QString _setupName;
	QtDataSync::SyncController *_syncController;
	ColorMap _colorMap;
	int _currentMax;
	int _currentValue;
};

#endif // DATASYNCCONTROL_H
