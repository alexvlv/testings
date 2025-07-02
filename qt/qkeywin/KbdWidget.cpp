#include "KbdWidget.h"
#include "ui_KbdWidget.h"
#include "KeysManager.h"

#include <QtCore/qregularexpression.h>
#include <QTimer>
#include <QAbstractSpinBox>
#include <cassert>

#ifndef Q_PROCESSOR_ARM
#define BUTTON_MIN_WIDTH 30
#endif



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

	kbtns = findChildren<QPushButton *>(QRegularExpression("btnK"));
	std::sort(kbtns.begin(),kbtns.end(),[](const QObject *a, const QObject *b) { return a->objectName() < b->objectName();});
	qDebug()<< __PRETTY_FUNCTION__ << kbtns;
	for (int i=0; i<kbtns.size();i++) {
		 connect(kbtns[i], &QPushButton::clicked, [this, i]() {
			 qDebug() << "K-Button clicked:"  << i;
			 Q_EMIT onFkey(i+KeysManager::KEY_K1);
		 });
	}
	connect(ui->btnShift, &QPushButton::toggled, keyb, &KeysManager::setShift);
	connect(ui->btnOK, &QPushButton::clicked, keyb, &KeysManager::stopEditOK);
	connect(ui->btnCancel, &QPushButton::clicked, keyb, &KeysManager::stopEditCancel);

#ifdef BUTTON_MIN_WIDTH
	foreach (QPushButton *b, findChildren<QPushButton *>()) b->setMinimumSize(BUTTON_MIN_WIDTH,0);
#endif
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
	QAbstractSpinBox* b= qobject_cast<QAbstractSpinBox *>(w);
	if(b) {
		connect(ui->btnUp, &QPushButton::clicked, b, &QAbstractSpinBox::stepUp);
		connect(ui->btnDown, &QPushButton::clicked, b, &QAbstractSpinBox::stepDown);
	}
}
//-------------------------------------------------------------------------
void KbdWidget::onEditDone(QWidget *w, bool) // [Slot]
{
	ui->stackedWidget->setCurrentWidget(ui->pageMain);
	QAbstractSpinBox* b= qobject_cast<QAbstractSpinBox *>(w);
	if(b) {
		disconnect(ui->btnUp, &QPushButton::clicked, b, &QAbstractSpinBox::stepUp);
		disconnect(ui->btnDown, &QPushButton::clicked, b, &QAbstractSpinBox::stepDown);
	}

}
//-------------------------------------------------------------------------
