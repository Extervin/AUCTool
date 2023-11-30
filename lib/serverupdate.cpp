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
#include <quazip.h>
#include <quazipfile.h>

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
    ui->tableWidget->setColumnWidth(5, 55);

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
        QMessageBox::warning(this, "Ошибка", "Невозможно открыть файл для сохранения!");
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
        QMessageBox::warning(this, "Ошибка", "Невозможно открыть файл для загрузки!");
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

void ServerUpdate::extractZipArchiveToTemp(const QString &sourceFilePath, const QString &tempDirPath) {
    QuaZip zip(sourceFilePath);
    if (!zip.open(QuaZip::mdUnzip)) {
        qDebug() << "Failed to open ZIP archive for extraction.";
        return;
    }

    QDir tempDir(tempDirPath);
    if (!tempDir.exists() && !tempDir.mkpath(".")) {
        qDebug() << "Failed to create temp directory: " << tempDirPath;
        return;
    }

    QuaZipFileInfo info;
    QuaZipFile file(&zip);
    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
        if (!zip.getCurrentFileInfo(&info)) {
            qDebug() << "Failed to get current file info from ZIP archive.";
            return;
        }

        QString filePath = tempDirPath + "/" + info.name;
        QFile outFile(filePath);
        if (!outFile.open(QIODevice::WriteOnly)) {
            qDebug() << "Failed to open output file for extraction.";
            return;
        }

        file.setFileName(info.name);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open file in ZIP archive.";
            outFile.close();
            return;
        }

        outFile.write(file.readAll());
        file.close();
        outFile.close();
    }

    zip.close();
    qDebug() << "Extracted ZIP archive:" << sourceFilePath << "to" << tempDirPath;
}


void ServerUpdate::copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath) {
    QDir tempDir(tempDirPath);
    QDir targetDir(targetDirPath);

    if (!tempDir.exists()) {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        return;
    }

    if (!targetDir.exists()) {
        qDebug() << "Target directory does not exist: " << targetDirPath;
        return;
    }

    QStringList subDirectories = targetDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &subDirectory, subDirectories) {
        QString subDirPath = targetDir.absoluteFilePath(subDirectory);
        QString destinationPath = subDirPath + "/" + tempDir.dirName();

        QDir destinationDir(destinationPath);
        if (!destinationDir.exists()) {
            qDebug() << "Destination directory does not exist: " << destinationPath;
            continue;
        }

        QDirIterator it(tempDirPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QString sourceFilePath = it.filePath();
            QString destinationFilePath = destinationPath + "/" + it.fileName();

            if (!QFile::copy(sourceFilePath, destinationFilePath)) {
                qDebug() << "Failed to copy file:" << sourceFilePath << "to" << destinationFilePath;
                continue;
            }
            qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
        }
    }
}

void ServerUpdate::deleteArchiveAndTemp(const QString &sourceFilePath, const QString &tempDirPath) {
    QFile archive(sourceFilePath);
    QDir tempDir(tempDirPath);

    if (archive.exists()) {
        archive.remove();
        qDebug() << "Deleted archive:" << sourceFilePath;
    }

    if (tempDir.exists()) {
        tempDir.removeRecursively();
        qDebug() << "Deleted temp directory:" << tempDirPath;
    }
}

void ServerUpdate::copyArchiveToIP(const QString &sourceFilePath, const QString &destinationDirPath) {
    QFile sourceFile(sourceFilePath);

    if (!sourceFile.exists()) {
        qDebug() << "Source file does not exist.";
        return;
    }

    QDir destinationDir(destinationDirPath);
    if (!destinationDir.exists()) {
        qDebug() << "Destination directory does not exist: " << destinationDirPath;
        return;
    }

    QString destinationFilePath = destinationDirPath + "/" + QFileInfo(sourceFilePath).fileName();
    if (!QFile::copy(sourceFilePath, destinationFilePath)) {
        qDebug() << "Failed to copy file to:" << destinationDirPath;
        return;
    }

    qDebug() << "Copied file:" << sourceFilePath << "to" << destinationDirPath;
}

