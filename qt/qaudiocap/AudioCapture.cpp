#include "AudioCapture.h"

#include <QDebug>
#include <QMediaCaptureSession>
#include <QAudioOutput>
#include <QAudioInput>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QMediaFormat>
#include <QMediaRecorder>
#include <QUrl>

//-------------------------------------------------------------------------
AudioCapture::AudioCapture(QString fname, QObject *parent)
	: QObject{parent}, m_MediaCap(new QMediaCaptureSession(this))
{
	//QAudioOutput *audioOutput = new QAudioOutput(QMediaDevices::defaultAudioOutput());
	//m_MediaCap->setAudioOutput(audioOutput);
	QAudioInput *audioInput = new QAudioInput(QMediaDevices::defaultAudioInput());
	m_MediaCap->setAudioInput(audioInput);
	QMediaRecorder *recorder = new QMediaRecorder;
	qDebug() << recorder->mediaFormat().supportedAudioCodecs(QMediaFormat::Encode);
	qDebug() << recorder->mediaFormat().supportedFileFormats(QMediaFormat::Encode);
	QMediaFormat fmt(QMediaFormat::Wave);
	recorder->setMediaFormat(fmt);
	recorder->setOutputLocation(QUrl::fromLocalFile(fname));
	qDebug() << recorder->recorderState() << recorder->actualLocation();
	m_MediaCap->setRecorder(recorder);
	recorder->record();
	qDebug() << recorder->recorderState() << recorder->actualLocation();
	qDebug() << recorder->mediaFormat().audioCodec() << recorder->mediaFormat().fileFormat()  ;

}
//-------------------------------------------------------------------------
