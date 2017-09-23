#include "FrameScene.hpp"

#include <QDebug>

FrameScene::FrameScene(quint64 id, QString outputFilename,  QSize resolution)
	: QObject(nullptr)
	, mID(id)
	, mOutputFilename(outputFilename)
	, mResolution(resolution)
{
	setAutoDelete(true);

}


FrameScene::~FrameScene()
{
	for(Layer *l:mLayers){
		delete l;
	}
	mLayers.clear();
	//qDebug()<<"FRAME" <<mID<<" deleted";
}

void FrameScene::run()
{
	//qDebug()<<"Rendering Framescene:";
	QSharedPointer<QImage> out(new QImage(mResolution, QImage::Format_ARGB32));
	QPainter painter(out.data());
	//painter.setRenderHints((QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing));
	for(QString name: mLayersOrder){
		if(mLayers.contains(name)){
			Layer *layer=mLayers[name];
			if(nullptr!=layer && layer->opacity()>0.0){
				//qDebug()<<" + LAYER "<<name;
				painter.setTransform(layer->transform(), false);
				layer->render(*this, painter);
			}
		}
	}

	if(""!=mOutputFilename){
		out->save(mOutputFilename);
	}
	emit renderComplete(mID, out);
}


void FrameScene::addImageLayer(QString name, QSharedPointer<QImage> image, qreal opacity, QTransform trans){
	mLayersOrder<<name;
	mLayers[name]=new ImageLayer(image, opacity, trans);
}


void FrameScene::addTitleLayer(QString name, QString title, QString subTitle, qreal opacity, QTransform trans){
	mLayersOrder<<name;
	mLayers[name]=new TitleLayer(title, subTitle, opacity, trans);
}


