#pragma once

#include <QObject>

class KeysManager : public QObject
{
	Q_OBJECT
public:
	explicit KeysManager(QObject *parent = nullptr);

	enum Keys {
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

		KEY_NO,
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

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private:
	void processKeyEvent(QObject *obj, QEvent *event);

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

		Qt::Key_F1,
		Qt::Key_F2,
		Qt::Key_F3,
		Qt::Key_F4,
		Qt::Key_F5,
		static_cast<Qt::Key>(0)
	};

	constexpr static const char * const KeyNames[KEY_MAX+1]  = {
		"#0","#1","#2","#3","#4","#5","#6","#7","#8","#9",
		"NO","OK",
		"K1","K2","K3","K4","K5",
		"UNKNOWN"
	};

	static  QMap<Qt::Key, int> KeysCodeIdx;
	static  QMap<Qt::Key, const char *> KeysCodeNames;
};
