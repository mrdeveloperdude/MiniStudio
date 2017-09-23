#include "LiveThread.hpp"

#include "FrameScene.hpp"
#include "CameraGrabber.hpp"

#include <QScreen>
#include <QGuiApplication>
#include <QWindow>
#include <QThreadPool>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QCamera>
#include <QCameraInfo>
#include <QEasingCurve>


LiveThread::LiveThread()
	: mFrameNumber(0)
	, mDone(false)
	, mLastTime(0)
	, lastCompletedFrame(0)
	, mIsSaving(false)
	, mCamera(nullptr)
	, mCameraGrabber(nullptr)
	, mLastCameraOpacity(1.0)
	, mMagLevel(1.0)
	, mPIPSize(1.0)
	, mScreenSwitch()
	, mMagSwitch(QEasingCurve::OutBack, QEasingCurve::OutQuad, 100.0, 500.0)
	, mCameraSwitch()
	, mTitleSwitch(QEasingCurve::OutBounce, QEasingCurve::OutCubic)
	, mLogoSwitch()
	, mHold(false)
{
	init();
}

LiveThread::~LiveThread()
{

	delete mCameraGrabber;
	delete mCamera;
}

void LiveThread::init()
{
	qDebug()<<"LIVE INIT";
	mCamera=new QCamera(QCameraInfo::defaultCamera());
	if(!connect(mCamera, SIGNAL(error(QCamera::Error)), this, SLOT(onCameraError(QCamera::Error)))) {
		qWarning()<<"ERROR: Could not connect camera error ";
	}

	if(!connect(mCamera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(onCameraStateChanged(QCamera::State)))) {
		qWarning()<<"ERROR: Could not connect camera state change ";
	}
	mCameraGrabber=new CameraGrabber();
	if(!connect(mCameraGrabber, &CameraGrabber::frameAvailable, this, &LiveThread::onCameraFrameReady )) {
		qWarning()<<"ERROR: Could not connect camera grabber";
	}
	mCamera->setViewfinder(mCameraGrabber);

	QCameraViewfinderSettings viewfinderSettings;
	//viewfinderSettings.setResolution(640, 480); 	viewfinderSettings.setMinimumFrameRate(0.0); 	viewfinderSettings.setMaximumFrameRate(30.0);
	mCamera->setViewfinderSettings(viewfinderSettings);
	mCamera->setCaptureMode(QCamera::CaptureVideo);
	mCamera->start();
}

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif
void qSleep(quint32 ms)
{

#ifdef Q_OS_WIN
	Sleep(uint(ms));
#else
	struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
	nanosleep(&ts, NULL);
#endif
}

void LiveThread::run()
{
	QScreen *screen = QGuiApplication::primaryScreen();
	if (nullptr==screen) {
		qDebug()<<"NO PRIMARY SCREEN IN LIVE THREAD, ABORTING";
		return;
	}

	clear();

	QSharedPointer<QImage> red(new QImage(screen->size(), QImage::Format_ARGB32)) ;
	QPainter redPaint(red.data());
	redPaint.fillRect(red->rect(),Qt::red);


	QSharedPointer<QImage> logoImage(new QImage("/home/lennart/octomy_tv_logo.png"));

	QTransform logoTrans;
	logoTrans.translate(0.9*screen->size().width()-logoImage->width(),0.1*screen->size().height());


	QTransform pipTrans;
	pipTrans.scale(0.4,0.4);
	pipTrans.translate(0.1*screen->size().width(),0.1*screen->size().height());

	QSharedPointer<QImage> magFrame(new QImage(QSize(200,200), QImage::Format_ARGB32)) ;
	magFrame->fill(0x00000000);
	QPainter magPaint(magFrame.data());
	magPaint.fillRect(magFrame->rect(),Qt::green);

	QPixmap grabPixmap;
	while(!mDone) {
		const quint64 now=QDateTime::currentMSecsSinceEpoch();
		const qint64 interval=now-mLastTime;
		QPoint mousePos = QCursor::pos();
		if(!mHold) {
			grabPixmap = screen->grabWindow(0);
		}
		if(!grabPixmap.isNull()) {
			QImage img=grabPixmap.toImage();
			QImage  *imgCopy=new QImage(img);
			QSharedPointer<QImage> screenGrab(imgCopy);
			QString framePath;
			if(mIsSaving) {
				mFrameNumber++;
				framePath=mBasePath+QString("/frame_%1.png").arg(mFrameNumber, 6, 10, QChar('0'));
				//qDebug()<<"FRAME: "<<framePath;
			}
			FrameScene *frame=new FrameScene(mFrameNumber, framePath, screenGrab->size());
			frame->addImageLayer("screen", screenGrab);
			if(!mLastCameraFrame.isNull()) {
				qreal val=mCameraSwitch.update(interval);
				if(mCameraSwitch.value()>0.0) {
					QTransform pip2(pipTrans);
					pip2.scale(mPIPSize, mPIPSize);
					QSharedPointer<QImage> camCop(new QImage(*mLastCameraFrame.data()));
					frame->addImageLayer("camera", camCop, mLastCameraOpacity*val, pip2);
				}
			}
			{
				qreal val=mMagSwitch.update(interval);
				if(mMagSwitch.value()>0.0) {
					QTransform magTrans;
					QPoint magPos(mousePos.x()-magFrame->width() / 2, mousePos.y()-magFrame->height() / 2);
					magTrans.translate(magPos.x(), magPos.y());
					//magTrans.scale(mMagLevel,mMagLevel);
					QSize magSize(magFrame->width(),magFrame->height());
					qreal magLev=1.0+(mMagLevel-1.0)*val;
					qreal mMagLevelInv=1.0 / magLev;
					QRect magSourceRect(mousePos.x() - magSize.width()*mMagLevelInv,  mousePos.y()- magSize.height()*mMagLevelInv,magSize.width()*(mMagLevelInv*2),magSize.height()*(mMagLevelInv*2));
					magPaint.setOpacity(1.0);
					magPaint.drawImage(magFrame->rect(), *screenGrab, magSourceRect);
					magPaint.setOpacity(0.2*val);
					magPaint.fillRect(magFrame->rect(), Qt::red);

					frame->addImageLayer("magnifier", magFrame, val, magTrans);
				}
			}
			{
				qreal val=mTitleSwitch.update(interval);
				if(mTitleSwitch.value()>0.0) {
					QTransform titleTrans;
					titleTrans.translate((-1.0+val)*frame->resolution().width(),0.0);
					frame->addTitleLayer("title", mCaption, mSubCaption, 1.0, titleTrans);
				}
			}
			{
				qreal val=mLogoSwitch.update(interval);
				if(mLogoSwitch.value()>0.0) {
					frame->addImageLayer("logo", logoImage, val, logoTrans);
				}
			}
			connect(frame, &FrameScene::renderComplete, this, &LiveThread::onFrameRenderComplete, (Qt::ConnectionType)(Qt::QueuedConnection | Qt::UniqueConnection));
			QThreadPool::globalInstance()->start(frame);
		} else {
			qWarning()<<"ERROR: grab failed";
		}
		mLastTime=now;
		const qint64 left=(1000.0/(screen->refreshRate()/4))-interval;
		if(left>0) {
			//	qDebug()<<"SLEEPING "<<left;
			qSleep(left);
		}
	}
	clear();
}


