#include "Presentation.hpp"
#include "ui_Presentation.h"

#include <QDebug>

Presentation::Presentation(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Presentation)
{
	ui->setupUi(this);
	connect(ui->textEditContent, &QTextEdit::textChanged, this, &Presentation::onTextChanged);
}

Presentation::~Presentation()
{
	delete ui;
}


void Presentation::onTextChanged()
{
	qDebug()<<"TEXT CHANGED";
}
