#include "EventLogger.h"
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

//-------------------------------------------------------------------------
EventLogger::EventLogger(QObject *parent)
	: QObject{parent}
{

}
//-------------------------------------------------------------------------
bool EventLogger::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type()) {
	case QEvent::MouseMove:
	case QEvent::Paint:
	case QEvent::HoverMove:
	case QEvent::UpdateRequest:
	case QEvent::ShortcutOverride:
		break;	case QEvent::KeyPress:
	case QEvent::KeyRelease:
		processKeyEvent(obj, event);
		break;
	default:
		qDebug() << "Event on" << obj->objectName() << ":" << event->type();
		break;
	}
	return QObject::eventFilter(obj, event);
}
//-------------------------------------------------------------------------
void EventLogger::processKeyEvent(QObject *obj, QEvent *event)
{
	QKeyEvent *evk = dynamic_cast<QKeyEvent *>(event);
	Q_ASSERT(evk);
	qDebug() << "Event on" << obj->objectName() << ":" << event->type() << Qt::hex << evk->key() << evk->nativeScanCode() << evk->text();

}
//-------------------------------------------------------------------------
