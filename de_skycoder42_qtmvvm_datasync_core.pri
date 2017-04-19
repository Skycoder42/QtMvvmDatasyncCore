QT *= network jsonserializer datasync

HEADERS += \
	$$PWD/datasynccontrol.h \
	$$PWD/userdataexchangecontrol.h \
    $$PWD/userinfo.h

SOURCES += \
	$$PWD/datasynccontrol.cpp \
	$$PWD/userdataexchangecontrol.cpp \
    $$PWD/userinfo.cpp

INCLUDEPATH += $$PWD
