/*
$Id$
*/

#include "macro.h"
#include ".git.h"

#include <QCoreApplication>
#include <QDebug>



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "Hello, world!";
    qDebug() << "GIT Rev.: " VERSION " Compiled: " __DATE__  " " __TIME__;
    qDebug() << xstr(SIGN(5));
    exit(0);
    return a.exec();
}
