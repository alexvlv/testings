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


private:
	void client_activated(QObject *);

	Ui::KbdWidget *ui;

	static const unsigned NUM_KEYS = 15;

	QList<QPushButton *> fbtns;
	QVector<QAction *> actions;
	QList<QObject *> clients;
};

#endif // KBDWIDGET_H
