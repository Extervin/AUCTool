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


ServerUpdate::ServerUpdate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerUpdate)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnWidth(0, 200);
    ui->tableWidget->setColumnWidth(1, 200);
    ui->tableWidget->setColumnWidth(2, 55);
    ui->tableWidget->setColumnWidth(3, 55);
    ui->tableWidget->setColumnWidth(4, 55);
    ui->tableWidget->setColumnWidth(5, 70);

    connect(this, &QObject::destroyed, this, &ServerUpdate::saveAndClose);

    ui->tableWidget->setSortingEnabled(true);

    loadFromFile(ipStoragePath);

    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &ServerUpdate::on_tableWidget_cellChanged);
}

ServerUpdate::~ServerUpdate()
{
    delete ui;
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
        for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
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
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Can't open IP adresses storage for loading!");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split("\t");

        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        for (int col = 0; col < parts.size(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(parts[col]);
            ui->tableWidget->setItem(row, col, item);
        }
    }
    file.close();
}

void ServerUpdate::on_addButton_clicked()
{
    int rowCount = ui->tableWidget->rowCount();
    QString newRowName = QString::number(rowCount + 1); // Генерация нового имени строки

    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    // Добавление имени строки в первый столбец новой строки
    QTableWidgetItem *nameItem = new QTableWidgetItem("New Line");
    ui->tableWidget->setItem(row, 0, nameItem);

    // Заполнение пустыми элементами остальных столбцов новой строки
    for (int col = 1; col < ui->tableWidget->columnCount(); ++col) {
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

void ServerUpdate::saveAndClose() {
    if (!isHidden()) {
        saveToFile(ipStoragePath);
        close();
    }
}

void ServerUpdate::closeEvent(QCloseEvent *event) {
    saveToFile(ipStoragePath); // сохранение данных при закрытии
    QWidget::closeEvent(event); // вызов базовой реализации метода
}

void ServerUpdate::copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath) {
    QDir tempDir(tempDirPath);
    QDir targetDir(targetDirPath);

    if (!tempDir.exists()) {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        debugmessage("Temp directory does not exist: " + tempDirPath);
        return;
    }

    if (!targetDir.exists()) {
        qDebug() << "Target directory does not exist: " << targetDirPath;
        debugmessage("Target directory does not exist: " + targetDirPath);
        return;
    }

    QStringList subDirectories = targetDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &subDirectory, subDirectories) {
        QString destinationPath = targetDir.absoluteFilePath(subDirectory);

        QDir destinationDir(destinationPath);
        if (!destinationDir.exists()) {
            qDebug() << "Destination directory does not exist: " << destinationPath;
            debugmessage("Destination directory does not exist: " + destinationPath);
            continue;
        }

        QDirIterator it(tempDirPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QString sourceFilePath = it.filePath();
            QString destinationFilePath = destinationPath + "/" + it.fileName();

            if (QFile::exists(destinationFilePath)) {
                if (!QFile::remove(destinationFilePath)) {
                    qDebug() << "Failed to remove existing file:" << destinationFilePath;
                    debugmessage("Failed to remove existing file: " + destinationFilePath);
                    continue;
                }
            }

            if (!QFile::copy(sourceFilePath, destinationFilePath)) {
                qDebug() << "Failed to copy file:" << sourceFilePath << "to" << destinationFilePath;
                debugmessage("Failed to copy file: " + sourceFilePath + " to " + destinationFilePath);
                continue;
            }

            qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
            debugmessage("Copied file: " + sourceFilePath + " to " + destinationFilePath);
        }
    }
}

void ServerUpdate::removeTempDirectory(const QString &tempDirPath) {
    QDir tempDir(tempDirPath);

    if (tempDir.exists()) {
        if (!tempDir.removeRecursively()) {
            qDebug() << "Failed to remove temp directory: " << tempDirPath;
            debugmessage("Failed to remove temp directory: " + tempDirPath);
            return;
        }

        qDebug() << "Removed temp directory:" << tempDirPath;
        debugmessage("Removed temp directory: " + tempDirPath);
    } else {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        debugmessage("Temp directory does not exist: " + tempDirPath);
    }
}

void ServerUpdate::copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath) {
    QDir sourceDir(sourceDirPath);
    QDir tempDir(tempDirPath);

    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist: " << sourceDirPath;
        debugmessage("Source directory does not exist: " + sourceDirPath);
        return;
    }

    if (!tempDir.exists() && !tempDir.mkpath(".")) {
        qDebug() << "Failed to create temp directory: " << tempDirPath;
        debugmessage("Failed to create temp directory: " + tempDirPath);
        return;
    }

    // Получаем список файлов из исходной директории
    QStringList files = sourceDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    foreach (const QString &file, files) {
        QString sourceFilePath = sourceDirPath + "/" + file;
        QString destinationFilePath = tempDirPath + "/" + file;

        // Копируем файлы из источника во временную папку
        if (!QFile::copy(sourceFilePath, destinationFilePath)) {
            qDebug() << "Failed to copy file:" << sourceFilePath << "to" << destinationFilePath;
            debugmessage("Failed to copy file: " + sourceFilePath + " to " + destinationFilePath);
            continue;
        }

        qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
        debugmessage("Copied file: " + sourceFilePath + " to " + destinationFilePath);
    }
}

