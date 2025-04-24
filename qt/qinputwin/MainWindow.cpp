#include ".git.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "EventLogger.h"

#include <QWindow>
#include <QTimer>
#include <QMessageBox>

//-------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->labelVersion->setText(APPNAME " GIT rev.: " GIT_REV " at " GIT_DATE " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP " UTC]");
	QObject *ev= new EventLogger(this);
	installEventFilter(ev);
	//setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	//move(0,0);
	//windowHandle()->setPosition(0,0);
	//ui->lineEdit->installEventFilter(ev);
	//ui->lineEdit_2->installEventFilter(ev);
	//ui->textEdit->installEventFilter(ev);
	//QTimer::singleShot(100,this,SLOT(on_SingleShot()));
	QTimer::singleShot(100,this,&MainWindow::on_SingleShot);
}
//-------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}
//-------------------------------------------------------------------------
void MainWindow::on_pushButtonFocus_clicked()
{
	ui->textEdit->setFocus(Qt::ActiveWindowFocusReason);
	qDebug()<< __PRETTY_FUNCTION__ << pos() << windowHandle()->position() << windowHandle()->parent();
}
//-------------------------------------------------------------------------
void MainWindow::on_pushButtonFunc_clicked()
{
	QMessageBox::StandardButton rc = QMessageBox::question(this, tr("QMessageBox::information()"),"Achtung!");
	qDebug() << __PRETTY_FUNCTION__ << rc;
	//move(0,0);
	//windowHandle()->setPosition(0,0);
}
//-------------------------------------------------------------------------
void MainWindow::on_SingleShot()
{
	//qDebug() << __PRETTY_FUNCTION__ ;
	qDebug()<< __PRETTY_FUNCTION__ << pos() << windowHandle()->position() << windowHandle()->parent();
	move(0,0);
	//windowHandle()->setPosition(0,0);
}
//-------------------------------------------------------------------------
void MainWindow::showEvent(QShowEvent *event)
{
	//qDebug()<< __PRETTY_FUNCTION__ << pos() << windowHandle()->position() << windowHandle()->parent();
	//QSurfaceFormat f(windowHandle()->format());
	//f.setAlphaBufferSize(8);
	//windowHandle()->setFormat(f);
	//windowHandle()->setFlags(windowHandle()->flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint	| Qt::WindowDoesNotAcceptFocus);
	//windowHandle()->setPosition(0,0);
	//move(0,0);
	//windowHandle()->setPosition(0,0);
	qDebug()<< __PRETTY_FUNCTION__ << pos() << windowHandle()->position() << windowHandle()->parent();
}
//-------------------------------------------------------------------------
