#ifndef ANIMATEDSWITCH_HPP
#define ANIMATEDSWITCH_HPP

#include <QEasingCurve>
#include <QDebug>

class AnimatedSwitch{
	private:

		bool mEnabled;
		qreal mValue;
		qreal mSnapDist;

		QEasingCurve::Type mInType;
		QEasingCurve::Type mOutType;
		QEasingCurve mCurve;
		qreal mInTime;
		qreal mOutTime;
		qreal mEaseTime;

	public:
		explicit AnimatedSwitch(QEasingCurve::Type inType= QEasingCurve::InQuad, QEasingCurve::Type outType=QEasingCurve::OutQuad, qreal inTime=1000.0, qreal outTime=500.0)
			: mEnabled(false)
			, mValue(0.0)
			, mSnapDist(0.01)
			, mInType(inType)
			, mOutType(outType)
			, mCurve(inType)
			, mInTime(inTime)
			, mOutTime(outTime)
			, mEaseTime(inTime)
		{

		}

		inline qreal value()
		{
			return mValue;
		}

		void setEnabled(bool enabled)
		{
			if(mEnabled!=enabled){
				mEnabled=enabled;
				mCurve.setType(mEnabled?mInType:mOutType);
				mEaseTime=mEnabled?mInTime:mOutTime;
				qDebug()<<"ease time: "<<mEaseTime;
			}
		}

		qreal update(quint64 interval)
		{
			const qreal dir=mEnabled?1.0:-1.0;
			mValue+=(((qreal)interval)/mEaseTime)*dir;
			if(mEnabled && mValue> 1.0 - mSnapDist){
				mValue=1.0;
			}
			else if(!mEnabled && mValue < 0.0 + mSnapDist){
				mValue=0.0;
			}
			return mCurve.valueForProgress(mValue);
		}
};

#endif // ANIMATEDSWITCH_HPP
