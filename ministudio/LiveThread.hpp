#ifndef LIVETHREAD_HPP
#define LIVETHREAD_HPP

#include "AnimatedSwitch.hpp"

#include <QThread>
#include <QImage>
#include <QCamera>
#include <QSharedPointer>


class CameraGrabber;

class LiveThread : public QThread
{
		Q_OBJECT
	private:
		quint64 mFrameNumber;
		bool mDone;
		quint64 mLastTime;
		quint64 lastCompletedFrame;
		bool mIsSaving;
		QString mBasePath;
		QString mProjectName;
		QString mCaption;
		QString mSubCaption;
		QCamera *mCamera;
		CameraGrabber *mCameraGrabber;
		QSharedPointer <QImage> mLastCameraFrame;
		qreal mLastCameraOpacity;
		qreal mMagLevel;
		qreal mPIPSize;
		AnimatedSwitch mScreenSwitch;
		AnimatedSwitch mMagSwitch;
		AnimatedSwitch mCameraSwitch;
		AnimatedSwitch mTitleSwitch;
		AnimatedSwitch mLogoSwitch;
		bool mHold;

	public:
		explicit LiveThread();
		virtual ~LiveThread();

		void init();
		void stop();
		void setSaving(bool saving);
		void setProjectName(QString name);
		void setTitle(QString name);
		void setSubTitle(QString name);

	private:

		void clear();

	public:
		void run() override;

	public slots:
		void onFrameRenderComplete(quint64 id, QSharedPointer<QImage> im);
		void onCameraFrameReady(QSharedPointer<QImage> im);
		void onCameraOpacityChange(qreal opacity);
		void onCameraError(QCamera::Error error);
		void onCameraStateChanged(QCamera::State state);
		void onMagLevelChange(qreal level);
		void onPIPSizeChange(qreal pipSize);
		void onHoldEnabled(bool en);
		void onMagEnabled(bool en);
		void onTitleEnabled(bool en);
		void onLogoEnabled(bool en);
		void onCameraEnabled(bool en);



	signals:
		void frameRendered(quint64 id, QSharedPointer<QImage> im);
};

#endif // LIVETHREAD_HPP