void ServerUpdate::updateVersionFile(const QString &sourceDirPath, const QString &destinationDirPath) {
    QDir sourceDir(sourceDirPath);
    QDir destinationDir(destinationDirPath);

    if (!sourceDir.exists() || !destinationDir.exists()) {
        qDebug() << "Source or destination directory does not exist.";
        debugmessage("Source or destination directory does not exist.");
        return;
    }

    QString drugFilePath = sourceDirPath + "/drug.exe";
    QFileInfo drugFileInfo(drugFilePath);
    QDateTime lastModified = drugFileInfo.lastModified();

    QString versionFilePath = destinationDirPath + "/version.txt";
    QFile versionFile(versionFilePath);

    if (!versionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open version file for writing.";
        debugmessage("Failed to open version file for writing.");
        return;
    }

    QTextStream out(&versionFile);
    out << "Last modified: " << lastModified.toString() << "\n";

    versionFile.close();
    qDebug() << "Updated version file in" << destinationDirPath;
    debugmessage("Updated version file in " + destinationDirPath);
}

void ServerUpdate::startUpdate() {
    int reply = QMessageBox::question(this, "Confirmation", "Do you want to close all drug.exe applications on all servers before updating?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        killtask();
        QtConcurrent::run(this, &ServerUpdate::updateInBackground, settingsPath);
    } else if (reply == QMessageBox::No) {
        QtConcurrent::run(this, &ServerUpdate::updateInBackground, settingsPath);
    } else if (reply == QMessageBox::Cancel){
        return;
    }
}

void ServerUpdate::updateInBackground(const QString& settingsPath) {
    // Выполнение операций в фоновом режиме без блокировки интерфейса
    QSettings settings(settingsPath, QSettings::IniFormat);

    QString sourceUpdatePath = settings.value("updateSourcePath").toString();
    QStringList ipAddresses;

    // Получаем список IP-адресов из таблицы
    QMetaObject::invokeMethod(this, [&]() {
            for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
                QTableWidgetItem* ipItem = ui->tableWidget->item(row, 1);
                if (ipItem) {
                    QString ipAddress = ipItem->text();
                    ipAddresses << ipAddress;
                }
            }
        }, Qt::BlockingQueuedConnection);

    // Цикл для каждого IP-адреса
    for (const QString& ipAddress : ipAddresses) {
        QString destinationDirPath = "\\\\" + ipAddress + "\\d$\\Test\\targetContainer";
        QString tempDirForIP = destinationDirPath + "\\temp";

        // Копирование файлов на IP-адрес
        copyFilesToTemp(sourceUpdatePath, tempDirForIP);

        // Копирование содержимого временной папки в поддиректории
        copyTempToAllSubdirectories(tempDirForIP, destinationDirPath);

        // Обновление версионного файла
        updateVersionFile(sourceUpdatePath, destinationDirPath);

        // Удаление временной папки
        removeTempDirectory(tempDirForIP);
    }

    // Обновление интерфейса после завершения операций
    QMetaObject::invokeMethod(this, "updateFinished", Qt::QueuedConnection);
}

void ServerUpdate::on_serverUpdateButton_clicked() {
    startUpdate();
    on_refreshButton_clicked();
}


void ServerUpdate::on_refreshButton_clicked()
{
    QSettings settings(settingsPath, QSettings::IniFormat);

    QString sourceUpdatePath = settings.value("updateSourcePath").toString();

    checkServerAvailability();
    countFolders();
    checkVersionMatch(sourceUpdatePath);
    checkForProcess();
}

