#include "KeysManager.h"
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

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
		processKeyEvent(obj, event);
		case QEvent::KeyRelease:
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
	uint idx = KeysCodeIdx.value(static_cast<Qt::Key>(evk->key()),KEY_UNKNOWN);
	if( idx < KEY_UNKNOWN ) {
		emitters[idx]();
		Q_EMIT onKey(idx);
	} else {
		qWarning() << "KEY Unknown :" <<  event->type() << Qt::hex << evk->key() << evk->text();
	}
}
//-------------------------------------------------------------------------
/*
https://isocpp.org/wiki/faq/pointers-to-members
https://stackoverflow.com/questions/28746744/passing-capturing-lambda-as-function-pointer/28746827
*/
//-------------------------------------------------------------------------
