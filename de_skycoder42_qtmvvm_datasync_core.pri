QT *= network jsonserializer datasync

HEADERS += \
	$$PWD/datasynccontrol.h \
	$$PWD/userdataexchangecontrol.h \
    $$PWD/userinfo.h \
    $$PWD/userinfodatagram.h \
    $$PWD/userdata.h

SOURCES += \
	$$PWD/datasynccontrol.cpp \
	$$PWD/userdataexchangecontrol.cpp \
    $$PWD/userinfo.cpp \
    $$PWD/userinfodatagram.cpp \
    $$PWD/userdata.cpp

QPM_TRANSLATIONS += $$PWD/qtmvvm_datasync_core_de.ts \
	$$PWD/qtmvvm_datasync_core_template.ts

INCLUDEPATH += $$PWD
