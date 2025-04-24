#include "WidgetKey.h"
#include "ui_WidgetKey.h"

#include <QWindow>
#include <QKeyEvent>

//-------------------------------------------------------------------------
WidgetKey::WidgetKey(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::WidgetKey)
{
	ui->setupUi(this);
	//setFlags(flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint	| Qt::WindowDoesNotAcceptFocus);
	installEventFilter(this);
	setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	setWindowState(Qt::WindowFullScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_TranslucentBackground);
	qDebug()<< __PRETTY_FUNCTION__ << "Done" << this << windowHandle();
}
//-------------------------------------------------------------------------
WidgetKey::~WidgetKey()
{
	delete ui;
}
//-------------------------------------------------------------------------
void WidgetKey::setFocusItem(QObject *focusItem)
{ 	m_focusItem = focusItem; }
//-------------------------------------------------------------------------
void WidgetKey::showEvent(QShowEvent *event)
{
	//qDebug()<< __PRETTY_FUNCTION__ << pos() << windowHandle()->position() << windowHandle()->parent();
	QSurfaceFormat f(windowHandle()->format());
	f.setAlphaBufferSize(8);
	windowHandle()->setFormat(f);
	windowHandle()->setFlags(windowHandle()->flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint	| Qt::WindowDoesNotAcceptFocus);
	windowHandle()->setPosition(0,0);
	qDebug()<< __PRETTY_FUNCTION__ << pos() << windowHandle()->position() << windowHandle()->parent();

}
//-------------------------------------------------------------------------
bool WidgetKey::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type()) {
	case QEvent::MouseMove:
	case QEvent::Paint:
	case QEvent::HoverMove:
	case QEvent::UpdateRequest:
	case QEvent::ShortcutOverride:
		break;
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
		processKeyEvent(obj, event);
		break;
	default:
		qDebug() << "W Event on" << obj->objectName() << ":" << event->type();
		break;
	}
	return QObject::eventFilter(obj, event);
}
//-------------------------------------------------------------------------
void WidgetKey::processKeyEvent(QObject *obj, QEvent *event)
{
	QKeyEvent *evk = dynamic_cast<QKeyEvent *>(event);
	Q_ASSERT(evk);
	qDebug() << "W Event on" << obj->objectName() << ":" << event->type() << Qt::hex << evk->key() << evk->nativeScanCode() << evk->text();

}
//-------------------------------------------------------------------------
void WidgetKey::closeEvent(QCloseEvent *event)
{
	qDebug()<< __PRETTY_FUNCTION__;
	QWidget *w = qobject_cast<QWidget *>(m_focusItem);
	qDebug()<< __PRETTY_FUNCTION__ << w;
	if(w) w->clearFocus();
	event->accept();
}
//-------------------------------------------------------------------------
void WidgetKey::hideEvent(QHideEvent *event)
{
	qDebug()<< __PRETTY_FUNCTION__;
	QWidget::hideEvent(event);
	//QWidget *w = qobject_cast<QWidget *>(m_focusItem);
	//qDebug()<< __PRETTY_FUNCTION__ << w;
	//if(w) w->clearFocus();
}
//-------------------------------------------------------------------------
void WidgetKey::on_pushButton_clicked()
{
	qDebug()<< __PRETTY_FUNCTION__;
	assert(m_focusItem);
	QInputMethodEvent ev;
	ev.setCommitString("X");
	QCoreApplication::sendEvent(m_focusItem,&ev);
	QCoreApplication::processEvents();
	//QWidget *w = qobject_cast<QWidget *>(m_focusItem);
	//assert(w);
	//qDebug()<< __PRETTY_FUNCTION__ << w;
	//w->clearFocus();
	hide();
}
//-------------------------------------------------------------------------
