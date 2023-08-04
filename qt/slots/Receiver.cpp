#include "Receiver.h"

#include <QDebug>

//-------------------------------------------------------------------------
Receiver::Receiver(QObject *parent)
	: QObject{parent}
{

}
//-------------------------------------------------------------------------
int Receiver::slot(int v) //[slot]
{
	qDebug() << __PRETTY_FUNCTION__ << v << sender();
	return -v;
}
//-------------------------------------------------------------------------
int Receiver::slot_1(int v) //[slot]
{
	qDebug() << __PRETTY_FUNCTION__ << v << sender();
	return -v;
}
//-------------------------------------------------------------------------
uint16_t Receiver::slot_ex(TaskType t, uint8_t *data, uint16_t u) //[slot]
{
	qDebug() << __PRETTY_FUNCTION__ << (int)t << u  << (void *)data << sender();
	return (uint16_t)t;
}
//-------------------------------------------------------------------------

