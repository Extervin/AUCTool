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

//void ServerUpdate::updateAll() {

//}

//void ServerUpdate::processCheck() {

//}

//void ServerUpdate::countFolders() {

//}

//void ServerUpdate::versionCheck() {

//}

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

void ServerUpdate::on_serverUpdateButton_clicked() {
    QSettings settings(settingsPath, QSettings::IniFormat);

    QString sourceDirPath = settings.value("updateSourcePath").toString();
    QDir sourceDir(sourceDirPath);

    QStringList ipAddresses;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *ipItem = ui->tableWidget->item(row, 1);
        if (ipItem) {
            QString ipAddress = ipItem->text();
            ipAddresses << ipAddress;
        }
    }

    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist.";
        return;
    }

    QDirIterator it(sourceDirPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();

        QString sourceFilePath = it.filePath();

        foreach (const QString &ipAddress, ipAddresses) {
            QString destinationPath = "\\\\" + ipAddress + "\\d$\\Test\\targetContainer";

            QDir destinationDir(destinationPath);
            if (!destinationDir.exists()) {
                qDebug() << "Destination directory does not exist: " << destinationPath;
                continue;
            }

            QDirIterator targetDirs(destinationPath, QDir::Dirs | QDir::NoDotAndDotDot);
            while (targetDirs.hasNext()) {
                targetDirs.next();

                QString currentTargetDir = targetDirs.filePath();
                QString relativePath = it.filePath().mid(sourceDirPath.length());

                QString destinationFilePath = currentTargetDir + "/" + relativePath;
                QDir destinationFileDir(QFileInfo(destinationFilePath).absolutePath());
                if (!destinationFileDir.exists()) {
                    destinationFileDir.mkpath(".");
                }

                QFile::copy(sourceFilePath, destinationFilePath);
                qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
            }
        }
    }
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
    QStringList credentials = askCredentials();

    if (credentials.isEmpty()) {
        // Отменено пользователем или не введены учетные данные
        return;
    }

    QString username = credentials.value(0);
    QString password = credentials.value(1);

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QString ipAddress = ui->tableWidget->item(row, 1)->text();
        QString command = "tasklist /S " + ipAddress + " /U " + username + " /P " + password;


        QProcess process;
        process.start("cmd.exe", QStringList() << "/C" << command);
        process.waitForFinished(-1);

        QString processOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
        QTableWidgetItem *processStatusItem = ui->tableWidget->item(row, 4);

        qDebug() << processOutput;

        if (processOutput.contains("drug.exe") || processOutput.contains("DRUG.EXE") ||
            processOutput.contains("drugsys.exe") || processOutput.contains("DRUGSYS.EXE")) {
            processStatusItem->setText("✖"); // красный крестик
            processStatusItem->setData(Qt::ForegroundRole, QBrush(Qt::red));
        } else {
            processStatusItem->setText("✔"); // зеленая галочка
            processStatusItem->setData(Qt::ForegroundRole, QBrush(Qt::darkGreen));
        }
    }
}
