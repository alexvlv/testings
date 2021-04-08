/*
$Id$
*/

#include "macro.h"

#include <QCoreApplication>
#include <QDebug>



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "Hello, world!";
    qDebug() << xstr(SIGN(5));
    exit(0);
    return a.exec();
}
