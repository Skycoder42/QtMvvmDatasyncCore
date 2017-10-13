QT *= network jsonserializer datasync

HEADERS += \
	$$PWD/datasynccontrol.h \
	$$PWD/userdataexchangecontrol.h \
    $$PWD/changeremotecontrol.h

SOURCES += \
	$$PWD/datasynccontrol.cpp \
	$$PWD/userdataexchangecontrol.cpp \
    $$PWD/changeremotecontrol.cpp

TRANSLATIONS += $$PWD/qtmvvm_datasync_core_de.ts \
	$$PWD/qtmvvm_datasync_core_template.ts

INCLUDEPATH += $$PWD
