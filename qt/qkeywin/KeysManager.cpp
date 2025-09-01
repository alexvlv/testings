#include "KeysManager.h"
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QWidget>
#include <QtWidgets/qapplication.h>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qtextedit.h>
#include <QtWidgets/qplaintextedit.h>
#include <QtWidgets/qabstractspinbox.h>

#include <QAbstractButton>
#include <functional>

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
QMap<QPointer<QWidget>, KeysManager *> KeysManager::watched;
//-------------------------------------------------------------------------
KeysManager& KeysManager::get(QWidget *w)
{
	assert(w != nullptr);

	watched.remove(nullptr);
	QPointer<QWidget> pw(w);
	if(watched.contains(pw)) {
		KeysManager &km = *(watched.value(pw));
		return km;
	 } else {
		KeysManager* k = new KeysManager(w);
		watched.insert(pw, k);
		return *k;
	}
}
//-------------------------------------------------------------------------
KeysManager::KeysManager(QObject *parent)
	: QObject{parent}
{
	if(KeysCodeIdx.empty()) for(int i = 0; i<KEY_MAX; i++ ) {
		KeysCodeIdx.insert(KeyCodes[i],i);
		KeysCodeNames.insert(KeyCodes[i],KeyNames[i]);
	}
	parent->installEventFilter(this);
}
//-------------------------------------------------------------------------
void KeysManager::setSlaveButton(uint idx, QAbstractButton *btn)
{
	assert(idx < KEY_PTT);
	slaveButtons[idx] = btn;
}
//-------------------------------------------------------------------------
void KeysManager::setEditButtons(QAbstractButton *ok, QAbstractButton *cancel,
	QAbstractButton* shift, QAbstractButton* up, QAbstractButton* down)
{
	resetEditButtons();
	//qDebug() << __PRETTY_FUNCTION__ << "ok:" << ok << (ok?ok->text():"");
	//qDebug() << __PRETTY_FUNCTION__ << "cancel:" << cancel << (cancel?cancel->text():"");
	if(ok) { editButtons.ok = ok; connect(editButtons.ok, &QAbstractButton::clicked, this, &KeysManager::stopEditOK);}
	if(cancel) { editButtons.cancel = cancel; connect(editButtons.cancel, &QAbstractButton::clicked, this, &KeysManager::stopEditCancel);}
	if(shift) {
		editButtons.shift = shift; connect(editButtons.shift, &QAbstractButton::toggled, this, &KeysManager::setShift);
		connect(this, &KeysManager::onShift, editButtons.shift, &QAbstractButton::setChecked);
	}
	if(up) { editButtons.up = up; }
	if(down) { editButtons.down = down; }

	for (uint i = 0; i < NUM_EDIT_BUTTONS; i++) {
		if(editButtons.array[i]) editButtons.array[i]->setVisible(false);
	};
}
//-------------------------------------------------------------------------
void KeysManager::resetEditButtons()
{
	for (uint i = 0; i < NUM_EDIT_BUTTONS; i++) {
		if(editButtons.array[i]) disconnect(editButtons.array[i], nullptr, this, nullptr);
	};
	memset(&editButtons, 0, sizeof(editButtons));
}
//-------------------------------------------------------------------------
void KeysManager::setShift(bool fl) // [Slot]
{
	flShift = fl;
	Q_EMIT onShift(flShift);
}
//-------------------------------------------------------------------------
void KeysManager::doToggleShift() // [Slot]
{
	flShift = !flShift;
	Q_EMIT onShift(flShift);
}
//-------------------------------------------------------------------------
void KeysManager::setLabel(QLabel *l)
{
	label = l;
}
//-------------------------------------------------------------------------
bool KeysManager::isSpinBox() const
{
	return qobject_cast<QAbstractSpinBox *>(editor)!=nullptr;
}
//-------------------------------------------------------------------------
void KeysManager::startEdit(QWidget * w) // [Slot]
{
	if(editor) return;
	if(w) {
		w->setFocus(Qt::ActiveWindowFocusReason);
	} else {
		w = QApplication::focusWidget();
	}
	if(!w) return;
	if(editButtons.ok) editButtons.ok->setVisible(true);
	if(editButtons.cancel) editButtons.cancel->setVisible(true);
	setEditorReadOnly(w, false);
	setShift(false);
	flDigits = w->property("digits").toBool();
	editor = w;
	if(!label.isNull())  label->setText("");
	if(editButtons.shift) editButtons.shift->setVisible(!flDigits);
	//if(editButtons.shift) editButtons.shift->show();
	QAbstractSpinBox *spb = qobject_cast<QAbstractSpinBox *>(editor);
	if(editButtons.up) {
		editButtons.up->setVisible(spb != nullptr);
		if(spb) connect(editButtons.up, &QAbstractButton::clicked, spb, &QAbstractSpinBox::stepUp);
	}
	if(editButtons.down) {
		editButtons.down->setVisible(spb != nullptr);
		if(spb) connect(editButtons.down, &QAbstractButton::clicked, spb, &QAbstractSpinBox::stepDown);
	}
	qDebug() << "KeysManager::startEdit [" << parent()->objectName() << "] editor:" << editor << "digits only:" << flDigits;
	Q_EMIT onStartEdit(editor);
}
//-------------------------------------------------------------------------
void KeysManager::setEditorReadOnly(QWidget *w, bool fl)
{
	if(setReadOnly<QLineEdit>(w,fl)) return;
	if(setReadOnly<QTextEdit>(w,fl)) return;
	if(setReadOnly<QPlainTextEdit>(w,fl)) return;
	if(setReadOnly<QAbstractSpinBox>(w,fl)) return;
}
//-------------------------------------------------------------------------
void KeysManager::stopEdit(bool ok) // [Slot]
{
	setShift(false);
	if(editButtons.ok) editButtons.ok->setVisible(false);
	if(editButtons.cancel) editButtons.cancel->setVisible(false);
	if(editButtons.shift) editButtons.shift->setVisible(false);
	if(!editor) return;
	qDebug() << "KeysManager::stopEdit [" << parent()->objectName() << "] editor:" << editor << "result:" << flDigits;
	QAbstractSpinBox* spb = qobject_cast<QAbstractSpinBox *>(editor);
	if(spb) {
		if(editButtons.up) {
			editButtons.up->setVisible(false);
			disconnect(editButtons.up, &QAbstractButton::clicked, spb, &QAbstractSpinBox::stepUp);
		}
		if(editButtons.down) {
			editButtons.down->setVisible(false);
			disconnect(editButtons.down, &QAbstractButton::clicked, spb, &QAbstractSpinBox::stepDown);
		}
	}
	if(ok) { Q_EMIT onEditOk(editor);} else { Q_EMIT onEditCancel(editor);}
	Q_EMIT onEditDone(editor, ok);
	setEditorReadOnly(editor, true);
	editor = nullptr;
}
//-------------------------------------------------------------------------
void KeysManager::stopEditOK() // [Slot]
{
	//qDebug() << __PRETTY_FUNCTION__ << sender();
	stopEdit(true);
}
//-------------------------------------------------------------------------
void KeysManager::stopEditCancel() // [Slot]
{
	//qDebug() << __PRETTY_FUNCTION__ << sender();
	stopEdit(false);
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
	//qDebug() << "KEY Event on" << obj->objectName() << ":" << event->type() << Qt::hex << evk->key() << KeysCodeNames.value(static_cast<Qt::Key>(evk->key()),"---") << evk->text() << evk->isAutoRepeat();
	uint key = KeysCodeIdx.value(static_cast<Qt::Key>(evk->key()),KEY_UNKNOWN);
	if( key < KEY_UNKNOWN ) {
		using ProcessFunc = std::function<void(QObject *, uint, bool)>;
		ProcessFunc process = std::bind(event->type()==QEvent::KeyPress?&KeysManager::processKeyPress:&KeysManager::processKeyRelease, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		process(obj, key, evk->isAutoRepeat());
	} else {
		qWarning() << "KEY Unknown :" <<  event->type() << Qt::hex << evk->key() << evk->text();
	}
}
//-------------------------------------------------------------------------
void KeysManager::processKeyPress(QObject *obj, uint key, bool autorepeat)
{
	killTimer(timerId);
	timerId = startTimer(1000);
	keysPressed[key] = true;

	//qDebug() << __PRETTY_FUNCTION__ << obj << key << autorepeat;

	QAbstractSpinBox* b= qobject_cast<QAbstractSpinBox *>(editor);
	if(editor && key <= KEY_K5) {
		if(key <= KEY_OK) {
			processAlphaKey(key);
		} else {
			if(!autorepeat) switch(key) {
				case KEY_K1:
					stopEdit(true);
					break;
				case KEY_K2:
					stopEdit(false);
					break;
				case KEY_K3:
					//if(b) b->clear();
					if(!flDigits) doToggleShift();
					if(b) b->stepDown();
					break;
				case KEY_K4:
					if(b) b->stepUp();
					break;
			}
		}
	} else {
		if(!autorepeat) {
			QAbstractButton *btn = slaveButtons.value(key, nullptr);
			if(btn && btn->isEnabled() && btn->isVisible()) {
				btn->animateClick();
			} else {
				emitters[key]();
				Q_EMIT onKey(key);
			}
		}
	}
}
//-------------------------------------------------------------------------
void KeysManager::processKeyRelease(QObject *obj, uint key, bool autorepeat)
{
	//qDebug() << __PRETTY_FUNCTION__ << obj << key << autorepeat;
	if(autorepeat) return;
	keysPressed[key] = false;
	if(editor && key < KEY_K1 ) return;
	if(emitters_release[key]) emitters_release[key]();
	Q_EMIT onRelease(key);

}
//-------------------------------------------------------------------------
void KeysManager::processAlphaKey(uint key)
{
	QString syms = sSymbols.at(key);

	if(flDigits) {
		QInputMethodEvent ev;
		QString sym = syms.at(0);
		ev.setCommitString(sym);
		QCoreApplication::sendEvent(editor,&ev);
		if(!label.isNull()) label->setText("");
		currentAlphaKey = KEY_UNKNOWN;
		return;
	}

	if(key != currentAlphaKey) {
		numAlphaSyms = syms.size();
		//qDebug() << __PRETTY_FUNCTION__ << KeyNames[key] << numAlphaSyms;
		currentSym = 0;
		currentAlphaKey = key;
	} else {
		if(++currentSym >= numAlphaSyms) currentSym=0;
	}
	if(!label.isNull()) {
		QString s = "[" + syms.left(currentSym)+ "<b>" + syms[currentSym] + "</b>" + syms.right(numAlphaSyms-currentSym-1) + "]";
		if(!flShift) s= s.toLower();
		label->setText(s);
		//label->setVisible(true);
		//qDebug() << __PRETTY_FUNCTION__  << s;
	}
}
//-------------------------------------------------------------------------
void KeysManager::timerEvent(QTimerEvent *event)
{
	killTimer(timerId);
	timerId = 0;

	for(uint key=0; key<KEY_MAX; key++) {
		if(keysPressed[key]) processKeyRelease(nullptr, key, false);
	}

	if(editor && currentAlphaKey <= KEY_9) {
		QInputMethodEvent ev;
		assert(currentSym < sSymbols.at(currentAlphaKey).size());
		QString sym = sSymbols.at(currentAlphaKey).at(currentSym);
		if(!flShift) sym= sym.toLower();
		ev.setCommitString(sym);
		QCoreApplication::sendEvent(editor,&ev);
	}
	currentAlphaKey = KEY_UNKNOWN;
	currentSym = 0;
	if(!label.isNull())  label->setText("");
}
//-------------------------------------------------------------------------
/*
https://isocpp.org/wiki/faq/pointers-to-members
https://stackoverflow.com/questions/28746744/passing-capturing-lambda-as-function-pointer/28746827
https://stackoverflow.com/questions/7582546/using-generic-stdfunction-objects-with-member-functions-in-one-class
*/
//-------------------------------------------------------------------------
