#ifndef PRESENTATION_HPP
#define PRESENTATION_HPP

#include <QWidget>

namespace Ui {
	class Presentation;
}

class Presentation : public QWidget
{
		Q_OBJECT

	public:
		explicit Presentation(QWidget *parent = 0);
		~Presentation();

	private:
		Ui::Presentation *ui;

	public slots:

		void onTextChanged();
};

#endif // PRESENTATION_HPP
