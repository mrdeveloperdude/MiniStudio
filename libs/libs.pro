TEMPLATE = subdirs
TARGET = libs
CONFIG += staticlib

include (../common.pri)

SUBDIRS += \
	libstyle \
	libdrumstick \
	libalsa \
	libopencl \
	libutil \


libstyle.depends=		libutil
libalsa.depends=		libdrumstick
