TEMPLATE = lib
TARGET = style
CONFIG += staticlib

include (../../common.pri)

SOURCES += \
	OctoStyle.cpp \
	OctoStylePlugin.cpp \



HEADERS += \
	OctoStyle.hpp \
	OctoStyleAnimations.hpp \
	OctoStylePrivate.hpp \



RESOURCES += \
	OctoStyle.qrc \
