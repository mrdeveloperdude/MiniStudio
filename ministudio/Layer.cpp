#include "Layer.hpp"
#include "FrameScene.hpp"




Layer::Layer(QString name, qreal opacity, QTransform transform)
	: mName(name)
	, mOpacity(opacity)
	, mTransform(transform)
{

}

Layer::~Layer(){

}


QString Layer::name(){
	return mName;
}


qreal Layer::opacity(){
	return mOpacity;
}

QTransform &Layer::transform(){
	return mTransform;
}


////////////////////////////////////////////////////////////////////////////////

ImageLayer::ImageLayer(QSharedPointer<QImage> image, qreal opacity, QTransform transform)
	: Layer("Image", opacity, transform)
	, mImage(image)
{

}

ImageLayer::~ImageLayer(){

}

void ImageLayer::render(FrameScene &fs, QPainter &p)
{
	(void)fs;
	if(!mImage.isNull()){
		p.setOpacity(mOpacity);
		p.drawImage(QPointF(0,0),*mImage);
	}
	else{
		qWarning()<<"Trying to render null frame for "<<mName<<" layer";
	}
}


////////////////////////////////////////////////////////////////////////////////


TitleLayer::TitleLayer(QString title, QString subTitle, qreal opacity, QTransform transform)
	: Layer("Title", opacity, transform)
	, mTitle(title)
	, mSubTitle(subTitle)
{

}

TitleLayer::~TitleLayer(){

}

void TitleLayer::render(FrameScene &fs, QPainter &p)
{
	(void)fs;
	const QSize &sz=fs.resolution();
	const quint32 h=sz.height()/10;
	const quint32 w=(sz.width()*8)/10;
	const quint32 hh=h*1.5;
	p.setOpacity(mOpacity*0.2);
	const quint32 shadowOffset=h/7;
	p.fillRect(QRect(0,sz.height()-h*2+shadowOffset,w+shadowOffset,hh),Qt::black);
	p.setOpacity(mOpacity);
	p.fillRect(QRect(0,sz.height()-h*2,w,hh),Qt::white);
	p.setPen(Qt::black);
	QFont font("Dosis");
	font.setPixelSize(hh*0.5);
	font.setWeight(QFont::Normal);
	p.setFont(font);
	p.drawText(hh,sz.height()-h*2+hh*0.5, mTitle);
	font.setWeight(QFont::Bold);
	font.setPixelSize(hh*0.35);
	p.setFont(font);
	p.drawText(hh,sz.height()-h*2+hh*0.85, mSubTitle);
}
