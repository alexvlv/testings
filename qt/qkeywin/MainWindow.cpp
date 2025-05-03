#include "MainWindow.h"
#include "ui_MainWindow.h"
#include ".git.h"

#include "KbdWidget.h"
#include "KeysManager.h"

//-------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->lblVersion->setText(APPNAME " GIT rev.: <b>" GIT_REV "</b> at <u>" GIT_DATE "</u> on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP " UTC]");

	kbdw = new KbdWidget(this);
	connect(kbdw, &KbdWidget::activated, this, &MainWindow::onKeyboard);
	//kbdw->setClients({ui->lineEdit,ui->lineEditDigits,ui->textEdit,ui->spinBox});
	//kbdw->setEnabled({5,6,7,8,9,10,11,12,14,15});
	layout()->addWidget(kbdw);
	KeysManager *keyb = new KeysManager(this);
	connect(keyb, &KeysManager::onKey, this, &MainWindow::onKeyboard);
	keyb->setLabel(ui->labelAlpha);
	installEventFilter(keyb);
	//ui->pushButton->setFocus(Qt::ActiveWindowFocusReason);
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
	switch (i) {
	case KeysManager::KEY_K1:
		ui->lineEdit ->setFocus(Qt::ActiveWindowFocusReason);
		break;
	case KeysManager::KEY_K2:
		ui->textEdit ->setFocus(Qt::ActiveWindowFocusReason);
		break;
	case KeysManager::KEY_K3:
		ui->lineEditDigits ->setFocus(Qt::ActiveWindowFocusReason);
		break;
	case KeysManager::KEY_K4:
		ui->spinBox ->setFocus(Qt::ActiveWindowFocusReason);
		break;
	case KeysManager::KEY_K5:
		ui->checkBox ->setFocus(Qt::ActiveWindowFocusReason);
		break;
	default:
		break;
	}
}
//-------------------------------------------------------------------------