void ServerUpdate::checkVersionMatch(const QString &updateSourcePath) {
    QDir updateDir(updateSourcePath);
    if (!updateDir.exists()) {
        qDebug() << "Update directory does not exist.";
        debugmessage("Update directory does not exist.");
        return;
    }

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *ipItem = ui->tableWidget->item(row, 1);
        QTableWidgetItem *versionItem = ui->tableWidget->item(row, 5); // Шестая колонка для записи результатов

        if (ipItem && versionItem) {
            QString ipAddress = ipItem->text();
            QString versionFilePath = "\\\\" + ipAddress + "\\d$\\Test\\targetContainer\\version.txt";

            QFile versionFile(versionFilePath);
            if (!versionFile.exists()) {
                versionItem->setText("unmatch");
                versionItem->setForeground(Qt::red);
                continue;
            }

            if (!versionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug() << "Failed to open version file for reading.";
                debugmessage("Failed to open version file for reading.");
                return;
            }

            QTextStream in(&versionFile);
            QString versionInfo = in.readAll();
            versionFile.close();

            QString updateDrugFilePath = updateSourcePath + "/drug.exe";
            QFileInfo updateDrugFileInfo(updateDrugFilePath);
            QDateTime updateLastModified = updateDrugFileInfo.lastModified();

            if (versionInfo.contains(updateLastModified.toString())) {
                versionItem->setText("match");
                versionItem->setForeground(Qt::darkGreen);
            } else {
                versionItem->setText("unmatch");
                versionItem->setForeground(Qt::red);
            }
        }
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

void ServerUpdate::countFolders() {
    int rowCount = ui->tableWidget->rowCount();

    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem *ipItem = ui->tableWidget->item(row, 1); // Assuming IP addresses are in column 1

        if (ipItem) {
            QString ipAddress = ipItem->text();
            QDir directory("\\\\" + ipAddress + "\\d$\\Test\\targetContainer"); // Modify this path to match your directory structure

            int folderCount = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot).count();

            // Now you can use folderCount as the number of folders for the current IP address
            // You might want to update the table with this count
            QTableWidgetItem *folderCountItem = new QTableWidgetItem(QString::number(folderCount));
            ui->tableWidget->setItem(row, 2, folderCountItem); // Assuming the folder count column is at index 2
        }
    }
}

void ServerUpdate::checkServerAvailability() {
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QString ipAddress = ui->tableWidget->item(row, 1)->text();
        QTableWidgetItem *statusItem = ui->tableWidget->item(row, 3);

        if (!statusItem) {
            statusItem = new QTableWidgetItem();
            ui->tableWidget->setItem(row, 3, statusItem);
        }

        QProcess pingProcess;
        pingProcess.start("ping", QStringList() << "-n" << "1" << ipAddress);
        pingProcess.waitForFinished(-1);

        QString pingOutput = pingProcess.readAllStandardOutput();
        qDebug() << pingOutput;

        if (pingOutput.contains("Reply from")) {
            statusItem->setText("✔"); // зеленая галочка
            statusItem->setData(Qt::ForegroundRole, QBrush(Qt::darkGreen));
        } else {
            statusItem->setText("✖"); // красный крестик
            statusItem->setData(Qt::ForegroundRole, QBrush(Qt::red));
        }
    }
}

void ServerUpdate::checkForProcess() {
    QString username = credentials.value(0);
    QString password = credentials.value(1);

    if (username.isEmpty() || password.isEmpty()) {
        credentials = askCredentials();
        username = credentials.value(0);
        password = credentials.value(1);
    }
    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "Username or password missing";
        debugmessage("Username or password missing");
        return;
    }

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QString ipAddress = ui->tableWidget->item(row, 1)->text();
        QString command = "tasklist /S " + ipAddress + " /U " + username + " /P " + password;

        QProcess process;
        process.start("cmd.exe", QStringList() << "/C" << command);
        process.waitForFinished(-1);

        QString processOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
        QTableWidgetItem *processStatusItem = ui->tableWidget->item(row, 4);

        qDebug() << processOutput;

        if (processOutput.toLower().contains("drug.exe") || processOutput.toLower().contains("drugsys.exe")) {
            processStatusItem->setText("✖"); // красный крестик
            processStatusItem->setData(Qt::ForegroundRole, QBrush(Qt::red));
        } else {
            processStatusItem->setText("✔"); // зеленая галочка
            processStatusItem->setData(Qt::ForegroundRole, QBrush(Qt::darkGreen));
        }
    }
}

void ServerUpdate::on_toolButton_clicked() {

}

void ServerUpdate::killtask() {
    QString username = credentials.value(0);
    QString password = credentials.value(1);

    if (username.isEmpty() || password.isEmpty()) {
        credentials = askCredentials();
        username = credentials.value(0);
        password = credentials.value(1);
    }
    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "Username or password missing";
        debugmessage("Username or password missing");
        return;
    }

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QString ipAddress = ui->tableWidget->item(row, 1)->text();
        closeDrugProcesses(ipAddress, username, password);
    }
}

void ServerUpdate::closeDrugProcesses(const QString& ipAddress, const QString& username, const QString& password) {
    QString processName = "drug.exe"; // Имя процесса для завершения
    QString command = "taskkill /S " + ipAddress + " /U " + username + " /P " + password + " /IM " + processName + " /F";

    QProcess process;
    process.start("cmd.exe", QStringList() << "/C" << command);
    process.waitForFinished(-1);
    debugmessage(processName + " is closed");
}

void ServerUpdate::debugmessage(const QString& message) {
    ui->debugBrowser->append(message);
}
