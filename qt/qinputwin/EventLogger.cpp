#include "EventLogger.h"
#include <QDebug>
#include <QEvent>

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
		break;
	default:
		qDebug()<< "Event on" << obj->objectName() << ":" << event->type();
		break;
	}
	return QObject::eventFilter(obj, event);
}
//-------------------------------------------------------------------------
