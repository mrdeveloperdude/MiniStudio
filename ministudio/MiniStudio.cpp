
#include "MiniStudio.hpp"

#include "Tascam.hpp"
#include "LiveThread.hpp"
#include "StudioConfig.hpp"
#include "Presentation.hpp"

#include "TascamSimulator.hpp"

#include <QWidget>
#include <QLabel>
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QImageWriter>
#include <QImageIOHandler>
#include <QFileDialog>
#include <QMessageBox>
#include <QWindow>
#include <QScreen>
#include <QDebug>
#include <QSettings>
#include <QMenu>


#define MAX_TASCAM (256)


MiniStudio::MiniStudio(QObject *parent)
	:  QObject(parent)
	, mMidi(new Tascam(this))
	, mButtons(MAX_TASCAM)
	, mSliders(MAX_TASCAM)
	, mKnobs(MAX_TASCAM)
	, mLive(nullptr)
	, mConf(new StudioConfig())
	, mPresentation(new Presentation())
	, mMagEnabled(false)
	, mMagLevel(1.0)
	, mPIPSizeEnabled(false)
	, mPipSize(1.0)
	, mTitleEnabled(false)
	, mLogoEnabled(false)
	, mCameraEnabled(false)
	, mHoldEnabled(false)
	, mTrayIcon(new QSystemTrayIcon(this))
	, sim(new TascamSimulator())

{

	mButtonNames[19]="Rew";
	mButtonNames[20]="Fwd";
	mButtonNames[21]="Stop";
	mButtonNames[22]="Play";
	mButtonNames[23]="Record";

	mButtonNames[48]="Aux1";
	mButtonNames[49]="Aux2";
	mButtonNames[50]="Aux3";
	mButtonNames[51]="Aux4";

	mButtonNames[52]="Asgn";
	mButtonNames[53]="F1";
	mButtonNames[54]="F2";
	mButtonNames[55]="F3";

	mButtonNames[44]="High";
	mButtonNames[45]="HiMid";
	mButtonNames[46]="LowMid";
	mButtonNames[47]="Low";

	mButtonNames[255]="InputC+D";

	mButtonNames[4]="Mute5";
	mButtonNames[5]="Mute6";
	mButtonNames[6]="Mute7";
	mButtonNames[7]="Mute8";

	mButtonNames[36]="Select5";
	mButtonNames[37]="Select6";
	mButtonNames[38]="Select7";
	mButtonNames[39]="Select8";

	mSliderNames[64]="Channel1";
	mSliderNames[65]="Channel2";
	mSliderNames[66]="Channel3";
	mSliderNames[67]="Channel4";
	mSliderNames[68]="Channel5";
	mSliderNames[69]="Channel6";
	mSliderNames[70]="Channel7";
	mSliderNames[71]="Channel8";

	mKnobNames[96]="BigDial";
	mKnobNames[72]="Gain";
	mKnobNames[73]="Freq";
	mKnobNames[74]="Q";
	mKnobNames[77]="Pan";


	for(int i=0; i<MAX_TASCAM; ++i) {
		mButtons[i]=false;
		mSliders[i]=0.0;
		mKnobs[1]=0;
	}

	mConf->move(QApplication::desktop()->availableGeometry(mConf).topLeft() + QPoint(20, 20));
	//rich.show();

	mMidi->subscribe("US-428");
	//m_midi->setVerbose(true);

	if(!connect(mMidi, &Tascam::midiEvent, this, &MiniStudio::onMIDIReceive)) {
		qWarning()<<"ERROR: could not connect MIDI";
	}

	if(!connect(this, &MiniStudio::buttonEvent, this, &MiniStudio::onButtonEvent)) {
		qWarning()<<"ERROR: could not connect button MIDI";
	}

	if(!connect(this, &MiniStudio::sliderEvent, this, &MiniStudio::onSliderEvent)) {
		qWarning()<<"ERROR: could not connect slider MIDI";
	}


	if(nullptr!=sim) {
		if(!connect(sim, &TascamSimulator::buttonEvent, this, &MiniStudio::onButtonEvent)) {
			qWarning()<<"ERROR: could not connect button TascamSimulator";
		}

		if(!connect(sim, &TascamSimulator::sliderEvent, this, &MiniStudio::onSliderEvent)) {
			qWarning()<<"ERROR: could not connect slider TascamSimulator";
		}
	}

	mMidi->start();
	if(!connect(mConf, &StudioConfig::quitApp, this, &MiniStudio::onQuitApp)) {
		qWarning()<<"ERROR: could not connect studio close app";
	}
	if(!connect(mConf, &StudioConfig::showSimulator, this, &MiniStudio::onShowSimulator)) {
		qWarning()<<"ERROR: could not connect studio show sim";
	}


	if(!connect(mConf, &StudioConfig::textChanged, mPresentation, &Presentation::onTextChanged)) {
		qWarning()<<"ERROR: could not connect studio textChanged";
	}


	if(!connect(mConf, &StudioConfig::verbosity, this, &MiniStudio::onVerbosityChange)) {
		qWarning()<<"ERROR: could not connect studio verbosity";
	}





	//void onTextChanged();
	/*
	void MiniStudio::onTextChanged()
	{
		mConf->slidesText();
	}
	*/


	if(!connect(mTrayIcon, &QSystemTrayIcon::activated, this, &MiniStudio::onTrayActivated)) {
		qWarning()<<"ERROR: could not connect QSystemTrayIcon";
	}

	mTrayIcon->setObjectName("MiniStudioTray");
	mTrayIcon->setIcon(QPixmap(":/icons/pause.svg"));
	mTrayIcon->setToolTip("Show MiniStudio controls");
	auto men=new QMenu(nullptr);
	men->addAction(QPixmap(":/icons/no.svg"),"Quit", this, SLOT(onQuitApp()));
	men->addAction(QPixmap(":/icons/settings.svg"),"Show",  mConf, SLOT(show()));
	mTrayIcon->setContextMenu(men);
	mTrayIcon->show();


	loadSettings();
	showConfig(true);
}


