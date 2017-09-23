#include "CameraGrabber.hpp"

#include <QVideoFrame>
#include <QCameraInfo>




#include "CLVideoFilter.hpp"






CameraGrabber::CameraGrabber(QObject *parent)
	: QAbstractVideoSurface(parent)
	, clFilter(nullptr)
{
	//qDebug()<<"CameraGrabber ctor";
}

QList<QVideoFrame::PixelFormat> CameraGrabber::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
	//qDebug()<<"supportedPixelFormats";
	Q_UNUSED(handleType);
	return QList<QVideoFrame::PixelFormat>()
		   << QVideoFrame::Format_ARGB32
		   << QVideoFrame::Format_ARGB32_Premultiplied
		   << QVideoFrame::Format_RGB32
		   << QVideoFrame::Format_RGB24
		   << QVideoFrame::Format_RGB565
		   << QVideoFrame::Format_RGB555
		   << QVideoFrame::Format_ARGB8565_Premultiplied
		   << QVideoFrame::Format_BGRA32
		   << QVideoFrame::Format_BGRA32_Premultiplied
		   << QVideoFrame::Format_BGR32
		   << QVideoFrame::Format_BGR24
		   << QVideoFrame::Format_BGR565
		   << QVideoFrame::Format_BGR555
		   << QVideoFrame::Format_BGRA5658_Premultiplied
		   << QVideoFrame::Format_AYUV444
		   << QVideoFrame::Format_AYUV444_Premultiplied
		   << QVideoFrame::Format_YUV444
		   << QVideoFrame::Format_YUV420P
		   << QVideoFrame::Format_YV12
		   << QVideoFrame::Format_UYVY
		   << QVideoFrame::Format_YUYV
		   << QVideoFrame::Format_NV12
		   << QVideoFrame::Format_NV21
		   << QVideoFrame::Format_IMC1
		   << QVideoFrame::Format_IMC2
		   << QVideoFrame::Format_IMC3
		   << QVideoFrame::Format_IMC4
		   << QVideoFrame::Format_Y8
		   << QVideoFrame::Format_Y16
		   << QVideoFrame::Format_Jpeg
		   << QVideoFrame::Format_CameraRaw
		   << QVideoFrame::Format_AdobeDng;
}

bool CameraGrabber::present(const QVideoFrame &frame)
{
	//qDebug()<<"camframe";
	if (frame.isValid()) {
		QVideoFrame cloneFrame(frame);
		cloneFrame.map(QAbstractVideoBuffer::ReadOnly);




		if(nullptr==clFilter) {
			clFilter = new CLVideoFilter();
		}

		QSharedPointer<QImage> image;
		if(nullptr!=clFilter) {
			image=clFilter->run(&cloneFrame);
		} else {
			image=QSharedPointer<QImage> (new QImage(cloneFrame.bits(), cloneFrame.width(), cloneFrame.height(), QVideoFrame::imageFormatFromPixelFormat(cloneFrame .pixelFormat())));
		}
		emit frameAvailable(image);
		cloneFrame.unmap();
		return true;
	} else {
		qWarning()<<"frame was invalid";
	}
	return false;
}
