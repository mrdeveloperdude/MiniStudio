TEMPLATE = subdirs
CONFIG += ordered

include(../../common.pri)

SUBDIRS += \
	library \
#	utils \

OTHER_FILES += \
	drumstick-vpiano.supp
