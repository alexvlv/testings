#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QtWidgets/qpushbutton.h>

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
	void onKeyboard(int);

private:
	Ui::MainWindow *ui;
	KbdWidget *kbdw = nullptr;
	KeysManager *keyb = nullptr;
	QList<QPushButton *> kbtns;
};

#endif // MAINWINDOW_H
