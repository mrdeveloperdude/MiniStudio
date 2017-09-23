#ifndef DUMPMIDI_HPP
#define DUMPMIDI_HPP

/* MidiClient can deliver SequencerEvents with only
 * signals or posting QEvents to the QApplication loop */
#undef USE_QEVENTS
//#define USE_QEVENTS
/* Tp get timestamped events from ALSA, you need a running queue */
//#undef WANT_TIMESTAMPS
#define WANT_TIMESTAMPS
#include <QThread>
#include <QReadWriteLock>


#include "alsaevent.h"


namespace drumstick
{


//class SequencerEvent;
class MidiPort;
class MidiClient;
class MidiQueue;
class Subscription;

}

using namespace drumstick;
class Tascam : public QThread
{
	Q_OBJECT
private:
	MidiClient* m_Client;
	MidiPort* m_Port;
#ifdef WANT_TIMESTAMPS
	MidiQueue* m_Queue;
#endif
	bool m_Stopped;
	QReadWriteLock m_mutex;

	bool m_verbose;

public:
	Tascam(QObject *parent=nullptr);
	virtual ~Tascam();
	void dumpEvent(SequencerEvent* ev);
	void subscribe(const QString& portName);
	void stopMIDI();
	bool stopped();

	inline bool verbose()
	{
		return m_verbose;
	}

	inline void setVerbose(bool v)
	{
		m_verbose=v;
	}

	void run() override;
public slots:
	void subscription(MidiPort* port, Subscription* subs);
#ifdef USE_QEVENTS
protected:
	virtual void customEvent( QEvent *ev );
#else
	void sequencerEvent( SequencerEvent* ev );
#endif

signals:

	void midiEvent( SequencerEvent* ev );


};



#endif // DUMPMIDI_HPP
