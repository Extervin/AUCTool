#include "serveraucoperations.h"
#include "ui_serverupdate.h"
#include "serverupdate.h"
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

ServerAUCOperations::ServerAUCOperations()
{

}

void ServerAUCOperations::copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath) {
    QDir tempDir(tempDirPath);
    QDir targetDir(targetDirPath);

    if (!tempDir.exists()) {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        emit setdebugmessage("Temp directory does not exist: " + tempDirPath);
        return;
    }

    if (!targetDir.exists()) {
        qDebug() << "Target directory does not exist: " << targetDirPath;
        emit setdebugmessage("Target directory does not exist: " + targetDirPath);
        return;
    }

    QStringList subDirectories = targetDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &subDirectory, subDirectories) {
        QString destinationPath = targetDir.absoluteFilePath(subDirectory);

        QDir destinationDir(destinationPath);
        if (!destinationDir.exists()) {
            qDebug() << "Destination directory does not exist: " << destinationPath;
            emit setdebugmessage("Destination directory does not exist: " + destinationPath);
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
                    emit setdebugmessage("Failed to remove existing file: " + destinationFilePath);
                    continue;
                }
            }

            if (!QFile::copy(sourceFilePath, destinationFilePath)) {
                qDebug() << "Failed to copy file:" << sourceFilePath << "to" << destinationFilePath;
                emit setdebugmessage("Failed to copy file: " + sourceFilePath + " to " + destinationFilePath);
                continue;
            }

            qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
            emit setdebugmessage("Copied file: " + sourceFilePath + " to " + destinationFilePath);
        }
    }
}

void ServerAUCOperations::removeTempDirectory(const QString &tempDirPath) {
    QDir tempDir(tempDirPath);

    if (tempDir.exists()) {
        if (!tempDir.removeRecursively()) {
            qDebug() << "Failed to remove temp directory: " << tempDirPath;
            emit setdebugmessage("Failed to remove temp directory: " + tempDirPath);
            return;
        }

        qDebug() << "Removed temp directory:" << tempDirPath;
        emit setdebugmessage("Removed temp directory: " + tempDirPath);
    } else {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        emit setdebugmessage("Temp directory does not exist: " + tempDirPath);
    }
}

void ServerAUCOperations::copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath) {
    QDir sourceDir(sourceDirPath);
    QDir tempDir(tempDirPath);

    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist: " << sourceDirPath;
        emit setdebugmessage("Source directory does not exist: " + sourceDirPath);
        return;
    }

    if (!tempDir.exists() && !tempDir.mkpath(".")) {
        qDebug() << "Failed to create temp directory: " << tempDirPath;
        emit setdebugmessage("Failed to create temp directory: " + tempDirPath);
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
            emit setdebugmessage("Failed to copy file: " + sourceFilePath + " to " + destinationFilePath);
            continue;
        }

        qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
        emit setdebugmessage("Copied file: " + sourceFilePath + " to " + destinationFilePath);
    }
}

void ServerAUCOperations::updateVersionFile(const QString &sourceDirPath, const QString &destinationDirPath) {
    QDir sourceDir(sourceDirPath);
    QDir destinationDir(destinationDirPath);

    if (!sourceDir.exists() || !destinationDir.exists()) {
        qDebug() << "Source or destination directory does not exist.";
        emit setdebugmessage("Source or destination directory does not exist.");
        return;
    }

    QString drugFilePath = sourceDirPath + "/drug.exe";
    QFileInfo drugFileInfo(drugFilePath);
    QDateTime lastModified = drugFileInfo.lastModified();

    QString versionFilePath = destinationDirPath + "/version.txt";
    QFile versionFile(versionFilePath);

    if (!versionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open version file for writing.";
        emit setdebugmessage("Failed to open version file for writing.");
        return;
    }

    QTextStream out(&versionFile);
    out << "Last modified: " << lastModified.toString() << "\n";

    versionFile.close();
    qDebug() << "Updated version file in" << destinationDirPath;
    emit setdebugmessage("Updated version file in " + destinationDirPath);
}

void ServerAUCOperations::connectToNetworkShare(const QString& server, const QString& share, const QString& username, const QString& password) {
    emit setdebugmessage("Trying to connect via Z:\\ path...");
    QString connectCommand = QString("net use Z: \\\\%1\\%2 %3 /user:%4").arg(server, share, password, username);
    int result = QProcess::execute(connectCommand);
    if (result == 0) {
        qDebug() << "Connected to network share successfully.";
        emit setdebugmessage("Connected to" + server + " successfully, ");
    } else {
        qDebug() << "Failed to connect to network share via Z:\\ path";
    }
}

void ServerAUCOperations::disconnectFromNetworkShare() {
    QString disconnectCommand = QString("net use Z: /delete /y");
    int result = QProcess::execute(disconnectCommand);
    if (result == 0) {
        qDebug() << "Disconnected from network share successfully.";
    } else {
        qDebug() << "Failed to disconnect from network share.";
    }
}

