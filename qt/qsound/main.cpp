
#include ".git.h"

#include "PlayWav.h"
#include "MediaPlayer.h"
#include "PlayQAudioSink.h"

#include <QAudioDevice>
#include <QMediaDevices>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QDebug>

static const char *Version = "GIT rev.: " GIT_REV " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP "]";
const QString Fname = "audio8k16S.wav";
//-------------------------------------------------------------------------
static void playQSoundEffect(const QString &fname)
{
	static PlayWav play(fname);
}
//-------------------------------------------------------------------------
static void playQMediaPlayer(const QString &fname)
{
	static  MediaPlayer play(fname);
}
//-------------------------------------------------------------------------
static void playQAudioSink(const QString &fname)
{
	static  PlayQAudioSink play(fname);
}
//-------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	qInfo()<< APPNAME << Version;
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(Version);
	QCommandLineParser parser;
	parser.setApplicationDescription("Qt sound test application");
	parser.addHelpOption();
	parser.addVersionOption();
	uint idxEngine = 0;

	QString qsInFileName = QCoreApplication::applicationDirPath()+QDir::toNativeSeparators("/") +Fname;

	QCommandLineOption inFileOption({"f", "file"}, "input .wav file","infile");
	parser.addOption(inFileOption);

	QCommandLineOption engineOption({"e", "engine"}, "Sound engine used: \n1 - QSoundEffect \n2 - QMediaPlayer \n3 - QAudioSink","engine");
	parser.addOption(engineOption);

	parser.process(a);

	if(parser.isSet(inFileOption)) {
		qsInFileName = parser.value(inFileOption);
	} else {
		qInfo() << "Used default input file";
	}
	qInfo() << "Input file:" << qsInFileName;

	QAudioDevice info(QMediaDevices::defaultAudioOutput());
	qDebug() << info.supportedSampleFormats();
	qDebug() << info.preferredFormat();

	if(parser.isSet(engineOption)) {
		idxEngine = parser.value(engineOption).toUInt();
	}
	qDebug() << "Sound engine used:" << idxEngine;

	switch (idxEngine) {
	case 1:
		playQSoundEffect(qsInFileName);
		break;
	case 2:
		playQMediaPlayer(qsInFileName);
		break;
	case 3:
		playQAudioSink(qsInFileName);
		break;
	default:
		qCritical() << "No play engine selected, exiting";
		exit(1);
		break;
	}
	return a.exec();
}
//-------------------------------------------------------------------------
