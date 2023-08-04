#ifndef SENDER_H
#define SENDER_H

#include "Structs.h"

#include <QObject>

class Receiver;

class Sender : public QObject
{
	Q_OBJECT
public:
	explicit Sender(Receiver *receiver, QObject *parent = nullptr);

signals:
	int signal(int v);
	int signal_1(int v);
	uint16_t signal_ex(TaskType t, uint8_t *data = nullptr, uint16_t u = 0);

public slots:
	void start();

private:
	Receiver *receiver;
};

#endif // SENDER_H
