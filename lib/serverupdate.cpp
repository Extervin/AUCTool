// serverupdate.cpp
#include "serverupdate.h"
#include "ui_serverupdate.h"
#include <QKeyEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QStringList>
#include <map>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSettings>
#include <QDirIterator>
#include <QtConcurrent>
#include <windows.h>
#include <QFileDialog>
#include "serveraucoperations.h"

ServerUpdate::ServerUpdate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerUpdate)
{
    operation = new ServerAUCOperations(this); // Передаем указатель на родительский объект
    ui->setupUi(this);
    ui->tableWidget->setColumnWidth(0, 15);
    ui->tableWidget->setColumnWidth(1, 200);
    ui->tableWidget->setColumnWidth(2, 200);
    ui->tableWidget->setColumnWidth(3, 55);
    ui->tableWidget->setColumnWidth(4, 55);
    ui->tableWidget->setColumnWidth(5, 55);
    ui->tableWidget->setColumnWidth(5, 70);

    //connect(this, &QObject::destroyed, this, &ServerUpdate::saveAndClose);

    ui->tableWidget->setSortingEnabled(true);

    loadFromFile(ipStoragePath);

    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &ServerUpdate::on_tableWidget_cellChanged);

    // В конструкторе или методе инициализации ServerUpdate:


    connect(operation, &ServerAUCOperations::sendDebugMessage, this, &ServerUpdate::receiveDebugMessage);

}

ServerUpdate::~ServerUpdate()
{
    delete ui;
    delete operation;
}

void ServerUpdate::receiveDebugMessage(const QString &message) {
    ui->debugBrowser->append(message);
}

void ServerUpdate::adddebugmessage(const QString &message) {
    ui->debugBrowser->append(message);
}

void ServerUpdate::on_tableWidget_cellChanged(int row, int column)
{
    saveToFile(ipStoragePath);
}

void ServerUpdate::saveToFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Can't open IP adresses storage for saving!");
        return;
    }

    QTextStream out(&file);
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QString rowData;
        for (int col = 1; col < ui->tableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidget->item(row, col);
            if (item) {
                rowData += item->text() + "\t";
            }
        }
        out << rowData.trimmed() << "\n";
    }

    file.close();
}


void ServerUpdate::loadFromFile(const QString& filePath) {
    qDebug() << "loading";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Can't open IP addresses storage for loading!");
        return;
    }
    qDebug() << "opened";
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split("\t");

        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        for (int col = 0; col < parts.size(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(parts[col]);
            ui->tableWidget->setItem(row, col+1, item);
        }

        // Вставка чекбокса в нулевую колонку
        QTableWidgetItem *checkBoxItem = new QTableWidgetItem();
        checkBoxItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        checkBoxItem->setCheckState(Qt::Unchecked);
        ui->tableWidget->setItem(row, 0, checkBoxItem);
    }
    file.close();
}


void ServerUpdate::on_addButton_clicked() {
    int rowCount = ui->tableWidget->rowCount();
    QString newRowName = QString::number(rowCount + 1); // Генерация нового имени строки

    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    // Добавление чекбокса в первый столбец новой строки
    QTableWidgetItem *checkBoxItem = new QTableWidgetItem();
    checkBoxItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    checkBoxItem->setCheckState(Qt::Unchecked);
    ui->tableWidget->setItem(row, 0, checkBoxItem);

    // Добавление имени строки во второй столбец новой строки
    QTableWidgetItem *nameItem = new QTableWidgetItem("New Line");
    ui->tableWidget->setItem(row, 1, nameItem);

    // Заполнение пустыми элементами остальных столбцов новой строки
    for (int col = 2; col < ui->tableWidget->columnCount(); ++col) {
        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->tableWidget->setItem(row, col, item);
    }

    saveToFile(ipStoragePath);
}


void ServerUpdate::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Delete) {
        QList<QTableWidgetItem *> selectedItems = ui->tableWidget->selectedItems();
        for (auto *item : selectedItems) {
            int row = item->row();
            ui->tableWidget->removeRow(row);
        }
        saveToFile(ipStoragePath); // Сохранение данных после удаления
    } else {
        QWidget::keyPressEvent(event);
    }
}

// Запрос логина и пароля
QStringList ServerUpdate::askCredentials() {
    QDialog dialog(this);
    QFormLayout form(&dialog);
    QLineEdit username(&dialog);
    QLineEdit password(&dialog);
    form.addRow("Username:", &username);
    form.addRow("Password:", &password);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted,
                     &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected,
                     &dialog, &QDialog::reject);

    form.addRow(&buttonBox);

    if (dialog.exec() == QDialog::Accepted) {
        return {username.text(), password.text()};
    }

    return QStringList();
}


void ServerUpdate::on_serverUpdateButton_clicked() {
    startserverupdate();
}

void ServerUpdate::on_refreshButton_clicked() {
    refresh();
}



