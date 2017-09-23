#ifndef LAYER_HPP
#define LAYER_HPP

#include <QString>
#include <QTransform>
#include <QImage>
#include <QDebug>
#include <QPainter>
#include <QSharedPointer>

class FrameScene;

class Layer{
	protected:
		QString mName;
		qreal mOpacity;
		QTransform mTransform;
	public:

		explicit Layer(QString name, qreal opacity=1.0, QTransform transform=QTransform());
		virtual ~Layer();

	public:

		QString name();
		qreal opacity();
		QTransform &transform();

		virtual void render(FrameScene &fs, QPainter &p) = 0;
};

////////////////////////////////////////////////////////////////////////////////

class ImageLayer: public Layer{
	private:
		QSharedPointer<QImage> mImage;
	public:
		explicit ImageLayer(QSharedPointer<QImage> image, qreal opacity=1.0, QTransform transform=QTransform());

		virtual ~ImageLayer();

		void render(FrameScene &fs, QPainter &p) override;
};

////////////////////////////////////////////////////////////////////////////////

class TitleLayer: public Layer{
	private:
		QString mTitle;
		QString  mSubTitle;

	public:
		explicit TitleLayer(QString title, QString subTitle, qreal opacity=1.0, QTransform transform=QTransform());

		virtual ~TitleLayer();
	public:
		void render(FrameScene &fs, QPainter &p) override;
};


#endif // LAYER_HPP
