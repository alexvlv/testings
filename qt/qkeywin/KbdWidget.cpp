#include "KbdWidget.h"
#include "ui_KbdWidget.h"

//-------------------------------------------------------------------------
KbdWidget::KbdWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::KbdWidget)
{
	ui->setupUi(this);
	//fbtns.insert({nullptr, nullptr});
	//qDebug()<< __PRETTY_FUNCTION__ << findChildren<QPushButton *>();
	fbtns = findChildren<QPushButton *>();
	qDebug()<< __PRETTY_FUNCTION__ << findChildren<QPushButton *>();

	actions = QVector<QAction *>(NUM_KEYS, nullptr);
	for (ActionIterator it = actions.begin(); it !=  actions.end(); it++) {
		*it = new QAction(this);
	}

	ActionIterator it = actions.begin();
	foreach (QPushButton *btn, fbtns) {
		connect(btn, &QPushButton::clicked,*it++,&QAction::trigger);
	}
}
//-------------------------------------------------------------------------
KbdWidget::~KbdWidget()
{
	delete ui;
}
//-------------------------------------------------------------------------
void KbdWidget::setClients(QList<QObject *> list)
{
	clients = list;
	for(ActionIterator it = actions.begin(); it != actions.end(); ++it) {
		(*it)->setEnabled(false);
		(*it)->setText("");
	}

	ActionIterator it = actions.begin();
	foreach (QObject *cl, clients) {
		if(cl) {
			connect(*it, &QAction::triggered, this, [this, cl]() { this->client_activated(cl); });
			(*it)->setEnabled(true);
		}
		it++;
	}

	it = actions.begin();
	for(ButtonIterator ib = fbtns.begin(); ib != fbtns.end(); ++ib) (*ib)->setEnabled((*it++)->isEnabled());
}
//-------------------------------------------------------------------------
void KbdWidget::client_activated(QObject *client)
{
	qDebug()<< __PRETTY_FUNCTION__ << client << sender();
}
//-------------------------------------------------------------------------
