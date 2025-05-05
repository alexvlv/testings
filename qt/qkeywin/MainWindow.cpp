#include "MainWindow.h"
#include "ui_MainWindow.h"
#include ".git.h"

#include "KbdWidget.h"
#include "KeysManager.h"

#include <QLayout>

//-------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->lblVersion->setText(APPNAME " GIT rev.: <b>" GIT_REV "</b> at <u>" GIT_DATE "</u> on " GIT_BRANCH " [Build: " BUILD_TIMESTAMP " UTC]");

	keyb = new KeysManager(this);
	connect(keyb, &KeysManager::onKey, this, &MainWindow::onKeyboard);
	installEventFilter(keyb);
	kbdw = new KbdWidget(keyb, this);
	//kbdw->move(0,1100);
	layout()->addWidget(kbdw);
	kbtns = findChildren<QPushButton *>(QRegularExpression("btn_N"));

	struct {
		bool operator()(const QPushButton *a, const QPushButton *b) const { return a->objectName() < b->objectName(); }
	} objectNameSort;
	std::sort(kbtns.begin(),kbtns.end(),objectNameSort);
	//std::sort(kbtns.begin(),kbtns.end(),[](const QObject *a, const QObject *b) { return a->objectName() < b->objectName();});
	//qDebug()<< __PRETTY_FUNCTION__ << kbtns;
}
//-------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}
//-------------------------------------------------------------------------
void MainWindow::onKeyboard(int i) // [slot]
{
	//qDebug()<< __PRETTY_FUNCTION__ << i << sender();
	switch (i) {
	case KeysManager::KEY_K1:
		//ui->lineEdit->setFocus(Qt::ActiveWindowFocusReason);
		//ui->lineEdit->setReadOnly(false);
		keyb->startEdit(ui->lineEdit);
		break;
	case KeysManager::KEY_K2:
		keyb->startEdit(ui->textEdit);
		break;
	case KeysManager::KEY_K3:
		keyb->startEdit(ui->lineEditDigits);
		break;
	case KeysManager::KEY_K4:
		keyb->startEdit(ui->spinBox);
		break;
	case KeysManager::KEY_K5:
		ui->checkBox->setChecked(!ui->checkBox->isChecked());
		break;
	default:
		if(i<=KeysManager::KEY_9) {
			kbtns[i]->animateClick();
		}
		break;
	}
}
//-------------------------------------------------------------------------
