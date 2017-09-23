#ifndef RICHEDIT_HPP
#define RICHEDIT_HPP

#include <QMainWindow>
#include <QMap>
#include <QPointer>

class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
class QPrinter;
class QToolBar;

class RichEdit : public QWidget
{
		Q_OBJECT
	private:

		QComboBox *comboStyle;
		QFontComboBox *comboFont;
		QComboBox *comboSize;

		QToolBar *tb;
		QString fileName;
		QTextEdit *textEdit;


	private:


		QAction *actionTextBold;
		QAction *actionTextUnderline;
		QAction *actionTextItalic;
		QAction *actionTextColor;
		QAction *actionAlignLeft;
		QAction *actionAlignCenter;
		QAction *actionAlignRight;
		QAction *actionAlignJustify;

	public:
		RichEdit(QWidget *parent = 0);

		bool load(const QString &f);


	protected:
		virtual void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

	private slots:
		void textBold();
		void textUnderline();
		void textItalic();
		void textFamily(const QString &f);
		void textSize(const QString &p);
		void textStyle(int styleIndex);
		void textColor();
		void textAlign(QAction *a);

		void currentCharFormatChanged(const QTextCharFormat &format);
		void cursorPositionChanged();

	private:
		bool maybeSave(){return true;}
		void setCurrentFileName(const QString &fileName);

		void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
		void fontChanged(const QFont &f);
		void colorChanged(const QColor &c);
		void alignmentChanged(Qt::Alignment a);


	private:
		QToolBar * addToolBar(QString name);

		void setupTextActions();

	public:

		QString text();
		void setText(QString);

	signals:
		void textChanged();

};


#endif // RICHEDIT_HPP