MiniStudio::~MiniStudio()
{
	saveSettings();
	qDebug()<<"dtor MiniStudio";

	mLive->stop();
	mLive->wait();

	delete mMidi;
	mMidi=nullptr;

	delete mConf;
	mConf=nullptr;

	delete mPresentation;
	mPresentation=nullptr;
}



void MiniStudio::saveSettings()
{
	QSettings *s=mConf->settings();
	if(nullptr!=s) {
		qDebug()<<"SAVING MINISTUDIO VALUES";
		s->setValue("magLevel",mMagLevel);
		s->setValue("mPipSize",mPipSize);
		s->setValue("mTitleEnabled",mTitleEnabled);
		s->setValue("mLogoEnabled",mLogoEnabled);
		s->setValue("mCameraEnabled",mCameraEnabled);
	}
}

void MiniStudio::loadSettings()
{
	QSettings *s=mConf->settings();
	if(nullptr!=s) {
		qDebug()<<"LOADING MINISTUDIO VALUES";
		mMagLevel=s->value("magLevel",mMagLevel).toReal();
		mPipSize=s->value("mPipSize",mPipSize).toReal();;
		mTitleEnabled=s->value("mTitleEnabled",mTitleEnabled).toBool();
		mLogoEnabled=s->value("mLogoEnabled",mLogoEnabled).toBool();
		mCameraEnabled=s->value("mCameraEnabled",mCameraEnabled).toBool();
	}
}




