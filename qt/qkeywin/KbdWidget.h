#ifndef KBDWIDGET_H
#define KBDWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QAction>


namespace Ui {
class KbdWidget;
}

class KbdWidget : public QWidget
{
	Q_OBJECT

public:
	explicit KbdWidget(QWidget *parent = nullptr);
	~KbdWidget();

	void setClients(QList<QObject *> list);
	void setEnabled(QList<int> keys, bool ena = true);
	void disableAll();

signals:
	void activated(int);

private:
	void client_activated(QObject *);

	Ui::KbdWidget *ui;

	static const unsigned NUM_KEYS = (5+12);

	QList<QPushButton *> fbtns;
	typedef QList<QPushButton *>::Iterator ButtonIterator;
	QVector<QAction *> actions;
	typedef QVector<QAction *>::Iterator ActionIterator;

	//QVector<QObject *> clients;
	//typedef QVector<QObject *>::Iterator ClientIterator;
};

#endif // KBDWIDGET_H
