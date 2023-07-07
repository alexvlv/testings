#include "MediaPlayer.h"

#include <QDebug>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QCoreApplication>
#include <QDir>

//-------------------------------------------------------------------------
//const QString MediaPlayer::Fname = "s16le_mono_8KHz.lpcm";
// ffmpeg -i s16le_mono_8KHz.wav -f s16le -acodec pcm_s16le s16le_mono_8KHz.lpcm
//const QString MediaPlayer::Fname = "sample_22_frames.mp3";
const QString MediaPlayer::Fname = "audio8k16S.wav";
//-------------------------------------------------------------------------
MediaPlayer::MediaPlayer(QObject *parent)
    : QObject{parent},player(new QMediaPlayer(this))
{
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
    QAudioOutput *audioOutput = new QAudioOutput(QMediaDevices::defaultAudioOutput());
    player->setAudioOutput(audioOutput);
    player->setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+QDir::toNativeSeparators("/") +Fname));
    player->setLoops(QMediaPlayer::Infinite);
    player->play();
}
//-------------------------------------------------------------------------
void MediaPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    //qDebug() << status;
    qDebug()<<"Media Status Changed:" << player->mediaStatus() << " " << player->error();
}
//-------------------------------------------------------------------------
