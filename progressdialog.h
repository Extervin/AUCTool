#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class Progress;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();

private:
    Ui::Progress *ui;
};

#endif // PROGRESSDIALOG_H
