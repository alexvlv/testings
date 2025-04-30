#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

namespace Ui {
class MainWindow;
}

class KbdWidget;

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
};

#endif // MAINWINDOW_H
