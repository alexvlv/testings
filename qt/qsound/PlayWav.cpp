#include "PlayWav.h"

#include <QDebug>
#include <QSoundEffect>
#include <QCoreApplication>
#include <QDir>

//-------------------------------------------------------------------------
//const char*  PlayWav::Fname = "/unit_tests/ASRC/audio8k16S.wav";
//const QString PlayWav::Fname = "audio8k16S.wav";
//-------------------------------------------------------------------------
PlayWav::PlayWav(QString fname, QObject *parent)
    : QObject{parent},effect(new QSoundEffect(this))
{
    connect(effect, SIGNAL(loadedChanged()),SLOT(onLoadedChanged()),Qt::QueuedConnection);
    connect(effect, SIGNAL(playingChanged()),SLOT(onPlayingChanged()),Qt::QueuedConnection);
    connect(effect, SIGNAL(loopsRemainingChanged()),SLOT(onLoopsRemainingChanged()),Qt::QueuedConnection);
	effect->setSource(QUrl::fromLocalFile(fname));
    //effect->setLoopCount(QSoundEffect::Infinite);
    effect->setLoopCount(3);
    effect->setVolume(1.0f);
    effect->play();
    qDebug() << effect->source();
    qDebug() << effect->status();
    qInfo()<< "Start playing...";
	qDebug() << __PRETTY_FUNCTION__ << "START!";
}
//-------------------------------------------------------------------------
void PlayWav::onLoadedChanged()
{
    qDebug() << effect->status();
}
//-------------------------------------------------------------------------
void PlayWav::onPlayingChanged()
{
    qDebug() << "Playing:" << effect->isPlaying();
}
//-------------------------------------------------------------------------
void PlayWav::onLoopsRemainingChanged()
{
    qDebug() << "LoopsRemaining:" << effect->loopsRemaining();
}
//-------------------------------------------------------------------------
