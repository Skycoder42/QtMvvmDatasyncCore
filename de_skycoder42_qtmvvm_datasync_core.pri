QT *= network jsonserializer datasync

HEADERS += \
	$$PWD/datasynccontrol.h \
	$$PWD/userdataexchangecontrol.h

SOURCES += \
	$$PWD/datasynccontrol.cpp \
	$$PWD/userdataexchangecontrol.cpp

TRANSLATIONS += $$PWD/qtmvvm_datasync_core_de.ts \
	$$PWD/qtmvvm_datasync_core_template.ts

INCLUDEPATH += $$PWD
