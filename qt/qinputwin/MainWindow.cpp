#include ".git.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "EventLogger.h"

//-------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->labelVersion->setText(APPNAME " GIT rev.: " GIT_REV " at " GIT_DATE " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP " UTC]");
	QObject *ev= new EventLogger(this);
	ui->lineEdit->installEventFilter(ev);
	ui->lineEdit_2->installEventFilter(ev);
	ui->textEdit->installEventFilter(ev);
}
//-------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}
//-------------------------------------------------------------------------
