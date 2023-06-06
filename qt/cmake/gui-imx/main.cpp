#include "mainwindow.h"

#include ".git.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qInfo()<< APPNAME " GIT rev.: " GIT_REV " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP "]";
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
