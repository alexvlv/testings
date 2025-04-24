#ifndef WIDGETKEY_H
#define WIDGETKEY_H

#include <QWidget>

namespace Ui {
class WidgetKey;
}

class WidgetKey : public QWidget
{
	Q_OBJECT

public:
	explicit WidgetKey(QWidget *parent = nullptr);
	~WidgetKey();
	void setFocusItem(QObject *focusItem);

	void showEvent(QShowEvent *event) override;

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
	void hideEvent(QHideEvent *event) override;
	void closeEvent(QCloseEvent *event) override;

private slots:
	void on_pushButton_clicked();

private:
	void processKeyEvent(QObject *obj, QEvent *event);

	Ui::WidgetKey *ui;
	QObject * m_focusItem = nullptr;

};

#endif // WIDGETKEY_H
