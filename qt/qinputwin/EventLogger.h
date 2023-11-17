#ifndef EVENTLOGGER_H
#define EVENTLOGGER_H

#include <QObject>

class EventLogger : public QObject
{
	Q_OBJECT
public:
	explicit EventLogger(QObject *parent = nullptr);

signals:

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // EVENTLOGGER_H
