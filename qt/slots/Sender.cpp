#include "Sender.h"
#include "Receiver.h"

#include <QDebug>
#include <QTimer>

//-------------------------------------------------------------------------
Sender::Sender(Receiver *receiver, QObject *parent)
	: QObject{parent},receiver(receiver)
{
	connect(this,SIGNAL(signal(int)),receiver,SLOT(slot(int)), Qt::QueuedConnection);
	connect(this,SIGNAL(signal_1(int)),receiver,SLOT(slot_1(int)), Qt::DirectConnection);
	connect(this,SIGNAL(signal_ex(TaskType, uint8_t *)),receiver,SLOT(slot_ex(TaskType)));
	QTimer::singleShot(100, this, SLOT(start()));
}
//-------------------------------------------------------------------------
void Sender::start() //[slot]
{
	int rv = 0;
	qDebug() << __PRETTY_FUNCTION__ << sender();
	//receiver->slot();
	//emit signal(33);
	rv = signal(33);
	qDebug() << __PRETTY_FUNCTION__ << "Qt::QueuedConnection" << rv;
	rv = signal_1(55);
	qDebug() << __PRETTY_FUNCTION__ << "Qt::QueuedConnection" << rv;
	rv = signal_ex(TaskType::StreamUniqueAliveAfterEos);
	qDebug() << __PRETTY_FUNCTION__ << "qRegisterMetaType Qt::DirectConnection" << rv;
}
//-------------------------------------------------------------------------
