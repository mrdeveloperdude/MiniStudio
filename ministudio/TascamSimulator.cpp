#include "TascamSimulator.hpp"
#include "ui_TascamSimulator.h"

TascamSimulator::TascamSimulator(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TascamSimulator)
{
	ui->setupUi(this);
}

TascamSimulator::~TascamSimulator()
{
	delete ui;
}


/*
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

*/


void TascamSimulator::on_pushButtonPLAY_pressed()
{
	emit buttonEvent(22, "Play", true);
}


void TascamSimulator::on_pushButtonPLAY_released()
{
	emit buttonEvent(22, "Play", false);
}



void TascamSimulator::on_pushButtonRECORD_pressed()
{
	emit buttonEvent(23, "Record", true);
}


void TascamSimulator::on_pushButtonRECORD_released()
{
	emit buttonEvent(23, "Record", false);
}

void TascamSimulator::on_pushButtonSTOP_pressed()
{
	emit buttonEvent(21, "Stop", true);
}


void TascamSimulator::on_pushButtonSTOP_released()
{
	emit buttonEvent(21, "Stop", false);
}



void TascamSimulator::on_pushButtonAux1_pressed()
{
	emit buttonEvent(48, "Aux1", true);
}


void TascamSimulator::on_pushButtonAux1_released()
{
	emit buttonEvent(48, "Aux1", false);
}


void TascamSimulator::on_pushButtonAux2_pressed()
{
	emit buttonEvent(49, "Aux2", true);
}


void TascamSimulator::on_pushButtonAux2_released()
{
	emit buttonEvent(49, "Aux2", false);
}


void TascamSimulator::on_pushButtonAux3_pressed()
{
	emit buttonEvent(50, "Aux3", true);
}


void TascamSimulator::on_pushButtonAux3_released()
{
	emit buttonEvent(50, "Aux3", false);
}


void TascamSimulator::on_pushButtonAux4_pressed()
{
	emit buttonEvent(51, "Aux4", true);
}


void TascamSimulator::on_pushButtonAux4_released()
{
	emit buttonEvent(51, "Aux4", false);
}



void TascamSimulator::on_pushButtonASGN_pressed()
{
	emit buttonEvent(52, "Asgn", true);
}


void TascamSimulator::on_pushButtonASGN_released()
{
	emit buttonEvent(52, "Asgn", false);
}


void TascamSimulator::on_pushButtonF1_pressed()
{
	emit buttonEvent(53, "F1", true);
}


void TascamSimulator::on_pushButtonF1_released()
{
	emit buttonEvent(53, "F1", false);
}


void TascamSimulator::on_pushButtonF2_pressed()
{
	emit buttonEvent(54, "F2", true);
}


void TascamSimulator::on_pushButtonF2_released()
{
	emit buttonEvent(54, "F2", false);
}


void TascamSimulator::on_pushButtonF3_pressed()
{
	emit buttonEvent(55, "F3", true);
}


void TascamSimulator::on_pushButtonF3_released()
{
	emit buttonEvent(55, "F3", false);
}





void TascamSimulator::on_dialBig_valueChanged(int value)
{
	const qreal range=ui->dialBig->maximum()-ui->dialBig->minimum();
	const qreal fv=(value-ui->dialBig->minimum())/range;
	emit sliderEvent(96, "BigDial", fv);
}
