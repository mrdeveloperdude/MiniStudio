TEMPLATE = lib
TARGET = util
CONFIG += staticlib


include(../../common.pri)


SOURCES += \
	utility/Standard.cpp \
	utility/BufferHoneyPot.cpp \
	utility/Utility.cpp \



HEADERS += \
	utility/Standard.hpp \
	utility/BufferHoneyPot.hpp \
	utility/Utility.hpp \
