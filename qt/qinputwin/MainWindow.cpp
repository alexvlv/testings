#include ".git.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"

//-------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->labelVersion->setText(APPNAME " GIT rev.: " GIT_REV " at " GIT_DATE " on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP " UTC]");
}
//-------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}
//-------------------------------------------------------------------------
