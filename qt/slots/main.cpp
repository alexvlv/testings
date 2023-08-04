#include ".git.h"

#include "Sender.h"
#include "Receiver.h"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    qInfo()<< APPNAME " GIT rev.: " GIT_REV " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP "]";
	QCoreApplication a(argc, argv);
	Receiver r;
	Sender s(&r);
    return a.exec();
}
