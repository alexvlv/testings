#ifndef PLAYWAV_H
#define PLAYWAV_H

#include <QObject>
#include <QString>

class QSoundEffect;

//-------------------------------------------------------------------------
class PlayWav : public QObject
{
    Q_OBJECT
public:
	explicit PlayWav(QString fname, QObject *parent = nullptr);

signals:


private:
	//static const QString Fname;
    QSoundEffect *effect;

private slots:
    void onLoadedChanged();
    void onPlayingChanged();
    void onLoopsRemainingChanged();
};
//-------------------------------------------------------------------------
#endif // PLAYWAV_H