static void logMidi(SequencerEvent* sev )
{
	auto cout=qDebug();
#ifdef WANT_TIMESTAMPS
	cout << qSetFieldWidth(8) << right << sev->getTick();
	/* More timestamp options:
	cout << sev->getRealTimeSecs();
	cout << sev->getRealTimeNanos(); */
	/* Getting the time from the queue status object;
	QueueStatus sts = m_Queue->getStatus();
	cout << qSetFieldWidth(8) << right << sts.getClockTime();
	cout << sts.getTickTime(); */
	cout << qSetFieldWidth(0) << " ";
#endif
	cout << qSetFieldWidth(3) << right << sev->getSourceClient() << qSetFieldWidth(0) << ":";
	cout << qSetFieldWidth(3) << left << sev->getSourcePort() << qSetFieldWidth(0) << " ";
	switch (sev->getSequencerType()) {
	case SND_SEQ_EVENT_NOTEON: {
		NoteOnEvent* e = static_cast<NoteOnEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(23) << left << "Note on";
			cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
			cout << qSetFieldWidth(3) << e->getKey() << " ";
			cout << qSetFieldWidth(3) << e->getVelocity();
		}
		break;
	}
	case SND_SEQ_EVENT_NOTEOFF: {
		NoteOffEvent* e = static_cast<NoteOffEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(23) << left << "Note off";
			cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
			cout << qSetFieldWidth(3) << e->getKey() << " ";
			cout << qSetFieldWidth(3) << e->getVelocity();
		}
		break;
	}
	case SND_SEQ_EVENT_KEYPRESS: {
		KeyPressEvent* e = static_cast<KeyPressEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(23) << left << "Polyphonic aftertouch";
			cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
			cout << qSetFieldWidth(3) << e->getKey() << " ";
			cout << qSetFieldWidth(3) << e->getVelocity();
		}
		break;
	}
	case SND_SEQ_EVENT_CONTROL14:
	case SND_SEQ_EVENT_NONREGPARAM:
	case SND_SEQ_EVENT_REGPARAM:
	case SND_SEQ_EVENT_CONTROLLER: {
		ControllerEvent* e = static_cast<ControllerEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(23) << left << "Control change";
			cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
			cout << qSetFieldWidth(3) << e->getParam() << " ";
			cout << qSetFieldWidth(3) << e->getValue();
		}
		break;
	}
	case SND_SEQ_EVENT_PGMCHANGE: {
		ProgramChangeEvent* e = static_cast<ProgramChangeEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(23) << left << "Program change";
			cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
			cout << qSetFieldWidth(3) << e->getValue();
		}
		break;
	}
	case SND_SEQ_EVENT_CHANPRESS: {
		ChanPressEvent* e = static_cast<ChanPressEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(23) << left << "Channel aftertouch";
			cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
			cout << qSetFieldWidth(3) << e->getValue();
		}
		break;
	}
	case SND_SEQ_EVENT_PITCHBEND: {
		PitchBendEvent* e = static_cast<PitchBendEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(23) << left << "Pitch bend";
			cout << qSetFieldWidth(2) << right << e->getChannel() << " ";
			cout << qSetFieldWidth(5) << e->getValue();
		}
		break;
	}
	case SND_SEQ_EVENT_SONGPOS: {
		ValueEvent* e = static_cast<ValueEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Song position pointer" << qSetFieldWidth(0);
			cout << e->getValue();
		}
		break;
	}
	case SND_SEQ_EVENT_SONGSEL: {
		ValueEvent* e = static_cast<ValueEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Song select" << qSetFieldWidth(0);
			cout << e->getValue();
		}
		break;
	}
	case SND_SEQ_EVENT_QFRAME: {
		ValueEvent* e = static_cast<ValueEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "MTC quarter frame" << qSetFieldWidth(0);
			cout << e->getValue();
		}
		break;
	}
	case SND_SEQ_EVENT_TIMESIGN: {
		ValueEvent* e = static_cast<ValueEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "SMF time signature" << qSetFieldWidth(0);
			cout << hex << e->getValue();
			cout << dec;
		}
		break;
	}
	case SND_SEQ_EVENT_KEYSIGN: {
		ValueEvent* e = static_cast<ValueEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "SMF key signature" << qSetFieldWidth(0);
			cout << hex << e->getValue();
			cout << dec;
		}
		break;
	}
	case SND_SEQ_EVENT_SETPOS_TICK: {
		QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Set tick queue pos." << qSetFieldWidth(0);
			cout << e->getQueue();
		}
		break;
	}
	case SND_SEQ_EVENT_SETPOS_TIME: {
		QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Set rt queue pos." << qSetFieldWidth(0);
			cout << e->getQueue();
		}
		break;
	}
	case SND_SEQ_EVENT_TEMPO: {
		TempoEvent* e = static_cast<TempoEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Set queue tempo";
			cout << qSetFieldWidth(3) << right << e->getQueue() << qSetFieldWidth(0) << " ";
			cout << e->getValue();
		}
		break;
	}
	case SND_SEQ_EVENT_QUEUE_SKEW: {
		QueueControlEvent* e = static_cast<QueueControlEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Queue timer skew" << qSetFieldWidth(0);
			cout << e->getQueue();
		}
		break;
	}
	case SND_SEQ_EVENT_START:
		cout << left << "Start";
		break;
	case SND_SEQ_EVENT_STOP:
		cout << left << "Stop";
		break;
	case SND_SEQ_EVENT_CONTINUE:
		cout << left << "Continue";
		break;
	case SND_SEQ_EVENT_CLOCK:
		cout << left << "Clock";
		break;
	case SND_SEQ_EVENT_TICK:
		cout << left << "Tick";
		break;
	case SND_SEQ_EVENT_TUNE_REQUEST:
		cout << left << "Tune request";
		break;
	case SND_SEQ_EVENT_RESET:
		cout << left << "Reset";
		break;
	case SND_SEQ_EVENT_SENSING:
		cout << left << "Active Sensing";
		break;
	case SND_SEQ_EVENT_CLIENT_START: {
		ClientEvent* e = static_cast<ClientEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Client start"
				 << qSetFieldWidth(0) << e->getClient();
		}
		break;
	}
	case SND_SEQ_EVENT_CLIENT_EXIT: {
		ClientEvent* e = static_cast<ClientEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Client exit"
				 << qSetFieldWidth(0) << e->getClient();
		}
		break;
	}
	case SND_SEQ_EVENT_CLIENT_CHANGE: {
		ClientEvent* e = static_cast<ClientEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Client changed"
				 << qSetFieldWidth(0) << e->getClient();
		}
		break;
	}
	case SND_SEQ_EVENT_PORT_START: {
		PortEvent* e = static_cast<PortEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Port start" << qSetFieldWidth(0);
			cout << e->getClient() << ":" << e->getPort();
		}
		break;
	}
	case SND_SEQ_EVENT_PORT_EXIT: {
		PortEvent* e = static_cast<PortEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Port exit" << qSetFieldWidth(0);
			cout << e->getClient() << ":" << e->getPort();
		}
		break;
	}
	case SND_SEQ_EVENT_PORT_CHANGE: {
		PortEvent* e = static_cast<PortEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Port changed" << qSetFieldWidth(0);
			cout << e->getClient() << ":" << e->getPort();
		}
		break;
	}
	case SND_SEQ_EVENT_PORT_SUBSCRIBED: {
		SubscriptionEvent* e = static_cast<SubscriptionEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Port subscribed" << qSetFieldWidth(0);
			cout << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
			cout << e->getDestClient() << ":" << e->getDestPort();
		}
		break;
	}
	case SND_SEQ_EVENT_PORT_UNSUBSCRIBED: {
		SubscriptionEvent* e = static_cast<SubscriptionEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "Port unsubscribed" << qSetFieldWidth(0);
			cout << e->getSenderClient() << ":" << e->getSenderPort() << " -> ";
			cout << e->getDestClient() << ":" << e->getDestPort();
		}
		break;
	}
	case SND_SEQ_EVENT_SYSEX: {
		SysExEvent* e = static_cast<SysExEvent*>(sev);
		if (e != NULL) {
			cout << qSetFieldWidth(26) << left << "System exclusive" << qSetFieldWidth(0);
			unsigned int i;
			for (i = 0; i < e->getLength(); ++i) {
				cout << hex << (unsigned char) e->getData()[i] << " ";
			}
			cout << dec;
		}
		break;
	}
	default:
		cout << qSetFieldWidth(26) << "Unknown event type" << qSetFieldWidth(0);
		cout << sev->getSequencerType();
	};
	cout << qSetFieldWidth(0) << endl;
}


