
#include ".git.h"

#include "PlayWav.h"
#include "MediaPlayer.h"

#include <QCoreApplication>
#include <QDebug>

//-------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    qInfo()<< APPNAME " GIT rev.: " GIT_REV " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP "]";
    QCoreApplication a(argc, argv);
    //PlayWav play;
    MediaPlayer play;
    return a.exec();
}
//-------------------------------------------------------------------------
