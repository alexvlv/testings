#include ".git.h"
#include "MainWindow.h"

#include <QDebug>
#include <QApplication>
#include <QWindow>

int main(int argc, char *argv[])
{
	qInfo()<< APPNAME " GIT rev.: " GIT_REV " at " GIT_DATE " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP " UTC]";
	QApplication a(argc, argv);
	MainWindow w;
	QApplication::setFont(w.font());
	//w.setWindowState(Qt::WindowMaximized);
	w.setWindowState(Qt::WindowFullScreen);
	w.show();
	return a.exec();
}
