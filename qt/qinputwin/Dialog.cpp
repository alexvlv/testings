#include "Dialog.h"
#include "ui_Dialog.h"
#include <QtCore/qtimer.h>
#include <QtGui/qscreen.h>
#include <QtGui/qwindow.h>

Dialog::Dialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dialog)
{
	ui->setupUi(this);
	QScreen *screen = QWidget::screen();
	if (screen) {
		qDebug()<< "SCREEN:" << screen->availableGeometry();
	}
	qDebug()<< __PRETTY_FUNCTION__ << pos();
	if (windowHandle()) qDebug()<<  windowHandle()->position();
	//QTimer::singleShot(100,this,&Dialog::on_SingleShot);
}

Dialog::~Dialog()
{
	delete ui;
}

void Dialog::on_SingleShot()
{
	//qDebug() << __PRETTY_FUNCTION__ ;
	//windowHandle()->setPosition(200,200);
	qDebug()<< __PRETTY_FUNCTION__ << pos() << windowHandle()->position() << windowHandle()->parent();
	//move(100,100);
	//windowHandle()->setPosition(0,0);
	QScreen *screen = QWidget::screen();
	if (screen) {
		qDebug()<< "SCREEN:" << screen->availableGeometry();
	}
}
//-------------------------------------------------------------------------
