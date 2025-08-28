#ifndef KEYDIALOG_H
#define KEYDIALOG_H

#include <QDialog>

namespace Ui {
class KeyDialog;
}

class KeyDialog : public QDialog
{
	Q_OBJECT

public:
	explicit KeyDialog(QWidget *parent = nullptr);
	~KeyDialog();

private:
	Ui::KeyDialog *ui;
};

#endif // KEYDIALOG_H
