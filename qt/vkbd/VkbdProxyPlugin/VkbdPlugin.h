#ifndef VKBDPLUGIN_H
#define VKBDPLUGIN_H

#include <qpa/qplatforminputcontextplugin_p.h>
#include <QtPlugin>


QT_BEGIN_NAMESPACE

class VkbdPlugin : public QPlatformInputContextPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE "VkbdProxyPlugin.json")

public:
	explicit VkbdPlugin(QObject *parent = nullptr);

private:
	QPlatformInputContext* create(const QString &key, const QStringList &paramList) override;
};

QT_END_NAMESPACE

#endif // VKBDPLUGIN_H