void MiniStudio::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
	qDebug()<<"ACTIVATED! "<<reason;
}


void MiniStudio::onMIDIReceive(SequencerEvent* sev )
{
	auto cout=qDebug();
	snd_seq_event_type_t type=sev->getSequencerType();
	switch (type) {
	//		case SND_SEQ_EVENT_CONTROL14:
	//case SND_SEQ_EVENT_NONREGPARAM:
	//case SND_SEQ_EVENT_REGPARAM:
	case SND_SEQ_EVENT_CONTROLLER: {
		ControllerEvent* e = static_cast<ControllerEvent*>(sev);
		if (e != 0) {
			//Handle all buttons
			uint parameter=e->getParam();
			int value=e->getValue();
			if(parameter>=mButtons.size()) {
				parameter=mButtons.size()-1;
			}
			if(mButtonNames.contains(parameter)) {
				QString buttonName=mButtonNames[parameter];
				const bool press=127==value;
				if(mButtons[parameter] != press) {
					mButtons[parameter]=press;
					//cout << "BUTTON "<<buttonName<<" ("<<parameter<< (press?") PRESS":") RELEASE");
					emit buttonEvent(parameter, buttonName, press);
				}
				return;

			} else if(mSliderNames.contains(parameter)) {
				QString sliderName=mSliderNames[parameter];
				qreal v=value/127.0f;
				qreal old=mSliders[parameter];
				if(old!=v) {
					mSliders[parameter]=v;
					emit onSliderEvent(parameter,sliderName,v);
					//cout << "SLIDER "<<sliderName<<" ("<<parameter<<") CHANGE TO "<<v;
				}
				return;
			} else if(mKnobNames.contains(parameter)) {
				qint64 old=mKnobs[parameter];
				if(old!=value) {
					const bool up=(value>old);
					mKnobs[parameter]=value;
					QString knobName=mKnobNames[parameter];
					emit onKnobEvent(parameter,knobName,up);
					//cout << "KNOB "<<knobName<<" ("<<parameter<<") "<< (up?"UP":"DOWN");
				}
				return;
			}
		}
		break;
	}
	}
	//Fall back to general logging
	logMidi(sev);
}



