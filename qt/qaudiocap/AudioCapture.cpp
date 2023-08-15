#include "AudioCapture.h"

#include <QMediaCaptureSession>
#include <QAudioOutput>
#include <QAudioInput>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QMediaRecorder>

//-------------------------------------------------------------------------
AudioCapture::AudioCapture(QObject *parent)
	: QObject{parent}, m_MediaCap(new QMediaCaptureSession(this))
{
	QAudioOutput *audioOutput = new QAudioOutput(QMediaDevices::defaultAudioOutput());
	m_MediaCap->setAudioOutput(audioOutput);
	QAudioInput *audioInput = new QAudioInput(QMediaDevices::defaultAudioInput());
	m_MediaCap->setAudioInput(audioInput);
	//QMediaRecorder *recorder = new QMediaRecorder;
	//m_MediaCap->setRecorder(recorder);

}
//-------------------------------------------------------------------------
