#include "KeysManager.h"
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QWidget>
#include <QtWidgets/qapplication.h>



//-------------------------------------------------------------------------
const QStringList  KeysManager::sSymbols = {
	QString::fromUtf8("0 "),
	QString::fromUtf8("1,!?"),
	QString::fromUtf8("2АБВГ"),
	QString::fromUtf8("3ДЕЖЗ"),
	QString::fromUtf8("4ИЙКЛ"),
	QString::fromUtf8("5МНОП"),
	QString::fromUtf8("6РСТУ"),
	QString::fromUtf8("7ФЦХЧ"),
	QString::fromUtf8("8ШЩЪЫ"),
	QString::fromUtf8("9ЬЭЮЯ"),
	QString::fromUtf8("-"),
	QString::fromUtf8("-"),
};
//-------------------------------------------------------------------------
QMap<Qt::Key, uint> KeysManager::KeysCodeIdx;
QMap<Qt::Key, const char *> KeysManager::KeysCodeNames;
//-------------------------------------------------------------------------
KeysManager::KeysManager(QObject *parent)
	: QObject{parent}
{
	if(KeysCodeIdx.empty()) for(int i = 0; i<KEY_MAX; i++ ) {
		KeysCodeIdx.insert(KeyCodes[i],i);
		KeysCodeNames.insert(KeyCodes[i],KeyNames[i]);
	}
}
//-------------------------------------------------------------------------
void KeysManager::setShift(bool fl)
{
	flShift = fl;
}
//-------------------------------------------------------------------------
void KeysManager::doToggleShift() // [Slot]
{
	flShift = !flShift;
}
//-------------------------------------------------------------------------
void KeysManager::setLabel(QLabel *l)
{
	label = l;
}
//-------------------------------------------------------------------------
void setLabel(const QLabel *);
bool KeysManager::eventFilter(QObject *obj, QEvent *event)
{
	switch (event->type()) {
		case QEvent::MouseMove:
		case QEvent::Paint:
		case QEvent::HoverMove:
		case QEvent::UpdateRequest:
		case QEvent::ShortcutOverride:
			break;
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
			processKeyEvent(obj, event);
			break;
		default:
			//qDebug() << "[" << objectName() << "]: Event on" << obj->objectName() << ":" << event->type();
			break;
	}
	return QObject::eventFilter(obj, event);
}
//-------------------------------------------------------------------------
void KeysManager::processKeyEvent(QObject *obj, QEvent *event)
{
	QKeyEvent *evk = dynamic_cast<QKeyEvent *>(event);
	Q_ASSERT(evk);
	//qDebug() << "[" << objectName() << "]: KEY Event on" << obj->objectName() << ":" << event->type() << Qt::hex << evk->key() << Qt::dec << evk->nativeScanCode() << evk->nativeVirtualKey() << evk->text();
	//qDebug() << "KEY Event on" << obj->objectName() << ":" << event->type() << Qt::hex << evk->key() << KeysCodeNames.value(static_cast<Qt::Key>(evk->key()),"---") << evk->text();
	uint key = KeysCodeIdx.value(static_cast<Qt::Key>(evk->key()),KEY_UNKNOWN);
	if( key < KEY_UNKNOWN ) {
		event->type()==QEvent::KeyPress?processKeyPress(obj, key):processKeyRelease(obj, key);
	} else {
		qWarning() << "KEY Unknown :" <<  event->type() << Qt::hex << evk->key() << evk->text();
	}
}
//-------------------------------------------------------------------------
void KeysManager::processKeyPress(QObject *obj, uint key)
{
	QWidget *focus =  QApplication::focusWidget();
	if(key <= KEY_OK && focus ) {
		processAlphaKey(key);
	} else {
		emitters[key]();
		Q_EMIT onKey(key);
	}
}
//-------------------------------------------------------------------------
void KeysManager::processKeyRelease(QObject *obj, uint key)
{
	if( key > KEY_9 || currentAlphaKey > KEY_9 ) return;
}
//-------------------------------------------------------------------------
void KeysManager::processAlphaKey(uint key)
{
	killTimer(timerId);
	timerId = startTimer(1000);

	QString syms = sSymbols.at(key);

	if(key != currentAlphaKey) {
		numAlphaSyms = syms.size();
		qDebug() << __PRETTY_FUNCTION__ << KeyNames[key] << numAlphaSyms;
		currentSym = 0;
		currentAlphaKey = key;
	} else {
		if(++currentSym >= numAlphaSyms) currentSym=0;
	}
	if(!label.isNull()) {
		QString s = syms.left(currentSym)+ "<b>" + syms[currentSym] + "</b>" + syms.right(numAlphaSyms-currentSym-1);
		label->setText(s);
		label->setVisible(true);
		qDebug() << __PRETTY_FUNCTION__  << s;
	}
}
//-------------------------------------------------------------------------
void KeysManager::timerEvent(QTimerEvent *event)
{
	QWidget *focus =  QApplication::focusWidget();
	if(focus && currentAlphaKey <= KEY_9) {
		QInputMethodEvent ev;
		assert(currentSym < sSymbols.at(currentAlphaKey).size());
		ev.setCommitString(sSymbols.at(currentAlphaKey).at(currentSym));
		QCoreApplication::sendEvent(focus,&ev);
	}
	currentAlphaKey = KEY_UNKNOWN;
	currentSym = 0;
	if(!label.isNull()) label->setVisible(false);
}
//-------------------------------------------------------------------------
/*
https://isocpp.org/wiki/faq/pointers-to-members
https://stackoverflow.com/questions/28746744/passing-capturing-lambda-as-function-pointer/28746827
*/
//-------------------------------------------------------------------------
