#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QtWidgets/qpushbutton.h>

class QLabel;
class QStatusBar;

namespace Ui {
class MainWindow;
}

class KbdWidget;
class KeysManager;

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

public slots:
	void onKeyButtonClicked(int);

private:
	Ui::MainWindow *ui;
	QLabel *lblHostInfo;
	QPushButton *btnConnectDisconnect;
	QStatusBar *mBar;
	KbdWidget *kbdw = nullptr;
	KeysManager *keyb = nullptr;
	QList<QPushButton *> kbtns;
};

#endif // MAINWINDOW_H
