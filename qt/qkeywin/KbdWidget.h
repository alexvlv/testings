#ifndef KBDWIDGET_H
#define KBDWIDGET_H

#include <QWidget>
#include <QPushButton>


namespace Ui {
class KbdWidget;
}

class KbdWidget : public QWidget
{
	Q_OBJECT

public:
	explicit KbdWidget(QWidget *parent = nullptr);
	~KbdWidget();

	void setClients(QList<QWidget *> list);


private:
	void client_activated(QWidget *);

	Ui::KbdWidget *ui;
	QList<QPushButton *> fbtns;
	QList<QWidget *> clients;
};

#endif // KBDWIDGET_H
