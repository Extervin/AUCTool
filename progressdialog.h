#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);

    ~ProgressDialog();

public slots:

    void updateDebugInfo(const QString &info);

    void receiveListSize(const int& listSize);

    void receiveUpdateProgressBar();

private slots:
    void on_progressBar_valueChanged(int value);

    void on_exportButton_clicked();

    void on_closeButton_clicked();

private:
    Ui::ProgressDialog *ui;
};

#endif // PROGRESSDIALOG_H
