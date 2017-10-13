#include "userdataexchangecontrol.h"
#include <QtDataSync/Setup>
#include <coremessage.h>
#include <QSettings>
using namespace QtDataSync;

UserDataExchangeControl::UserDataExchangeControl(const QString &setupName, QObject *parent) :
	Control(parent),
	_exchanger(new UserDataNetworkExchange(setupName, this)),
	_model(new QGadgetListModel<UserInfo>(this))
{
	QSettings settings;
	settings.beginGroup(QStringLiteral("QtMvvm/DataSync"));
	auto port = (quint16)settings.value(QStringLiteral("port"), UserDataNetworkExchange::DataExchangePort).toUInt();
	settings.endGroup();

	connect(_exchanger, &UserDataNetworkExchange::deviceNameChanged,
			this, &UserDataExchangeControl::deviceNameChanged);
	connect(_exchanger, &UserDataNetworkExchange::usersChanged,
			this, &UserDataExchangeControl::usersChanged);
	connect(_exchanger, &UserDataNetworkExchange::userDataReceived,
			this, &UserDataExchangeControl::userDataReceived);

	setPort(port, true);
}

quint16 UserDataExchangeControl::port() const
{
	return _exchanger->port();
}

QString UserDataExchangeControl::deviceName() const
{
	return _exchanger->deviceName();
}

QAbstractListModel *UserDataExchangeControl::model() const
{
	return _model;
}

void UserDataExchangeControl::exportTo(int index)
{
	auto info = _model->gadget(index);
	CoreMessage::getInput(tr("Send user data"),
						  tr("Enter a key to protect your data with before sending (optional):"),
						  QMetaType::QString,
						  [=](QVariant key) {
								if(key.isValid())
									_exchanger->exportTo(info, key.toString());
						  },
						  QVariant(),
						  {{"echoMode", 2}});
}

void UserDataExchangeControl::setPort(quint16 port, bool force)
{
	if (!force && _exchanger->port() == port)
		return;

	_exchanger->stopExchange();
	_model->resetModel({});
	if(_exchanger->startExchange(port)) {
		QSettings settings;
		settings.beginGroup(QStringLiteral("QtMvvm/DataSync"));
		settings.setValue(QStringLiteral("port"), _exchanger->port());
		settings.endGroup();
		emit portChanged(_exchanger->port());
	} else {
		emit portChanged(0);
		CoreMessage::warning(tr("Port binding failed"), tr("Failed to bind to selected port with error: %1").arg(_exchanger->socketError()));
	}
}

void UserDataExchangeControl::setDeviceName(QString deviceName)
{
	_exchanger->setDeviceName(deviceName);
}

void UserDataExchangeControl::usersChanged(QList<UserInfo> users)
{
	foreach(auto userInfo, users) {
		auto wasFound = false;
		for(auto i = 0; i < _model->rowCount(); i++) {
			auto info = _model->gadget(i);
			if(info.address() == userInfo.address()) {
				wasFound = true;
				if(info.name() != userInfo.name()) {
					_model->removeGadget(i);
					_model->insertGadget(i, userInfo);
				}
				break;
			}
		}
		if(!wasFound)
			_model->addGadget(userInfo);
	}
}

void UserDataExchangeControl::userDataReceived(const UserInfo &userInfo, bool secured)
{
	auto importDoneHandler = [](){
		CoreMessage::information(tr("User data import"), tr("Import successfully completed!"));
	};
	auto importErrorHandler = [](const QException &e) {
		CoreMessage::critical(tr("User data import"), tr("Import failed with error: %1").arg(QString::fromUtf8(e.what())));
	};

	if(secured) {
		CoreMessage::getInput(tr("Receive user data"),
							  tr("You received user data! If you want to import it, "
								 "enter the key to decrypt the received data:"),
							  QMetaType::QString,
							  [=](QVariant key) {
									if(key.isValid())
										_exchanger->importFrom(userInfo, key.toString()).onResult(this, importDoneHandler, importErrorHandler);
								},
							  QVariant(),
							  {{"echoMode", 2}},
							  tr("Import"));
	} else {
		CoreMessage::question(tr("Receive user data"),
							  tr("Do you want to import the received user data?"),
							  [=](bool ok){
			if(ok)
				_exchanger->importFrom(userInfo).onResult(this, importDoneHandler, importErrorHandler);
		});
	}
}
