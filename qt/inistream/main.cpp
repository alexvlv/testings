#include <QCoreApplication>
#include <QDebug>
#include "IniStreamConfig.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	IniStreamConfig cfg;

	// Simulate receiving a UDP chunk
	cfg.parse(R"(
[cam0]
URL=http://example.com
Path=C:\Data
enabled=yes
)");

	// Iterate all keys dynamically
	qDebug() << "All keys in [cam0]:";
	for (const QString &key : cfg.keys("cam0")) {
		qDebug() << key << "->" << cfg.get("cam0", key).toString();
	}

	// Simulate a UDP update removing a key
	cfg.parse(R"(
[cam0]
URL=
)");

	qDebug() << "\nAfter deletion:";
	for (const QString &key : cfg.keys("cam0")) {
		qDebug() << key << "->" << cfg.get("cam0", key).toString();
	}

	return 0;
}
