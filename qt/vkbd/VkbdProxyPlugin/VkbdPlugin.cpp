
#include ".git.h"

#include "VkbdPlugin.h"
#include "VkbdInputContext.h"
#include <QDebug>


VkbdPlugin::VkbdPlugin(QObject *parent)
	: QPlatformInputContextPlugin(parent)
{
}

QPlatformInputContext* VkbdPlugin::create(const QString &key, const QStringList &paramList)
{
	Q_UNUSED(paramList);
	qDebug()<< __PRETTY_FUNCTION__ << key << paramList;

	if (key == QLatin1String("vkbdp")) {
		qInfo()<< APPNAME " GIT rev.: " GIT_REV " at " GIT_DATE " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP " UTC]";
		return new VkbdInputContext;
	}
	//static_assert(false, "You need to implement this function");
	return nullptr;
}

/*
QT_IM_MODULE=vkbd /mnt/nfs/qthello
lsof +D /usr/lib/plugins
lsof -c qthello
Install path:
/usr/lib/plugins/platforminputcontexts
*/
