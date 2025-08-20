#ifndef INPUTQAUDIOSOURCE_H
#define INPUTQAUDIOSOURCE_H

#include <QObject>
#include <QFile>
#include <QAudio>

class QAudioSource;

//-------------------------------------------------------------------------
class InputQAudioSource : public QObject
{
	Q_OBJECT
public:
	explicit InputQAudioSource(QString fname, QObject *parent = nullptr);

signals:

private slots:
	void handleStateChanged(QAudio::State state);

private:
	QAudioSource *mAudioIn;
	QIODevice *mAudioDeviceIn;
	QFile destinationFile;


};
//-------------------------------------------------------------------------
#endif // INPUTQAUDIOSOURCE_H
