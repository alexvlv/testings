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
	connect(keyb, &KeysManager::onStartEdit, this,  &KbdWidget::onStartEdit);
	connect(keyb, &KeysManager::onEditDone, this,  &KbdWidget::onEditDone);

	kbtns = findChildren<QPushButton *>(QRegularExpression("btnK"));
	std::sort(kbtns.begin(),kbtns.end(),[](const QObject *a, const QObject *b) { return a->objectName() < b->objectName();});
	qDebug()<< __PRETTY_FUNCTION__ << kbtns;
	//for (int i=0; i<kbtns.size();i++) {
	//	 connect(kbtns[i], &QPushButton::clicked, [this, i]() {
	//		 qDebug() << "K-Button clicked:"  << i;
	//		 Q_EMIT onFkey(i+KeysManager::KEY_K1);
	//	 });
	//}

	keyb->setEditButtons(ui->btnOK, ui->btnCancel, ui->btnShift, ui->btnUp, ui->btnDown);

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
void KbdWidget::onStartEdit(QWidget *w) // [Slot]
{
	ui->stackedWidget->setCurrentWidget(ui->pageEdit);
}
//-------------------------------------------------------------------------
void KbdWidget::onEditDone(QWidget *w, bool) // [Slot]
{
	ui->stackedWidget->setCurrentWidget(ui->pageMain);
}
//-------------------------------------------------------------------------
