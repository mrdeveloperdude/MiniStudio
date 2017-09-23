#include "Tascam.hpp"


#include "alsaclient.h"
#include "alsaport.h"
#include "alsaqueue.h"
#include "subscription.h"


#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QTextStream>
#include <QtDebug>
#include <QReadLocker>
#include <QWriteLocker>
static QTextStream cout(stdout, QIODevice::WriteOnly);
static QTextStream cerr(stderr, QIODevice::WriteOnly);

Tascam::Tascam(QObject *parent)
	: QThread(parent)
	, m_verbose(false)
{
	m_Client = new MidiClient(this);
	m_Client->open();
	m_Client->setClientName("MiniStudio");
#ifndef USE_QEVENTS // using signals instead
	connect( m_Client, SIGNAL(eventReceived(SequencerEvent*)),
			 SLOT(sequencerEvent(SequencerEvent*)),
			 Qt::DirectConnection );
#endif

	m_Port = new MidiPort(this);
	m_Port->attach( m_Client );
	m_Port->setPortName("MiniStudio port");
	m_Port->setCapability( SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE );
	m_Port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION | SND_SEQ_PORT_TYPE_MIDI_GENERIC );
#ifdef WANT_TIMESTAMPS
	m_Queue = m_Client->createQueue("MiniStudio");
	m_Port->setTimestamping(true);
	//m_Port->setTimestampReal(true);
	m_Port->setTimestampQueue(m_Queue->getId());
#endif
	connect( m_Port, SIGNAL(subscribed(MidiPort*,Subscription*)),             SLOT(subscription(MidiPort*,Subscription*)));
	qDebug() << "Trying to subscribe from Announce";
	m_Port->subscribeFromAnnounce();
}

Tascam::~Tascam()
{
	qDebug() << "dtor Tascam";
	stopMIDI();
	wait();
	m_Port->detach();
	delete m_Port;
	m_Port=nullptr;
	m_Client->close();
	delete m_Client;
	m_Client=nullptr;
}

bool Tascam::stopped()
{
	QReadLocker locker(&m_mutex);
	return m_Stopped;
}

void Tascam::stopMIDI()
{
	QWriteLocker locker(&m_mutex);
	m_Stopped = true;
}

void Tascam::subscription(MidiPort*, Subscription* subs)
{
	qDebug() << "Subscription made from"
			 << subs->getSender()->client << ":"
			 << subs->getSender()->port;
}

void Tascam::subscribe(const QString& portName)
{

	PortInfoList inputs(m_Client->getAvailableInputs());

	PortInfo piSel;
	bool found=false;
	for(PortInfo pi: inputs) {
		//qDebug()<<"INPUT: "<<pi.getName()<<"("<<pi.getClientName()<<"): "<<pi.getPort();
		if(portName==pi.getClientName()) {
			piSel=pi;
			found=true;
			break;
		}
	}
	if(found) {
		//qDebug()<<"subscribing from "<<piSel.getClientName();
		qDebug()<<"INPUT: "<<piSel.getName()<<"("<<piSel.getClientName()<<"): "<<piSel.getPort();
		m_Port->subscribeFrom(piSel.getClient(), piSel.getPort());
	} else {

		try {
			qDebug() << "Trying to subscribe" << portName.toLocal8Bit().data();
			m_Port->subscribeFrom(portName);
		} catch (const SequencerError& err) {
			cerr << "SequencerError exception. Error code: " << err.code()
				 << " (" << err.qstrError() << ")" << endl;
			cerr << "Location: " << err.location() << endl;
			throw err;
		}
		qDebug() << "Subscription seems to have succeeded!";
	}
}

void Tascam::run()
{
	try {
#ifdef USE_QEVENTS
		m_Client->addListener(this);
		m_Client->setEventsEnabled(true);
#endif
		m_Client->setRealTimeInput(false);
		m_Client->startSequencerInput();
#ifdef WANT_TIMESTAMPS
		m_Queue->start();
#endif
		m_Stopped = false;
		qDebug()<<"Starting event queue for MIDI";
		while (!stopped()) {
#ifdef USE_QEVENTS
			QApplication::sendPostedEvents();
#endif
			sleep(1);
		}
		qDebug()<<"Event queue for MIDI DONE";
#ifdef WANT_TIMESTAMPS
		m_Queue->stop();
#endif
		m_Client->stopSequencerInput();
	} catch (const SequencerError& err) {
		cerr << "SequencerError exception. Error code: " << err.code()
			 << " (" << err.qstrError() << ")" << endl;
		cerr << "Location: " << err.location() << endl;
		qDebug()<<"Event queue for MIDI THROWING ERROR!";
		throw err;
	}

}
#ifdef USE_QEVENTS
void
QDumpMIDI::customEvent(QEvent *ev)
{
	if (ev->type() == SequencerEventType) {
		SequencerEvent* sev = static_cast<SequencerEvent*>(ev);
		if (sev != NULL) {
			dumpEvent(sev);
		}
	}
}
#else
void Tascam::sequencerEvent(SequencerEvent *ev)
{
	dumpEvent(ev);
	delete ev;
}

#endif
void Tascam::dumpEvent(SequencerEvent* sev)
{

	//qDebug()<<"DUMP";
	if(m_verbose) {

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
	emit midiEvent(sev);
}
Tascam* test;
