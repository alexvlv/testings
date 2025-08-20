#include "InputQAudioSource.h"

#include <QAudioInput>
#include <QMediaDevices>
#include <QAudioSource>

#include <QDebug>

//-------------------------------------------------------------------------
InputQAudioSource::InputQAudioSource(QString fname, QObject *parent)
	: QObject{parent}
{
	QAudioDevice device(QMediaDevices::defaultAudioInput());
	qDebug() << "AudioDeviceIn " << device.description();

	QAudioFormat audioFormat;
	audioFormat.setSampleRate(8000);
	audioFormat.setChannelCount(1);
	audioFormat.setSampleFormat(QAudioFormat::Int16);

	QAudioDevice info = QMediaDevices::defaultAudioInput();
	 if (!info.isFormatSupported(audioFormat)) {
		 qWarning() << "Default format not supported, trying to use the nearest.";
	}

	destinationFile.setFileName(fname);
	if(!destinationFile.open( QIODevice::WriteOnly | QIODevice::Truncate )) {
		qCritical() << "Can not open input file!";
		exit(1);
	}
	mAudioIn = new QAudioSource(device, audioFormat);
	connect(mAudioIn, &QAudioSource::stateChanged, this, &InputQAudioSource::handleStateChanged);
	mAudioIn->start(&destinationFile);
	//mAudioDeviceIn = mAudioIn->start();

}
//-------------------------------------------------------------------------
void InputQAudioSource::handleStateChanged(QAudio::State state)
{
	//qDebug() << status;
	qDebug() << __PRETTY_FUNCTION__ << "state:" << state << "err:" << mAudioIn->error();
}
//-------------------------------------------------------------------------
