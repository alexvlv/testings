#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QObject>
#include <QString>
#include <QMediaPlayer>

//-------------------------------------------------------------------------
class MediaPlayer : public QObject
{
    Q_OBJECT
public:
	explicit MediaPlayer(QString fname, QObject *parent = nullptr);

signals:

private:
	//static const QString Fname;
    QMediaPlayer *player;

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus);

};
//-------------------------------------------------------------------------
#endif // MEDIAPLAYER_H
