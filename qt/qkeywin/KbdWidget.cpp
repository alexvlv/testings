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
}
//-------------------------------------------------------------------------
KbdWidget::~KbdWidget()
{
	delete ui;
}
//-------------------------------------------------------------------------
void KbdWidget::setClients(QList<QWidget *> list)
{
	clients = list;
	//for (int i; i < list.size(); i++) {
	QList<QPushButton *>::iterator it = fbtns.begin();
	foreach (QWidget *wcl, clients) {
		connect(*it++, &QPushButton::clicked,
					this, [this, wcl]() { this->client_activated(wcl); });
	}
	for(QList<QPushButton *>::iterator ib = it; ib != fbtns.end(); ++ib) (*ib)->setEnabled(false);
}
//-------------------------------------------------------------------------
void KbdWidget::client_activated(QWidget *client)
{
	qDebug()<< __PRETTY_FUNCTION__ << client;
}
//-------------------------------------------------------------------------
