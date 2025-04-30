#include "MainWindow.h"
#include "ui_MainWindow.h"
#include ".git.h"

#include "KbdWidget.h"

//-------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->lblVersion->setText(APPNAME " GIT rev.: " GIT_REV " at " GIT_DATE " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP " UTC]");

	kbdw = new KbdWidget(this);
	connect(kbdw, &KbdWidget::activated, this, &MainWindow::onKeyboard);
	//kbdw->setClients({ui->lineEdit,ui->lineEditDigits,ui->textEdit,ui->spinBox});
	//kbdw->setEnabled({5,6,7,8,9,10,11,12,14,15});
	layout()->addWidget(kbdw);

}
//-------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}
//-------------------------------------------------------------------------
void MainWindow::onKeyboard(int i) // [slot]
{
	qDebug()<< __PRETTY_FUNCTION__ << i << sender();
}
//-------------------------------------------------------------------------