void ServerUpdate::startserverupdate() {
    QString username = credentials.value(0);
    QString password = credentials.value(1);

    if (username.isEmpty() || password.isEmpty()) {
        credentials = askCredentials();
        username = credentials.value(0);
        password = credentials.value(1);
    }
    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "Username or password missing";
        adddebugmessage("Username or password missing");
        return;
    }

    int reply = QMessageBox::question(this, "Confirmation", "Do you want to close all drug.exe applications on all servers before updating?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
            QString ipAddress = ui->tableWidget->item(row, 2)->text();
            operation->closeDrugProcesses(ipAddress, username, password);
            QCoreApplication::processEvents();
            // Ждем некоторое время перед следующим обновлением
            QThread::msleep(100); // Например, 100 миллисекунд
        }
    } else if (reply == QMessageBox::No) {

    } else if (reply == QMessageBox::Cancel){
        return;
    }

    QSettings settings(settingsPath, QSettings::IniFormat);

    QString sourceUpdatePath = settings.value("updateSourcePath").toString();

    QStringList ipAddresses;

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *checkBoxItem = ui->tableWidget->item(row, 0); // Получаем чекбокс в первой колонке
        if (checkBoxItem && checkBoxItem->checkState() == Qt::Checked) {
            QTableWidgetItem *ipItem = ui->tableWidget->item(row, 2); // Получаем IP-адрес из третьей колонки
            if (ipItem) {
                QString ipAddress = ipItem->text();
                ipAddresses << ipAddress;
            }
        }
    }

    qDebug() << ipAddresses << sourceUpdatePath << username << password;

    operation->updateInBackground(sourceUpdatePath, ipAddresses, username, password);

    QCoreApplication::processEvents();
    // Ждем некоторое время перед следующим обновлением
    QThread::msleep(100); // Например, 100 миллисекунд

}

void ServerUpdate::refresh() {

    QString username = credentials.value(0);
    QString password = credentials.value(1);

    if (username.isEmpty() || password.isEmpty()) {
        credentials = askCredentials();
        username = credentials.value(0);
        password = credentials.value(1);
    }
    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "Username or password missing";
        adddebugmessage("Username or password missing");
        return;
    }

    QSettings settings(settingsPath, QSettings::IniFormat);

    QString sourceUpdatePath = settings.value("updateSourcePath").toString();
    std::vector<int> reply;
    // Получаем список IP-адресов из таблицы
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *checkBoxItem = ui->tableWidget->item(row, 0); // Получаем чекбокс в первой колонке
        if (checkBoxItem && checkBoxItem->checkState() == Qt::Checked) {
            QTableWidgetItem *ipItem = ui->tableWidget->item(row, 2); // Получаем IP-адрес из третьей колонки
            if (ipItem) {
                QString ipAddress = ipItem->text();
                reply = operation->refreshInBackground(sourceUpdatePath, ipAddress, username, password);
            }
            if (true) {

            }
            // Пинг
            if (reply[0] == 1) {
                QTableWidgetItem *item = new QTableWidgetItem("✔"); // зеленая галочка
                item->setForeground(Qt::darkGreen);
                ui->tableWidget->setItem(row, 4, item); // Указываем номер строки и столбца
            } else if (reply[0] == 0) {
                QTableWidgetItem *item = new QTableWidgetItem("✖"); // красный крестик
                item->setForeground(Qt::red);
                ui->tableWidget->setItem(row, 4, item); // Указываем номер строки и столбца
            } else if (reply[0] == 2) {
                QTableWidgetItem *item = new QTableWidgetItem("err");
                item->setForeground(Qt::red);
                ui->tableWidget->setItem(row, 4, item); // Указываем номер строки и столбца
            }

            // drug.exe
            if (reply[3] == 0) {
                QTableWidgetItem *item = new QTableWidgetItem("✔"); // зеленая галочка
                item->setForeground(Qt::darkGreen);
                ui->tableWidget->setItem(row, 5, item); // Указываем номер строки и столбца
            } else if (reply[3] == 1) {
                QTableWidgetItem *item = new QTableWidgetItem("✖"); // красный крестик
                item->setForeground(Qt::red);
                ui->tableWidget->setItem(row, 5, item); // Указываем номер строки и столбца
            } else if (reply[3] == 2) {
                QTableWidgetItem *item = new QTableWidgetItem("err");
                item->setForeground(Qt::red);
                ui->tableWidget->setItem(row, 5, item); // Указываем номер строки и столбца
            }

            //folders
            if (reply[1] == 0) {
                QTableWidgetItem *item = new QTableWidgetItem("err");
                item->setForeground(Qt::red);
                ui->tableWidget->setItem(row, 3, item); // Указываем номер строки и столбца
            } else {
                int value = reply[1];
                QTableWidgetItem *item = new QTableWidgetItem(QString::number(value)); // красный крестик
                ui->tableWidget->setItem(row, 3, item); // Указываем номер строки и столбца
            }

            // version
            if (reply[2] == 1) {
                QTableWidgetItem *item = new QTableWidgetItem("match");
                item->setForeground(Qt::darkGreen);
                ui->tableWidget->setItem(row, 6, item); // Указываем номер строки и столбца
            } else if (reply[2] == 0) {
                QTableWidgetItem *item = new QTableWidgetItem("unmatch");
                item->setForeground(Qt::red);
                ui->tableWidget->setItem(row, 6, item); // Указываем номер строки и столбца
            } else if (reply[2] == 2) {
                QTableWidgetItem *item = new QTableWidgetItem("error");
                item->setForeground(Qt::red);
                ui->tableWidget->setItem(row, 6, item); // Указываем номер строки и столбца
            }
            QCoreApplication::processEvents();
            // Ждем некоторое время перед следующим обновлением
            QThread::msleep(100); // Например, 100 миллисекунд
        }
    }
}

void ServerUpdate::on_toolButton_clicked() {
    QString importPath = QFileDialog::getOpenFileName(this, tr("Choose file for import"), QDir::currentPath(), tr("Text files (*.txt)"));

    if (!importPath.isEmpty()) {
        loadFromFile(importPath);
    }
}

void ServerUpdate::on_loginButton_clicked()
{
    credentials = askCredentials();
}


void ServerUpdate::on_chooseAll_stateChanged(int arg1)
{
    Qt::CheckState checkState = static_cast<Qt::CheckState>(arg1);

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *checkBoxItem = ui->tableWidget->item(row, 0);
        if (checkBoxItem) {
            checkBoxItem->setCheckState(checkState);
        }
    }
}