void LiveThread::clear()
{
	QScreen *screen = QGuiApplication::primaryScreen();
	QSharedPointer<QImage> im(new QImage(screen->size(), QImage::Format_ARGB32));
	im->fill(0xff000000);
	emit frameRendered(lastCompletedFrame+1, im);
}

void LiveThread::onFrameRenderComplete(quint64 id, QSharedPointer<QImage> im)
{
	if(! mDone && id>=lastCompletedFrame) {
		lastCompletedFrame=id;
		//qDebug()<<"live:thread complete "<<id;
		emit frameRendered(id, im);
	}
}



void LiveThread::onCameraFrameReady(QSharedPointer<QImage> im)
{
	//qDebug()<<"GOT CAM FRAME";
	mLastCameraFrame=QSharedPointer<QImage>(new QImage(*im.data()));
}


void LiveThread::onCameraError(QCamera::Error)
{
	qWarning()<<"ERROR: Camera error: "<<mCamera->errorString();
}

void LiveThread::onCameraStateChanged(QCamera::State state)
{
	qDebug()<<"Camera state changed: "<<state;
}

void LiveThread::onCameraOpacityChange(qreal opacity)
{
	//qDebug()<<"GOT CAM OPACITY "<<opacity;
	mLastCameraOpacity=opacity;

}

void LiveThread::stop()
{
	mDone=true;
}

void LiveThread::setSaving(bool saving)
{
	if(mIsSaving!=saving && saving) {
		mBasePath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
		if (mBasePath.isEmpty()) {
			mBasePath = QDir::currentPath();
		}
		mBasePath+=QString("/MiniStudio_"+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+QString((""==mProjectName)?"":("_"+mProjectName)));

		QDir initialDir(mBasePath);
		initialDir.mkpath(mBasePath);
	}
	mIsSaving=saving;
}

void LiveThread::setProjectName(QString name)
{
	mProjectName=name;
}


void LiveThread::setTitle(QString name)
{
	mCaption=name;
}


void LiveThread::setSubTitle(QString name)
{
	mSubCaption=name;
}


void LiveThread::onMagLevelChange(qreal level)
{
	mMagLevel=level;
}


void LiveThread::onMagEnabled(bool en)
{
	mMagSwitch.setEnabled(en);
}


void LiveThread::onTitleEnabled(bool en)
{
	mTitleSwitch.setEnabled(en);
}


void LiveThread::onLogoEnabled(bool en)
{
	mLogoSwitch.setEnabled(en);
}

void LiveThread::onCameraEnabled(bool en)
{
	mCameraSwitch.setEnabled(en);
}

void LiveThread::onPIPSizeChange(qreal pipSize)
{
	mPIPSize=pipSize;
}


void LiveThread::onHoldEnabled(bool hold)
{
	mHold=hold;
}
