#ifndef RECEIVER_H
#define RECEIVER_H

#include "Structs.h"

#include <QObject>

#include <stdint.h>

class Receiver : public QObject
{
	Q_OBJECT
public:
	Receiver(QObject *parent = nullptr);

public slots:
	int slot(int v);
	int slot_1(int v);
	uint16_t slot_ex(TaskType t, uint8_t *data = nullptr, uint16_t u = 0);

};

#endif // RECEIVER_H
