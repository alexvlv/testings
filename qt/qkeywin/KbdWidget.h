#ifndef KBDWIDGET_H
#define KBDWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QAction>


namespace Ui {
class KbdWidget;
}

class KeysManager;

class KbdWidget : public QWidget
{
	Q_OBJECT

public:
	explicit KbdWidget(KeysManager *keyb, QWidget *parent = nullptr);
	~KbdWidget();

public slots:
	void onStartEdit(QWidget * = nullptr);
	void onEditDone(QWidget *, bool);
	void onKeyboard(int);

signals:
	void onFkey(int);

private:
	Ui::KbdWidget *ui;
	KeysManager *keyb = nullptr;
	QList<QPushButton *> kbtns;

};

#endif // KBDWIDGET_H
