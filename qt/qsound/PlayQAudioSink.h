#ifndef PLAYQAUDIOSINK_H
#define PLAYQAUDIOSINK_H

#include <QObject>
#include <QAudio>
#include <QFile>
#include <QAudioSink>

//-------------------------------------------------------------------------
class PlayQAudioSink : public QObject
{
	Q_OBJECT
public:
	explicit PlayQAudioSink(QString fname, QObject *parent = nullptr);

signals:

private slots:
	void handleStateChanged(QAudio::State state);

private:
	QFile sourceFile;   // class member.
	QAudioSink* audio; // class member.

};
//-------------------------------------------------------------------------

#endif // PLAYQAUDIOSINK_H
