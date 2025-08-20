#include "PlayQAudioSink.h"

#include <QFile>
#include <QAudioSink>
#include <QMediaDevices>

#include <QDebug>

//-------------------------------------------------------------------------
PlayQAudioSink::PlayQAudioSink(QString fname, QObject *parent)
	: QObject{parent}
{
	sourceFile.setFileName(fname);
	sourceFile.open(QIODevice::ReadOnly);

	QAudioFormat format;
	// Set up the format, eg.
	format.setSampleRate(8000);
	format.setChannelCount(1);
	//format.setSampleFormat(QAudioFormat::UInt8);
	format.setSampleFormat(QAudioFormat::Int16);

	QAudioDevice info(QMediaDevices::defaultAudioOutput());
	if (!info.isFormatSupported(format)) {
		qWarning() << "Raw audio format not supported by backend, cannot play audio.";
		return;
	}

	audio = new QAudioSink(format, this);
	connect(audio, &QAudioSink::stateChanged, this, &PlayQAudioSink::handleStateChanged);
	audio->start(&sourceFile);
	qDebug() << __PRETTY_FUNCTION__ << "START!" << "err:" << audio->error();
}
//-------------------------------------------------------------------------
void PlayQAudioSink::handleStateChanged(QAudio::State state)
{
	//qDebug() << status;
	qDebug() << __PRETTY_FUNCTION__ << "state:" << state << "err:" << audio->error();
}
//-------------------------------------------------------------------------

