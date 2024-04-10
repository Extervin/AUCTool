#include "progressdialog.h"
#include "ui_progressdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QTextStream>

ProgressDialog::ProgressDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

}

void ProgressDialog::updateDebugInfo(const QString &info) {
    ui->debugBrowser->append(info);
}

void ProgressDialog::receiveUpdateProgressBar(){
    int currentValue = ui->progressBar->value();
    // Увеличиваем на 1
    int newValue = currentValue + 1;
    // Устанавливаем новое значение
    ui->progressBar->setValue(newValue);
}

void ProgressDialog::receiveListSize(const int &listSize) {
    ui->progressBar->setRange(0, listSize);
    ui->progressBar->setValue(0);
}



ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::on_progressBar_valueChanged(int value)
{
    if (value == ui->progressBar->maximum()) {
        ui->closeButton->setEnabled(true);
        ui->exportButton->setEnabled(true);
        setWindowTitle("Operation is ready!");
    }
}

void ProgressDialog::on_exportButton_clicked()
{
    // Получаем текст из текстового браузера
    QString text = ui->debugBrowser->toPlainText();

    // Запрашиваем у пользователя путь для сохранения файла
    QString defaultFileName = "update_logs_" + QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".txt";
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), defaultFileName, tr("Text Files (*.txt)"));

    // Если пользователь выбрал место сохранения и нажал ОК
    if (!filePath.isEmpty()) {
        // Создаем файл для записи
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            // Записываем текст в файл
            QTextStream out(&file);
            out << text;
            file.close();

            // Выводим сообщение об успешном сохранении
            QMessageBox::information(this, tr("Success"), tr("File saved successfully."));
        } else {
            // Выводим сообщение об ошибке при сохранении
            QMessageBox::critical(this, tr("Error"), tr("Failed to save file."));
        }
    }
}

void ProgressDialog::on_closeButton_clicked()
{
    close();
}

