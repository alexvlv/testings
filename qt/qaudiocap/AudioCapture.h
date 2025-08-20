#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <QObject>

class QMediaCaptureSession;

//-------------------------------------------------------------------------
class AudioCapture : public QObject
{
	Q_OBJECT
public:
	explicit AudioCapture(QString fname, QObject *parent = nullptr);

signals:

public slots:

private:
	QMediaCaptureSession *m_MediaCap;
};
//-------------------------------------------------------------------------

#endif // AUDIOCAPTURE_H
