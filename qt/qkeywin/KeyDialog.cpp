#include "KeyDialog.h"
#include "ui_KeyDialog.h"

KeyDialog::KeyDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::KeyDialog)
{
	ui->setupUi(this);
	setParent(0); // Create TopLevel-Widget
	//setWindowState(Qt::WindowFullScreen);
	setWindowFlags( windowFlags() | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_TranslucentBackground, true);
}

KeyDialog::~KeyDialog()
{
	delete ui;
}

/*
	https://stackoverflow.com/questions/18316710/frameless-and-transparent-window
 */
