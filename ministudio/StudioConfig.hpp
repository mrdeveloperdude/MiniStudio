#ifndef STUDIOCONFIG_HPP
#define STUDIOCONFIG_HPP

#include <QWidget>


namespace Ui
{
class StudioConfig;
}

class QLineEdit;
class QSettings;

class StudioConfig : public QWidget
{
	Q_OBJECT
private:
	Ui::StudioConfig *ui;
	QPixmap mLastPreviewPixmap;
	bool mDidFirstPlay;
	QSettings *mSettings;


public:
	explicit StudioConfig(QWidget *parent = nullptr);
	~StudioConfig();

public:
	QString projectName();
	QString title();
	QString subTitle();

	QSettings *settings();

protected:
	void resizeEvent(QResizeEvent *) override;

public slots:

	void onPreviewUpdated(quint64 id, QSharedPointer<QImage> img);

private slots:
	void on_pushButtonQuit_clicked();

	void on_pushButtonSimulator_clicked();

	void on_pushButtonLog_toggled(bool checked);

private:

	void saveLineEdit(QLineEdit &le);
	void loadLineEdit(QLineEdit &le);
	void saveSettings();
	void loadSettings();

signals:

	void quitApp();
	void textChanged();
	void showSimulator();

	void verbosity(bool );
};

#endif // STUDIOCONFIG_HPP
