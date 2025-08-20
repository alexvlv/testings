
#include ".git.h"
#include "AudioCapture.h"
#include "InputQAudioSource.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>

#include <QDebug>

//-------------------------------------------------------------------------
static const char *Version = "GIT rev.: " GIT_REV " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP "]";
const QString Fname = "/mnt/nfs/input.raw";
//-------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    qInfo()<< APPNAME " GIT rev.: " GIT_REV " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP "]";
    QCoreApplication a(argc, argv);
	a.setApplicationVersion(Version);
	QCommandLineParser parser;
	parser.setApplicationDescription("Qt audio capture test application");
	parser.addHelpOption();
	parser.addVersionOption();
	QCommandLineOption outFileOption({"f", "file"}, "output .wav file","outfile");
	parser.addOption(outFileOption);
	parser.process(a);

	QString qsOutFileName = Fname;
	if(parser.isSet(outFileOption)) {
		qsOutFileName = parser.value(outFileOption);
	} else {
		qInfo() << "Used default output file";
	}
	qInfo() << "Output file:" << qsOutFileName;

	//AudioCapture acap(qsOutFileName);
	InputQAudioSource as(qsOutFileName);
	return a.exec();
}
//-------------------------------------------------------------------------

/*

aplay -v -c1 -r8000 -f S16_LE -t raw /mnt/nfs/input.raw

*/
