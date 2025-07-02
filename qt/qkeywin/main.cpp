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
	QFont f = w.font();
#ifndef Q_PROCESSOR_ARM
	f.setPointSize(f.pointSize()/2);
#endif
	w.setFont(f);
	QApplication::setFont(w.font());
#ifdef Q_PROCESSOR_ARM
	//w.setWindowState(Qt::WindowMaximized);
	w.setWindowState(Qt::WindowFullScreen);
#endif
	w.show();
	return a.exec();
}
