#ifndef SCREENSHOT_H
#define SCREENSHOT_H

//https://www.semicomplete.com/projects/xdotool/xdotool.xhtml

//xdotool getactivewindow windowsize 1280 720 ; xdotool getactivewindow windowmove 100 100


class TascamSimulator;

#include "RichEdit.hpp"



#include <QPixmap>
#include <QWidget>
#include <QMap>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QSpinBox;
class QVBoxLayout;
QT_END_NAMESPACE

class Tascam;
class LiveThread;
class StudioConfig;
class Presentation;

namespace drumstick
{
class SequencerEvent;
}



class MiniStudio : public QObject
{
	Q_OBJECT

private:

	Tascam *mMidi;
	QVector<bool> mButtons;
	QVector<qreal> mSliders;
	QVector<qint64> mKnobs;
	QMap<uint, QString> mButtonNames;
	QMap<uint, QString> mSliderNames;
	QMap<uint, QString> mKnobNames;
	LiveThread *mLive;
	StudioConfig *mConf;
	Presentation *mPresentation;
	bool mMagEnabled;
	qreal mMagLevel;
	bool mPIPSizeEnabled;
	qreal mPipSize;
	bool mTitleEnabled;
	bool mLogoEnabled;
	bool mCameraEnabled;
	bool mHoldEnabled;

	QSystemTrayIcon *mTrayIcon;
	TascamSimulator *sim;

public:
	explicit MiniStudio(QObject *parent=nullptr);

	virtual ~MiniStudio();

private:

	void showConfig(bool show);
	void setRecording(bool run, bool rec);
	void showMagnifier(bool show);

	void saveSettings();
	void loadSettings();

public slots:
	void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
	void onMIDIReceive(drumstick::SequencerEvent* ev );
	void onButtonEvent(uint id, QString name, bool presssed);
	void onSliderEvent(uint id, QString name, qreal value);
	void onKnobEvent(uint id, QString name, bool up);
	void onQuitApp();
	void onShowSimulator();
	void onVerbosityChange(bool);


signals:
	void buttonEvent(uint id, QString name, bool presssed);
	void sliderEvent(uint id, QString name, qreal value);


};


#endif // SCREENSHOT_H
