QT *= network jsonserializer datasync

HEADERS += \
	$$PWD/datasynccontrol.h \
	$$PWD/userdataexchangecontrol.h \
    $$PWD/userinfo.h \
    $$PWD/userinfodatagram.h

SOURCES += \
	$$PWD/datasynccontrol.cpp \
	$$PWD/userdataexchangecontrol.cpp \
    $$PWD/userinfo.cpp \
    $$PWD/userinfodatagram.cpp

INCLUDEPATH += $$PWD
