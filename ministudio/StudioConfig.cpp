#include "StudioConfig.hpp"
#include "ui_StudioConfig.h"

#include <QDesktopWidget>
#include <QDebug>
#include <QLineEdit>
#include <QSettings>

StudioConfig::StudioConfig(QWidget*parent)
	: QWidget(parent)
	, ui(new Ui::StudioConfig)
	, mDidFirstPlay(false)
	, mSettings(new QSettings(this))

{
	ui->setupUi(this);

	move(QApplication::desktop()->availableGeometry(this).topLeft() + QPoint(20, 20));

	QList<int> sz;
	sz<<ui->gridLayoutForm->geometry().width()<< 0;
	ui->splitterPreview->setSizes(sz);
	loadSettings();

	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	connect(ui->widgetSlideContent, &RichEdit::textChanged, this, &StudioConfig::textChanged);
}


void StudioConfig::resizeEvent(QResizeEvent * )
{
	QSize scaledSize = mLastPreviewPixmap.size();
	scaledSize.scale(ui->labelPreview->size(), Qt::KeepAspectRatio);
	if (!ui->labelPreview->pixmap() || scaledSize != ui->labelPreview->pixmap()->size()) {

		ui->labelPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		ui->labelPreview->setAlignment(Qt::AlignCenter);

		const QRect screenGeometry = QApplication::desktop()->screenGeometry(this);
		quint32 div=4;
		ui->labelPreview->setMinimumSize(screenGeometry.width() / div, screenGeometry.height() / div);
		ui->labelPreview->setPixmap(mLastPreviewPixmap.scaled(ui->labelPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
}


StudioConfig::~StudioConfig()
{
	qDebug()<<"dtor StudioConfig";
	saveSettings();
	mSettings->deleteLater();
	delete ui;
}



void StudioConfig::onPreviewUpdated(quint64 id, QSharedPointer<QImage> img)
{
	if(!mDidFirstPlay) {
		QList<int> sz;
		sz<<ui->gridLayoutForm->geometry().width()<< (ui->labelPreview->size().width());
		ui->splitterPreview->setSizes(sz);
		mDidFirstPlay=true;
	}
	//qDebug()<<"conf:preview updated "<<id;
	QSize oldSize = mLastPreviewPixmap.size();

	QPixmap newPixmap=QPixmap::fromImage(*img);
	QSize newSize = newPixmap.size();
	mLastPreviewPixmap=newPixmap;

	ui->labelPreview->setPixmap(mLastPreviewPixmap.scaled(ui->labelPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	if(oldSize!=newSize) {
		resizeEvent(nullptr);
	}

}



QString StudioConfig::projectName()
{
	return ui->lineEditProjectName->text();
}


QString StudioConfig::title()
{
	return ui->lineEditTitle->text();
}


QString StudioConfig::subTitle()
{
	return ui->lineEditSubTitle->text();
}

void StudioConfig::on_pushButtonQuit_clicked()
{
	close();
	emit quitApp();
}


void StudioConfig::saveLineEdit(QLineEdit &le)
{
	qDebug()<<"SAVING "<<le.objectName() <<" AS "<< le.text();
	mSettings->setValue(le.objectName(), le.text());
}


void StudioConfig::loadLineEdit(QLineEdit &le)
{
	QString t=mSettings->value(le.objectName(), "DEFAULT").toString();
	qDebug()<<"LOADING "<<le.objectName() <<" AS "<< t;
	le.setText(t);
}

void StudioConfig::saveSettings()
{
	saveLineEdit(*ui->lineEditProjectName);
	saveLineEdit(*ui->lineEditTitle);
	saveLineEdit(*ui->lineEditSubTitle);
}

void StudioConfig::loadSettings()
{
	loadLineEdit(*ui->lineEditProjectName);
	loadLineEdit(*ui->lineEditTitle);
	loadLineEdit(*ui->lineEditSubTitle);
}


QSettings *StudioConfig::settings()
{
	return mSettings;
}

void StudioConfig::on_pushButtonSimulator_clicked()
{
	emit showSimulator();
}

void StudioConfig::on_pushButtonLog_toggled(bool checked)
{
	emit verbosity(checked);
}
