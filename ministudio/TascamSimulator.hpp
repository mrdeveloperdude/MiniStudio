#ifndef TASCAMSIMULATOR_HPP
#define TASCAMSIMULATOR_HPP

#include <QWidget>

namespace Ui
{
class TascamSimulator;
}

class TascamSimulator : public QWidget
{
	Q_OBJECT

private:
	Ui::TascamSimulator *ui;

public:
	explicit TascamSimulator(QWidget *parent = 0);
	~TascamSimulator();

signals:
	void buttonEvent(uint id, QString name, bool presssed);
	void sliderEvent(uint id, QString name, qreal value);

private slots:

	void on_pushButtonPLAY_pressed();
	void on_pushButtonPLAY_released();

	void on_pushButtonRECORD_pressed();
	void on_pushButtonRECORD_released();

	void on_pushButtonSTOP_pressed();
	void on_pushButtonSTOP_released();

	void on_pushButtonAux1_pressed();
	void on_pushButtonAux1_released();

	void on_pushButtonAux2_pressed();
	void on_pushButtonAux2_released();

	void on_pushButtonAux3_pressed();
	void on_pushButtonAux3_released();

	void on_pushButtonAux4_pressed();
	void on_pushButtonAux4_released();



	void on_pushButtonASGN_pressed();
	void on_pushButtonASGN_released();

	void on_pushButtonF1_pressed();
	void on_pushButtonF1_released();

	void on_pushButtonF2_pressed();
	void on_pushButtonF2_released();

	void on_pushButtonF3_pressed();
	void on_pushButtonF3_released();
	void on_dialBig_valueChanged(int value);
};

#endif // TASCAMSIMULATOR_HPP