void MiniStudio::showConfig(bool show)
{
	qDebug()<<"CONFIG: "<<(show?"SHOW":"HIDE");
	mConf->setVisible(show);
}


void MiniStudio::setRecording(bool run, bool rec)
{
	qDebug()<<"LIVE: "<<(run?(rec?"RECORDING":"PLAYING"):"STOPPED");
	if(run) {
		if(nullptr==mLive) {
			mLive=new LiveThread;
			if(nullptr!=mConf) {
				if(! connect(mLive, &LiveThread::frameRendered, mConf, &StudioConfig::onPreviewUpdated) ) {
					qWarning()<<"ERROR: could not connect frame render";
				}
			}
			mLive->setProjectName((nullptr!=mConf)?mConf->projectName():"");
			mLive->setTitle((nullptr!=mConf)?mConf->title():"");
			mLive->setSubTitle((nullptr!=mConf)?mConf->subTitle():"");
			mLive->setSaving(rec);
			mLive->onCameraEnabled(mCameraEnabled);
			mLive->onLogoEnabled(mLogoEnabled);
			mLive->onMagEnabled(mMagEnabled);
			mLive->onMagLevelChange(mMagLevel);
			mLive->onPIPSizeChange(mPipSize);
			mLive->onTitleEnabled(mTitleEnabled);
			mLive->start();
		}
	} else {
		if(nullptr!=mLive) {
			mLive->stop();
			mLive->wait();
			mLive->deleteLater();
			mLive=nullptr;
		}
	}
	if(nullptr!=mLive) {
		mLive->setSaving(rec);
	}
}



