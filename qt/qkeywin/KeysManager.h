#pragma once

#include "macro.h"

#include <QObject>
#include <QPointer>
#include <QDebug>
#include <QtWidgets/qlabel.h>
#include <functional>

class KeysManager : public QObject
{
	Q_OBJECT
public:
	explicit KeysManager(QObject *parent = nullptr);
	void setLabel(QLabel *);
	void setShift(bool);
	void startEdit(QWidget * = nullptr);

	enum Key {
		KEY_0 = 0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,

		KEY_DEL,
		KEY_OK,

		KEY_K1,
		KEY_K2,
		KEY_K3,
		KEY_K4,
		KEY_K5,

		KEY_UNKNOWN,
		KEY_MAX
	};


signals:
	void onKey(uint);

	#define SIGNAL_NAME(X) onKey_##X()
	#define SIGNAL_DECL(X) void SIGNAL_NAME(X)

	SIGNAL_DECL(0); SIGNAL_DECL(1); SIGNAL_DECL(2); SIGNAL_DECL(3);SIGNAL_DECL(4);
	SIGNAL_DECL(5); SIGNAL_DECL(6); SIGNAL_DECL(7); SIGNAL_DECL(8);SIGNAL_DECL(9);
	SIGNAL_DECL(NO); SIGNAL_DECL(OK);
	SIGNAL_DECL(K1); SIGNAL_DECL(K2); SIGNAL_DECL(K3);SIGNAL_DECL(K4); SIGNAL_DECL(K5);

	void onEditDone(QWidget *, bool);
	void onEditOk(QWidget *);
	void onEditCancel(QWidget *);

public slots:
	void doToggleShift();

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
	void timerEvent(QTimerEvent *event) override;

private:
	void processKeyEvent(QObject *obj, QEvent *event);
	void processKeyPress(QObject *obj, uint key);
	void processKeyRelease(QObject *obj, uint key);
	void processAlphaKey(uint key);
	void stopEdit(bool ok);

	template<typename T>
	bool setReadOnly(QWidget *w, bool fl) { T* p= qobject_cast<T *>(w); if(p) p->setReadOnly(fl); return (p!=nullptr);};

	void setEditorReadOnly(QWidget *w, bool fl);

	constexpr static const Qt::Key KeyCodes[KEY_MAX] = {
		Qt::Key_Pause, // 0

		Qt::Key_F6, // 1
		Qt::Key_F7,
		Qt::Key_F8,

		Qt::Key_F9, // 4
		Qt::Key_F10,
		Qt::Key_F11,

		Qt::Key_F12, // 7
		Qt::Key_Print,
		Qt::Key_ScrollLock,

		Qt::Key_Backspace, // NO
		Qt::Key_Return, // OK

		Qt::Key_F1,   // in edit mode: Shift
		Qt::Key_F2,   // in edit mode: Cancel
		Qt::Key_F3,   // in edit mode: OK
		Qt::Key_F4,
		Qt::Key_F5,
		static_cast<Qt::Key>(0)
	};

	constexpr static const char * const KeyNames[KEY_MAX]  = {
		"#0","#1","#2","#3","#4","#5","#6","#7","#8","#9",
		"NO","OK",
		"K1","K2","K3","K4","K5",
		"UNKNOWN"
	};

	static  QMap<Qt::Key, uint> KeysCodeIdx;
	static  QMap<Qt::Key, const char *> KeysCodeNames;

	static const unsigned NO = 10;
	static const unsigned OK = 11;
	static const unsigned K1 = 12;
	static const unsigned K2 = 13;
	static const unsigned K3 = 14;
	static const unsigned K4 = 15;
	static const unsigned K5 = 16;

	#define EMIT_LAMBDA(X) [this]() { const uint idx=X; qDebug() << "KEY pressed:" << KeyNames[idx] << "[" << xstr(X) << "]" ; Q_EMIT SIGNAL_NAME(X);}
	using EmitFunc = std::function<void()>;
	EmitFunc emitters[KEY_MAX] = {
		EMIT_LAMBDA(0), EMIT_LAMBDA(1), EMIT_LAMBDA(2), EMIT_LAMBDA(3), EMIT_LAMBDA(4),
		EMIT_LAMBDA(5), EMIT_LAMBDA(6), EMIT_LAMBDA(7), EMIT_LAMBDA(8), EMIT_LAMBDA(9),
		EMIT_LAMBDA(NO), EMIT_LAMBDA(OK),
		EMIT_LAMBDA(K1), EMIT_LAMBDA(K2), EMIT_LAMBDA(K3), EMIT_LAMBDA(K4), EMIT_LAMBDA(K5),
		[this]() { ; }
	};

	static const QString sAlphKey; // = QString::fromUtf8("1АБВ");
	static const QStringList sSymbols;

	QPointer<QLabel> label;
	QWidget *editor = nullptr;
	bool flShift = false;
	uint currentAlphaKey = KEY_UNKNOWN;
	uint numAlphaSyms = 0;
	uint currentSym = 0;
	int  timerId = -1;
	bool flDigits = false;
};
