#include "KbdWidget.h"
#include "ui_KbdWidget.h"
#include "KeysManager.h"

#include <QtCore/qregularexpression.h>
#include <QTimer>
#include <cassert>

//-------------------------------------------------------------------------
KbdWidget::KbdWidget(KeysManager *keyb, QWidget *parent) :
	QWidget(parent),keyb(keyb),
	ui(new Ui::KbdWidget)
{
	ui->setupUi(this);
	ui->stackedWidget->setCurrentWidget(ui->pageMain);

	keyb->setLabel(ui->labelSymbols);

	connect(keyb, &KeysManager::onKey, this, &KbdWidget::onKeyboard);
	connect(keyb, &KeysManager::onStartEdit, this,  &KbdWidget::onStartEdit);
	connect(keyb, &KeysManager::onEditDone, this,  &KbdWidget::onEditDone);
	connect(keyb, &KeysManager::onShift, ui->btnShift,&QPushButton::setChecked);
	//connect(ui->btnCancel, &QPushButton::clicked, keyb

	kbtns = findChildren<QPushButton *>(QRegularExpression("btnK"));
	std::sort(kbtns.begin(),kbtns.end(),[](const QObject *a, const QObject *b) { return a->objectName() < b->objectName();});
	//qDebug()<< __PRETTY_FUNCTION__ << kbtns;

}
//-------------------------------------------------------------------------
KbdWidget::~KbdWidget()
{
	delete ui;
}
//-------------------------------------------------------------------------
void KbdWidget::onKeyboard(int key) // [slot]
{
	//qDebug()<< __PRETTY_FUNCTION__ << key << sender();
	switch (key) {
		case KeysManager::KEY_K1:
		case KeysManager::KEY_K2:
		case KeysManager::KEY_K3:
		case KeysManager::KEY_K4:
		case KeysManager::KEY_K5:
			//qDebug()<< __PRETTY_FUNCTION__  << kbtns[key-KeysManager::KEY_K1];
			//QTimer::singleShot(0,kbtns[key-KeysManager::KEY_K1],&QPushButton::animateClick);
			kbtns[key-KeysManager::KEY_K1]->animateClick();
			break;
		default:
			break;
	}
}
//-------------------------------------------------------------------------
void KbdWidget::onStartEdit(QWidget *w) // [Slot]
{
	ui->stackedWidget->setCurrentWidget(ui->pageEdit);
	ui->btnShift->setVisible(-!keyb->isDigits());
	bool fl = keyb->isSpinBox();
	ui->btnUp->setVisible(fl);
	ui->btnDown->setVisible(fl);
}
//-------------------------------------------------------------------------
void KbdWidget::onEditDone(QWidget *, bool) // [Slot]
{
	ui->stackedWidget->setCurrentWidget(ui->pageMain);
}
//-------------------------------------------------------------------------
