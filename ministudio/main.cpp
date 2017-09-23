#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

#include <QSurfaceFormat>


#include "MiniStudio.hpp"

#include "RunGuard.hpp"


//This file is used to maintain DRY principle while using OpenGL in Qt

// This is the only place where you get to fiddle my friend!
#define OCTOMY_QT_OGL_VERSION_MAJOR 4
#define OCTOMY_QT_OGL_VERSION_MINOR 0
#define OCTOMY_QT_OGL_PROFILE Core
#define OCTOMY_QT_OGL_DEPTH_BUFFER 24
#define OCTOMY_QT_OGL_STENSIL_BUFFER 0
#define OCTOMY_QT_OGL_SWAP_INTERVAL 1



//Necessary hack. See this: http://stackoverflow.com/questions/1489932/how-to-concatenate-twice-with-the-c-preprocessor-and-expand-a-macro-as-in-arg
#define COMBINE6(A,B,C,D,E,F) A##B##C##D##E##F
#define OCTOMY_QT_OGL_FUNCTIONS_CLASS_TEMP(MAJ,MIN,PRO) COMBINE6(QOpenGLFunctions_,MAJ,_,MIN,_,PRO)

#define OCTOMY_QT_OGL_FUNCTIONS_CLASS OCTOMY_QT_OGL_FUNCTIONS_CLASS_TEMP(OCTOMY_QT_OGL_VERSION_MAJOR, OCTOMY_QT_OGL_VERSION_MINOR, OCTOMY_QT_OGL_PROFILE)

#define STRINGIFY_TEMP(A) #A
#define STRINGIFY(B) STRINGIFY_TEMP(B)

#define COMBINE2(A,B) A##B
#define TEMP_PROFILE(A,B) COMBINE2(A, B)
#define OCTOMY_QT_OGL_SURFACE_PROFILE TEMP_PROFILE(OCTOMY_QT_OGL_PROFILE, Profile)

#define OCTOMY_QT_OGL_PROFILE_A(arg)      #arg
#define OCTOMY_QT_OGL_PROFILE_B(name) OCTOMY_QT_OGL_PROFILE_A(name)
#define OCTOMY_QT_OGL_PROFILE_STR OCTOMY_QT_OGL_PROFILE_B(OCTOMY_QT_OGL_PROFILE)


#include <QOpenGLWidget>

//This right here is a result:
#include STRINGIFY(OCTOMY_QT_OGL_FUNCTIONS_CLASS)

//#include <QOpenGLFunctions>
#include <QtOpenGLExtensions/QtOpenGLExtensions>


QSurfaceFormat properOctomyDefaultFormat()
{
	QSurfaceFormat format=QSurfaceFormat::defaultFormat();
	qDebug().noquote().nospace()<<"OCTOMY_QT_OGL_VERSION: "<< OCTOMY_QT_OGL_VERSION_MAJOR << "." << OCTOMY_QT_OGL_VERSION_MINOR << " vs. ORIG: "<<format.version().first<<"."<<format.version().second;
	qDebug().noquote().nospace()<<"OCTOMY_QT_OGL_PROFILE: "<< OCTOMY_QT_OGL_PROFILE_STR << " vs. ORIG: "<<format.profile();
	qDebug().noquote().nospace()<<"OCTOMY_QT_OGL_DEPTH_BUFFER: "<< OCTOMY_QT_OGL_DEPTH_BUFFER<< " vs. ORIG: "<<format.depthBufferSize();
	qDebug().noquote().nospace()<<"OCTOMY_QT_OGL_STENSIL_BUFFER: "<< OCTOMY_QT_OGL_STENSIL_BUFFER<< " vs. ORIG: "<<format.stencilBufferSize();
	qDebug().noquote().nospace()<<"OCTOMY_QT_OGL_SWAP_INTERVAL: "<< OCTOMY_QT_OGL_SWAP_INTERVAL<< " vs. ORIG: "<<format.swapInterval();
	format.setVersion( OCTOMY_QT_OGL_VERSION_MAJOR, OCTOMY_QT_OGL_VERSION_MINOR );
	format.setProfile( QSurfaceFormat::OCTOMY_QT_OGL_SURFACE_PROFILE );
	format.setRenderableType( QSurfaceFormat::OpenGL);
	format.setOption(QSurfaceFormat::DebugContext);
	format.setDepthBufferSize(OCTOMY_QT_OGL_DEPTH_BUFFER);
	format.setStencilBufferSize(OCTOMY_QT_OGL_STENSIL_BUFFER);
	// TODO: Look at making this part of includeGL stuff
	format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
	format.setSwapInterval(OCTOMY_QT_OGL_SWAP_INTERVAL);
	return format;
}



int main(int argc, char *argv[])
{
	QString appName="MiniStudio";
	RunGuard guard(appName);
	if ( !guard.tryToRun() ){
		qDebug()<<"An MiniStudio instance is already running, quitting...";
		return 0;
	}


	QSurfaceFormat format=properOctomyDefaultFormat();
	/*
	From QSurfaceFormat::setDefaultFormat() documentation:

	Note: When setting Qt::AA_ShareOpenGLContexts, it is strongly recommended
	to place the call to this function before the construction of the
	QGuiApplication or QApplication. Otherwise format will not be applied to
	the global share context and therefore issues may arise with context sharing
	afterwards.
	*/
	QSurfaceFormat::setDefaultFormat(format);

	qRegisterMetaType<QSharedPointer<QImage> >("QSharedPointer<QImage>");
	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("OctoMY™");
	QCoreApplication::setApplicationName(appName);
	QCoreApplication::setApplicationVersion("1.0");
	app.setAttribute(Qt::AA_UseDesktopOpenGL);
	app.setAttribute(Qt::AA_ShareOpenGLContexts, true); // <-- ENABLE GL Sharing CONTEXTS
	int ret=0;
	{
		MiniStudio studio;
		app.setQuitOnLastWindowClosed(false);
		ret=app.exec();
	}
	return ret;
}