void ServerUpdate::update() {
    // Шаг 1: Архивирование обновления
    QSettings settings(settingsPath, QSettings::IniFormat);

    QString sourceUpdatePath = settings.value("updateSourcePath").toString();
    createUpdateArchive(sourceUpdatePath);

    // Получаем список IP-адресов из таблицы
    QStringList ipAddresses;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *ipItem = ui->tableWidget->item(row, 1);
        if (ipItem) {
            QString ipAddress = ipItem->text();
            ipAddresses << ipAddress;
        }
    }
    // Шаг 2: Цикл для каждого IP-адреса
    QString sourceFilePath = sourceUpdatePath + "/update.zip";

    for (const QString &ipAddress : ipAddresses) {
        QString destinationDirPath = "\\\\" + ipAddress + "\\d$\\Test\\targetContainer";
        QString tempDirForIP = destinationDirPath + "\\temp";
        qDebug() << "Temp directory for IP:" << tempDirForIP;
        qDebug() << "Source archive path:" << sourceFilePath;
        qDebug() << "Copying from:" << tempDirForIP << "to:" << destinationDirPath;
        qDebug() << "Deleting archive:" << sourceFilePath;
        qDebug() << "Deleting temp directory:" << tempDirForIP;

        // Копирование архива на IP-адрес
        copyArchiveToIP(sourceFilePath, destinationDirPath);

        // Разархивирование архива во временную папку
        extractZipArchiveToTemp(destinationDirPath + "/" + QFileInfo(sourceFilePath).fileName(), tempDirForIP);

        // Копирование содержимого временной папки в поддиректории
        copyTempToAllSubdirectories(tempDirForIP, destinationDirPath);

        // Удаление архива и временной папки
        deleteArchiveAndTemp(destinationDirPath + "\\update.zip", tempDirForIP);
    }
}

void ServerUpdate::on_serverUpdateButton_clicked() {
    update();
    on_refreshButton_clicked();
}


void ServerUpdate::on_refreshButton_clicked()
{
    checkServerAvailability();
    countFolders();
    checkForProcess();
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
        qDebug() << "Не введены учетные данные";
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
    QSettings settings(settingsPath, QSettings::IniFormat);

    QString sourceDirectory = settings.value("updateSourcePath").toString();
    createUpdateArchive(sourceDirectory);
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
        qDebug() << "Не введены учетные данные";
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
}

void ServerUpdate::addFilesToArchive(const QString &sourceDir, QuaZip &zip, const QString &dirPath) {
    QDir directory(sourceDir);
    directory.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    foreach (const QString &file, directory.entryList()) {
        QString filePath = sourceDir + "/" + file;
        QString relativePath = QDir(sourceDir).relativeFilePath(filePath);

        if (!dirPath.isEmpty()) {
            relativePath = dirPath + "/" + relativePath;
        }

        QFileInfo fileInfo(filePath);
        if (fileInfo.isDir()) {
            QuaZipFile dirFile(&zip);
            QuaZipNewInfo info(relativePath + "/");
            if (!dirFile.open(QIODevice::WriteOnly, info)) {
                qWarning() << "Could not write directory to ZIP: " << filePath;
                continue;
            }
            dirFile.close();
            addFilesToArchive(filePath, zip, relativePath);
        } else {
            // Проверяем, чтобы файл не был созданным архивом
            if (file != "update.zip") {
                QuaZipFile outFile(&zip);
                QuaZipNewInfo info(relativePath);
                if (!outFile.open(QIODevice::WriteOnly, info)) {
                    qWarning() << "Could not write file to ZIP: " << filePath;
                    continue;
                }

                QFile fileToAdd(filePath);
                if (!fileToAdd.open(QIODevice::ReadOnly)) {
                    qWarning() << "Could not open file: " << filePath;
                    outFile.close();
                    continue;
                }

                outFile.write(fileToAdd.readAll());

                outFile.close();
                fileToAdd.close();
            }
        }
    }
}

void ServerUpdate::createUpdateArchive(const QString &sourceDir) {
    QString zipPath = sourceDir + "/update.zip";

    QuaZip zip(zipPath);
    if (!zip.open(QuaZip::mdCreate)) {
        qWarning() << "Could not create ZIP archive";
        return;
    }

    QStringList excludeFiles;
    excludeFiles << "update.zip";

    addFilesToArchive(sourceDir, zip);
    debugmessage("Update package created");

    zip.close();
}

void ServerUpdate::debugmessage(const QString& message) {
    ui->debugBrowser->append(message);
}
