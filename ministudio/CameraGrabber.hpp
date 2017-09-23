#ifndef CAMERAGRABBER_HPP
#define CAMERAGRABBER_HPP

#include <QAbstractVideoSurface>
#include <QList>
#include <QSharedPointer>


class CLVideoFilter;




class CameraGrabber : public QAbstractVideoSurface
{
	Q_OBJECT
private:

	CLVideoFilter *clFilter;
public:
	explicit CameraGrabber(QObject *parent = 0);
	QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
	bool present(const QVideoFrame &frame);

signals:
	void frameAvailable(QSharedPointer<QImage> frame);

public slots:

};

#endif // CAMERAGRABBER_HPP
