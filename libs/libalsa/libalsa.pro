TEMPLATE = lib
TARGET = alsa
CONFIG += staticlib

include (../../common.pri)
include (../libdrumstick/global.pri)

QT -= gui
QT += dbus
CONFIG += qt thread create_prl
DEFINES += drumstick_alsa_EXPORTS RTKIT_SUPPORT
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_HIDESYMS


SOURCES += \
	alsaclient.cpp \
	alsaevent.cpp \
	alsaport.cpp \
	alsaqueue.cpp \
	alsatimer.cpp \
	playthread.cpp \
	subscription.cpp \


HEADERS += \
	alsaclient.h \
	alsaevent.h \
	alsaport.h \
	alsaqueue.h \
	alsatimer.h \
	drumstick.h \
	drumstickcommon.h \
	macros.h \
	playthread.h \
	subscription.h \

LIBS += -lasound
