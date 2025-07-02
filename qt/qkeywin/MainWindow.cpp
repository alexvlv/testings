#include "MainWindow.h"
#include "ui_MainWindow.h"
#include ".git.h"

#include "KbdWidget.h"
#include "KeysManager.h"

#include <QLayout>

#ifdef Q_PROCESSOR_ARM
#define FONT_SIZE 48
#endif


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
	layout()->addWidget(kbdw);

	kbtns = findChildren<QPushButton *>(QRegularExpression("btn_N"));
	connect(kbdw, &KbdWidget::onFkey, this, &MainWindow::onKeyButtonClicked);
	struct {
		bool operator()(const QPushButton *a, const QPushButton *b) const { return a->objectName() < b->objectName(); }
	} objectNameSort;
	std::sort(kbtns.begin(),kbtns.end(),objectNameSort);
	//std::sort(kbtns.begin(),kbtns.end(),[](const QObject *a, const QObject *b) { return a->objectName() < b->objectName();});
	//qDebug()<< __PRETTY_FUNCTION__ << kbtns;
	for (int i=0; i<kbtns.size();i++) {
		const QPushButton *b = kbtns[i];
		connect(b, &QPushButton::clicked, [this, i, b ]() { qDebug() << "#-Button clicked:" << i << b; });
	}
	connect(ui->pushButton,  &QPushButton::clicked, [this](bool ok){ qDebug() << "Button clicked:" << ok; } );
	connect(ui->checkBox,  &QPushButton::toggled, [this](bool ok){ qDebug() << "checkBox clicked:" << ok << sender(); } );

#ifndef Q_PROCESSOR_ARM
	ui->spinBox->setStyleSheet("QSpinBox::down-button{ width: 24 } QSpinBox::up-button{ width: 24 } QSpinBox { font-size: 16px }");
	ui->checkBox->setStyleSheet("QCheckBox::indicator { width: 24px; height: 24px;} QCheckBox { font-size: 16px }");
#endif
}
//-------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}
//-------------------------------------------------------------------------
void MainWindow::onKeyButtonClicked(int i) // [slot]
{
	qDebug()<< __PRETTY_FUNCTION__ << i << sender();
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
	}
}
//-------------------------------------------------------------------------
void MainWindow::onKeyboard(int i) // [slot]
{
	//qDebug()<< __PRETTY_FUNCTION__ << i << sender();
	switch (i) {
	case KeysManager::KEY_K1:
	case KeysManager::KEY_K2:
	case KeysManager::KEY_K3:
	case KeysManager::KEY_K4:
	case KeysManager::KEY_K5:
		//onKeyButtonClicked(i);
		break;
	default:
		if(i<=KeysManager::KEY_9) {
			kbtns[i]->animateClick();
		}
		break;
	}
}
//-------------------------------------------------------------------------
/*
QSpinBox::down-button{
	width: 50
}
QSpinBox::up-button{
	width: 50
}
QSpinBox { font-size: 60px }

QCheckBox::indicator {
	 width: 50px;
	 height: 50px;
}
QCheckBox { font-size: 60px }

*/
