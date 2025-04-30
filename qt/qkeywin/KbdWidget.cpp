#include "KbdWidget.h"
#include "ui_KbdWidget.h"

//#include <QToolBar>

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
	actions = QVector<QAction *>(10,new QAction(this));
	//QToolBar *fileToolBar = new QToolBar(this);
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
	//for (int i; i < list.size(); i++) {
	QList<QPushButton *>::iterator it = fbtns.begin();
	foreach (QObject *cl, clients) {
		if(*it) {
			connect(*it, &QPushButton::clicked, this, [this, cl]() { this->client_activated(cl); });
			(*it)->setEnabled(true);
		}
		it++;
	}
	for(QList<QPushButton *>::iterator ib = it; ib != fbtns.end(); ++ib) (*ib)->setEnabled(false);
}
//-------------------------------------------------------------------------
void KbdWidget::client_activated(QObject *client)
{
	qDebug()<< __PRETTY_FUNCTION__ << client;
}
//-------------------------------------------------------------------------
