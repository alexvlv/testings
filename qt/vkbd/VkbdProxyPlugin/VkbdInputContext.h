#ifndef VKBDINPUTCONTEXT_H
#define VKBDINPUTCONTEXT_H

#include <qpa/qplatforminputcontext.h>
#include <QRectF>

QT_BEGIN_NAMESPACE


//-------------------------------------------------------------------------
class VkbdInputContext : public QPlatformInputContext
{
	Q_OBJECT
public:
	explicit VkbdInputContext();
	~VkbdInputContext();

	bool isValid() const override { return true; }
	QRectF keyboardRect() const;

	void setFocusObject(QObject *object) override;

	void showInputPanel() override;
	void hideInputPanel() override;
	bool isInputPanelVisible() const override;

private:
	QObject * m_focusItem = nullptr;

};
//-------------------------------------------------------------------------
QT_END_NAMESPACE

#endif // VKBDINPUTCONTEXT_H
