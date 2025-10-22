
#include "macro.h"
#include ".git.h"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInfo()<< APPNAME " GIT rev.: " GIT_REV " at " GIT_DATE " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP "]";
    qDebug() << xstr(SIGN(5));
    exit(0);
    return a.exec();
}
