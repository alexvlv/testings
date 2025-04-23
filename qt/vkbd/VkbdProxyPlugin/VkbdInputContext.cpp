#include "VkbdInputContext.h"

#include <QDebug>
#include <QGuiApplication>
#include <QEvent>
#include <QWidget>

//-------------------------------------------------------------------------
VkbdInputContext::VkbdInputContext()
	: QPlatformInputContext()
{

}
//-------------------------------------------------------------------------
VkbdInputContext::~VkbdInputContext()
{
	qDebug() << __PRETTY_FUNCTION__  << "...";
	qDebug() << __PRETTY_FUNCTION__  << "End!";
}
//-------------------------------------------------------------------------
QRectF VkbdInputContext::keyboardRect() const
{
	return QRectF();
}
//-------------------------------------------------------------------------
void VkbdInputContext::showInputPanel()
{
	//qDebug()<< __PRETTY_FUNCTION__ << m_focusItem;
	//if(view != nullptr) view->show();
	QPlatformInputContext::showInputPanel();
	emitInputPanelVisibleChanged();
}
//-------------------------------------------------------------------------
void VkbdInputContext::hideInputPanel()
{
	//qDebug()<< __PRETTY_FUNCTION__ << view;
	//if(view != nullptr) view->hide();
	QPlatformInputContext::hideInputPanel();
	emitInputPanelVisibleChanged();
}
//-------------------------------------------------------------------------
bool VkbdInputContext::isInputPanelVisible() const
{
	//bool fl = (view)?view->isVisible():false;
	bool fl =false;
	qDebug()<< __PRETTY_FUNCTION__ << fl;
	return fl;
}
//-------------------------------------------------------------------------
void VkbdInputContext::setFocusObject(QObject *object)
{
	if(!object) return;
	QWidget *widget = qobject_cast<QWidget *>(object);
	if(!widget) return;
	QWidget *window = widget->window();
	qDebug()<< __PRETTY_FUNCTION__ << widget << window;
	m_focusItem = object;
	QPlatformInputContext::setFocusObject(object);
}
//-------------------------------------------------------------------------
