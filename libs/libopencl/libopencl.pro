TARGET = opencl
TEMPLATE = lib

include(../../common.pri)
QT += multimedia-private


SOURCES += \
	CLVideoFilter.cpp \
	GLSLVideoFilter.cpp \

HEADERS += \
	CLVideoFilter.hpp \
	GLSLVideoFilter.hpp \
	

include(OpenCL.pri)