void ServerAUCOperations::updateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password) {

    emit setdebugmessage("Closing connections on path Z:\\...");
    disconnectFromNetworkShare();

    // Цикл для каждого IP-адреса
    for (const QString& ipAddress : ipAddresses) {
        QString destinationDirPath = "\\\\" + ipAddress + "\\d$\\Test\\targetContainer";
        QString tempDirForIP = "Z:\\temp";

        connectToNetworkShare(ipAddress, "d$\\Test\\targetContainer", username, password);

        // Копирование файлов на IP-адрес
        copyFilesToTemp(sourcePath, tempDirForIP);

        // Копирование содержимого временной папки в поддиректории
        copyTempToAllSubdirectories(tempDirForIP, "Z:\\");

        // Обновление версионного файла
        updateVersionFile(sourcePath, "Z:\\");

        // Удаление временной папки
        removeTempDirectory(tempDirForIP);

        disconnectFromNetworkShare();
    }
}

void ServerAUCOperations::checkVersionMatch(const QString& updateSourcePath, QTableWidget* tableWidget) {
    if (!tableWidget)
        return;

    QDir updateDir(updateSourcePath);
    if (!updateDir.exists()) {
        qDebug() << "Update directory does not exist.";
        // Обновление интерфейса или выдача сообщения об ошибке, если нужно
        return;
    }

    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QTableWidgetItem* ipItem = tableWidget->item(row, 1);
        QTableWidgetItem* versionItem = tableWidget->item(row, 5); // Шестая колонка для записи результатов

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
                // Обновление интерфейса или выдача сообщения об ошибке, если нужно
                continue;
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

void ServerAUCOperations::countFolders(QTableWidget* tableWidget) {
    if (!tableWidget)
        return;

    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QTableWidgetItem* ipItem = tableWidget->item(row, 1); // Предполагая, что IP-адрес находится во второй колонке

        if (ipItem) {
            QString ipAddress = ipItem->text();
            QDir directory("\\\\" + ipAddress + "\\d$\\Test\\targetContainer"); // Модифицируйте путь в соответствии с вашей структурой

            int folderCount = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot).count();

            QTableWidgetItem* folderCountItem = new QTableWidgetItem(QString::number(folderCount));
            tableWidget->setItem(row, 2, folderCountItem); // Предполагая, что колонка для папок находится в третьей колонке
        }
    }
}

void ServerAUCOperations::checkServerAvailability(QTableWidget* tableWidget) {
    if (!tableWidget)
        return;

    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QString ipAddress = tableWidget->item(row, 1)->text();
        QTableWidgetItem* statusItem = tableWidget->item(row, 3);

        if (!statusItem) {
            statusItem = new QTableWidgetItem();
            tableWidget->setItem(row, 3, statusItem);
        }

        QProcess pingProcess;
        pingProcess.start("ping", QStringList() << "-n" << "1" << ipAddress);
        pingProcess.waitForFinished(-1);

        QString pingOutput = pingProcess.readAllStandardOutput();

        if (pingOutput.contains("Reply from")) {
            statusItem->setText("✔"); // зеленая галочка
            statusItem->setData(Qt::ForegroundRole, QBrush(Qt::darkGreen));
        } else {
            statusItem->setText("✖"); // красный крестик
            statusItem->setData(Qt::ForegroundRole, QBrush(Qt::red));
        }
    }
}

void ServerAUCOperations::checkForProcess(const QString& username, const QString& password, QTableWidget* tableWidget) {
    if (!tableWidget)
        return;

    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        QString ipAddress = tableWidget->item(row, 1)->text();

        QString command = "tasklist /S " + ipAddress + " /U " + username + " /P " + password;

        QProcess process;
        process.start("cmd.exe", QStringList() << "/C" << command);
        process.waitForFinished(-1);

        QString processOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
        QTableWidgetItem* processStatusItem = tableWidget->item(row, 4);

        if (processOutput.contains("ERROR:") || processOutput.contains("Access is denied")) {
            qDebug() << "Incorrect password";
            processStatusItem->setText("err");
            return;
        }

        if (processOutput.toLower().contains("drug.exe") || processOutput.toLower().contains("drugsys.exe")) {
            processStatusItem->setText("✖"); // красный крестик
            processStatusItem->setData(Qt::ForegroundRole, QBrush(Qt::red));
        } else {
            processStatusItem->setText("✔"); // зеленая галочка
            processStatusItem->setData(Qt::ForegroundRole, QBrush(Qt::darkGreen));
        }
    }
}

void ServerAUCOperations::closeDrugProcesses(const QString& ipAddress, const QString& username, const QString& password) {
    QString processName = "drug.exe"; // Имя процесса для завершения
    QString command = "taskkill /S " + ipAddress + " /U " + username + " /P " + password + " /IM " + processName + " /F";

    QProcess process;
    process.start("cmd.exe", QStringList() << "/C" << command);
    process.waitForFinished(-1);
    emit setdebugmessage(processName + " is closed");
}



