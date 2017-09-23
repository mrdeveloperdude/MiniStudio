#ifndef FRAMESCENE_HPP
#define FRAMESCENE_HPP

#include "Layer.hpp"


#include <QRunnable>
#include <QStringList>
#include <QMap>
#include <QImage>
#include <QPainter>
#include <QRectF>
#include <QSharedPointer>


class FrameScene : public QObject, public QRunnable
{
		Q_OBJECT
	private:
		quint64 mID;
		QString mOutputFilename;
		QSize mResolution;
		QStringList mLayersOrder;
		QMap<QString, Layer *> mLayers;

	public:
		explicit FrameScene(quint64 id, QString outputFilename,  QSize resolution);
		virtual ~FrameScene();

		void addImageLayer(QString name, QSharedPointer<QImage> image, qreal opacity=1.0, QTransform trans=QTransform());
		void addTitleLayer(QString name, QString title, QString subTitle, qreal opacity=1.0, QTransform trans=QTransform());
		void run() override;

		const QSize &resolution()
		{
			return mResolution;
		}

	signals:

		void renderComplete(quint64 id, QSharedPointer<QImage> im);
};

#endif // FRAMESCENE_HPP
