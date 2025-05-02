#include "KbdWidget.h"
#include "ui_KbdWidget.h"

#include <cassert>

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
	fbtns.at(0)->setFocus(Qt::ActiveWindowFocusReason);
#if 0
	actions = QVector<QAction *>(NUM_KEYS, nullptr);
	int i=0;
	for (ActionIterator it = actions.begin(); it !=  actions.end(); it++, i++) {
		*it = new QAction(this);
		(*it)->setEnabled(true);
		qDebug()<< __PRETTY_FUNCTION__ << i;
		//(*it)->setShortcut(Qt::Key_F11+i);
		connect(*it, &QAction::triggered, this, [this, i]() {
			qDebug()<< __PRETTY_FUNCTION__ << i;
			emit activated(i);
		});
		addAction(*it);
	}
#endif
	//for( int i=0; i<NUM_KEYS; i++ ) actions.at(i)->setShortcut(Qt::Key_F1+i);
	//for( int i=0; i<10; i++ ) actions.at(i+5)->setShortcut(Qt::Key_F11+i);
	//actions.at(5)->setShortcut(Qt::Key_F11);
	//actions.at(6)->setShortcut(Qt::Key_F12);
	//ctions.at(7)->setShortcut(Qt::Key_F13);
	//actions.at(8)->setShortcut(Qt::Key_F14);

	//actions.at(15)->setShortcut(Qt::Key_F10);

	//ActionIterator it = actions.begin();
	//foreach (QPushButton *btn, fbtns) {
	//	connect(btn, &QPushButton::clicked,*it++,&QAction::trigger);
	//}

	//{ int i=0; foreach (QPushButton *btn, fbtns) btn->setShortcut(int(Qt::Key_F11 + i++) ); }
	//fbtns[0]->setShortcut(Qt::Key_SysReq);
	//fbtns[1]->setShortcut(Qt::Key_ScrollLock);
	//fbtns[2]->setShortcut(Qt::Key_Pause);
	//fbtns[3]->setShortcut(Qt::Key_F11);
	//fbtns[4]->setShortcut(Qt::Key_F12);

	//EventLogger *ev = new EventLogger(this);
	//ev->setObjectName(objectName());
	//installEventFilter(ev);
}
//-------------------------------------------------------------------------
KbdWidget::~KbdWidget()
{
	delete ui;
}
//-------------------------------------------------------------------------
void KbdWidget::disableAll()
{
	for(ActionIterator it = actions.begin(); it != actions.end(); ++it) {
		(*it)->setEnabled(false);
		(*it)->setText("");
	}
}
//-------------------------------------------------------------------------
void KbdWidget::setEnabled(QList<int> keys, bool ena)
{
	foreach (int i, keys) {
		assert(i<NUM_KEYS);
		actions.at(i)->setEnabled(ena);
	}
}
//-------------------------------------------------------------------------
void KbdWidget::setClients(QList<QObject *> list)
{
	/*
	for(ActionIterator it = actions.begin(); it != actions.end(); ++it) {
		(*it)->setEnabled(false);
		(*it)->setText("");
		//(*it)->disconnect();
	}
*/
	ActionIterator it = actions.begin();
	foreach (QObject *cl, list) {
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
