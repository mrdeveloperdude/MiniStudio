TEMPLATE = app
TARGET = ministudio

include(../common.pri)
include(../libs/libs.pri)

HEADERS += \
	AnimatedSwitch.hpp \
	CameraGrabber.hpp \
	CameraList.hpp \
	FrameScene.hpp \
	Layer.hpp \
	LiveThread.hpp \
	MiniStudio.hpp \
	PoorMansProbe.hpp \
	Presentation.hpp \
	RichEdit.hpp \
	RunGuard.hpp \
	StudioConfig.hpp \
	Tascam.hpp \
	TascamSimulator.hpp \
	widgets/LightWidget.hpp \


SOURCES += \
	AnimatedSwitch.cpp \
	CameraGrabber.cpp \
	CameraList.cpp \
	FrameScene.cpp \
	Layer.cpp \
	LiveThread.cpp \
	main.cpp \
	MiniStudio.cpp \
	PoorMansProbe.cpp \
	Presentation.cpp \
	RichEdit.cpp \
	RunGuard.cpp \
	StudioConfig.cpp \
	Tascam.cpp \
	TascamSimulator.cpp \
	widgets/LightWidget.cpp \


RESOURCES += \
	resources/icons.qrc \

FORMS += \
	ui/StudioConfig.ui \
	ui/Presentation.ui \
	ui/TascamSimulator.ui