void MiniStudio::showMagnifier(bool show)
{
	qDebug()<<"MAG: "<<(show?"SHOW":"HIDE");
	mMagEnabled=show;
	if(nullptr!=mLive) {
		mLive->onMagEnabled(show);
	}
}


void MiniStudio::onButtonEvent(uint, QString name, bool pressed)
{
	if(!pressed) {
		if("Play"==name) {
			setRecording(true, false);
		} else if("Record"==name) {
			setRecording(true, true);
		} else if("Stop"==name) {
			setRecording(false, false);
		} else if("InputC+D"==name) {
			bool show=!mConf->isVisible();
			showConfig(show);
		}
	}
	if("F1"==name) {
		if(nullptr!=mLive) {
			showMagnifier(pressed);
		}
	} else if("F2"==name) {

		qDebug()<<"HOLD: "<<(pressed?"ON":"OFF");
		mHoldEnabled=pressed;
		if(nullptr!=mLive) {
			mLive->onHoldEnabled(mHoldEnabled);
		}

	} else if("F3"==name) {
		if(nullptr!=mLive) {
			mPIPSizeEnabled=pressed;
			//qDebug()<<"pip scale enabled: "<<mPIPSizeEnabled;
		}
	} else if("Aux1"==name) {
		if(nullptr!=mLive && pressed) {
			mCameraEnabled=!mCameraEnabled;
			mLive->onCameraEnabled(mCameraEnabled);
			qDebug()<<"camera enabled: "<<mCameraEnabled;
		}
	} else if("Aux2"==name) {
		if(nullptr!=mLive && pressed) {
			mTitleEnabled=!mTitleEnabled;
			mLive->onTitleEnabled(mTitleEnabled);
			qDebug()<<"title enabled: "<<mTitleEnabled;
		}
	} else if("Aux3"==name) {
		if(nullptr != mPresentation && pressed) {
			const bool vis=!mPresentation->isVisible();
			mPresentation->setVisible(vis);
			if(vis) {
				mPresentation->showFullScreen();
				mConf->setFocus();
				mConf->raise();
				//QWidget::showFullScreen();
			}
			qDebug()<<"presentation enabled: "<<vis;
		}
	} else if("Aux4"==name) {
		if(nullptr!=mLive && pressed) {
			mLogoEnabled=!mLogoEnabled;
			mLive->onLogoEnabled(mLogoEnabled);
			qDebug()<<"logo enabled: "<<mLogoEnabled;
		}
	}

}


void MiniStudio::onSliderEvent(uint, QString name, qreal value)
{
	if("Channel1"==name) {
		//qDebug()<<"cam opac "<<value;
		if(nullptr!=mLive) {
			emit mLive->onCameraOpacityChange(value);
		}
	}
}



void MiniStudio::onKnobEvent(uint id, QString name, bool up)
{
	const qreal scaleFactor=0.05;
	if("BigDial"==name) {
		if(mMagEnabled && nullptr!=mLive) {
			qreal newLevel=mMagLevel+mMagLevel*(up?1:-1)*scaleFactor;
			if(newLevel>0.0) {
				mMagLevel=newLevel;
				qDebug()<<"mag level"<<mMagLevel;
				emit mLive->onMagLevelChange(mMagLevel);
			}
		} else if(mPIPSizeEnabled && nullptr!=mLive) {
			qreal newSize=mPipSize+mPipSize*(up?1:-1)*scaleFactor;
			if(newSize>0.0) {
				mPipSize=newSize;
				qDebug()<<"pip size"<<mPipSize;
				emit mLive->onPIPSizeChange(mPipSize);
			}
		}
	}
}


void MiniStudio::onQuitApp()
{
	qApp->exit();
}




void MiniStudio::onShowSimulator()
{
	if(nullptr!=sim) {
		sim->show();
	}
}



void MiniStudio::onVerbosityChange(bool v)
{
	if(nullptr!=mMidi)	{
		mMidi->setVerbose(v);
	}
}
