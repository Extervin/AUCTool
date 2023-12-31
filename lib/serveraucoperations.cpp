﻿#include "serveraucoperations.h"
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

ServerAUCOperations::ServerAUCOperations(QObject *parent) : QObject(parent) {

}

void ServerAUCOperations::copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath) {
    QDir tempDir(tempDirPath);
    QDir targetDir(targetDirPath);

    if (!tempDir.exists()) {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        emit sendDebugMessage("Temp directory does not exist: " + tempDirPath);
        return;
    }

    if (!targetDir.exists()) {
        qDebug() << "Target directory does not exist: " << targetDirPath;
        emit sendDebugMessage("Target directory does not exist: " + targetDirPath);
        return;
    }

    QStringList subDirectories = targetDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &subDirectory, subDirectories) {
        if (subDirectory == "temp") {
            qDebug() << "Skipping 'temp' directory: " << targetDir.absoluteFilePath(subDirectory);
            emit sendDebugMessage("Skipping 'temp' directory: " + targetDir.absoluteFilePath(subDirectory));
            continue;
        }

        QString destinationPath = targetDir.absoluteFilePath(subDirectory);

        QDir destinationDir(destinationPath);
        if (!destinationDir.exists()) {
            qDebug() << "Destination directory does not exist: " << destinationPath;
            emit sendDebugMessage("Destination directory does not exist: " + destinationPath);
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
                    emit sendDebugMessage("Failed to remove existing file: " + destinationFilePath);
                    continue;
                }
            }

            if (!QFile::copy(sourceFilePath, destinationFilePath)) {
                qDebug() << "Failed to copy file:" << sourceFilePath << "to" << destinationFilePath;
                emit sendDebugMessage("Failed to copy file: " + sourceFilePath + " to " + destinationFilePath);
                continue;
            }

            qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
            emit sendDebugMessage("Copied file: " + sourceFilePath + " to " + destinationFilePath);
        }
    }
}




void ServerAUCOperations::removeTempDirectory(const QString &tempDirPath) {
    QDir tempDir(tempDirPath);

    if (tempDir.exists()) {
        if (!tempDir.removeRecursively()) {
            qDebug() << "Failed to remove temp directory: " << tempDirPath;
            emit sendDebugMessage("Failed to remove temp directory: " + tempDirPath);
            return;
        }

        qDebug() << "Removed temp directory:" << tempDirPath;
        emit sendDebugMessage("Removed temp directory:" + tempDirPath);
    } else {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        emit sendDebugMessage("Temp directory does not exist: " + tempDirPath);
    }
}

void ServerAUCOperations::copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath) {
    QDir sourceDir(sourceDirPath);
    QDir tempDir(tempDirPath);

    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist: " << sourceDirPath;
        emit sendDebugMessage("Source directory does not exist: " + sourceDirPath);
        return;
    }

    if (!tempDir.exists() && !tempDir.mkpath(".")) {
        qDebug() << "Failed to create temp directory: " << tempDirPath;
        emit sendDebugMessage("Failed to create temp directory: " + tempDirPath);
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
            emit sendDebugMessage("Failed to copy file:" + sourceFilePath + "to" + destinationFilePath);
            continue;
        }

        qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
        emit sendDebugMessage("Copied file: " + sourceFilePath + " to " + destinationFilePath);
    }
}



bool ServerAUCOperations::updateVersionFile(const QString &sourceDirPath, const QString &destinationDirPath) {
    QString filePath = destinationDirPath + "/version.txt";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing.";
        emit sendDebugMessage("Failed to open file for writing.");
        return false;
    }

    QTextStream in(&file);

    // Считываем содержимое файла в QMap для последующего сравнения
    QMap<QString, QString> existingFiles;
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(" - Last modified: ");
        if (parts.length() == 2) {
            existingFiles[parts[0]] = parts[1];
        }
    }

    QDir sourceDir(sourceDirPath);
    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist.";
        emit sendDebugMessage("Source directory does not exist.");
        file.close();
        return false;
    }

    QStringList fileNames = sourceDir.entryList(QDir::Files);
    QTextStream out(&file);
    foreach (const QString &fileName, fileNames) {
        QString filePath = sourceDir.filePath(fileName);
        QFileInfo fileInfo(filePath);
        QDateTime lastModified = fileInfo.lastModified();

        // Проверяем, есть ли файл уже в списке. Если есть, обновляем информацию
        if (existingFiles.contains(fileName)) {
            if (existingFiles[fileName] != lastModified.toString()) {
                out << fileName << " - Last modified: " << lastModified.toString() << "\n";
                qDebug() << "Updated file information for" << fileName;
                emit sendDebugMessage("Updated file information for " + fileName);
            }
        } else {
            // Если файла не было в списке, добавляем его
            out << fileName << " - Last modified: " << lastModified.toString() << "\n";
            qDebug() << "Added file information for" << fileName;
            emit sendDebugMessage("Added file information for " + fileName);
        }
    }

    file.close();
    qDebug() << "Finished updating file information.";
    emit sendDebugMessage("Finished updating file information.");

    return true;
}

void ServerAUCOperations::connectToNetworkShare(const QString& server, const QString& share, const QString& username, const QString& password) {

    QString connectCommand = QString("net use Z: \\\\%1\\%2 /user:%4 %3").arg(server, share, password, username);
    int result = QProcess::execute(connectCommand);
    if (result == 0) {
        qDebug() << "Connected to " + server + " successfully.";
        emit sendDebugMessage("Connected to " + server + " successfully.");
    } else {
        qDebug() << "Failed to connect to " + server + " via Z:\\ path";
        emit sendDebugMessage("Failed to connect to " + server + " via Z:\\ path");
    }
}

void ServerAUCOperations::disconnectFromNetworkShare() {
    QString disconnectCommand = QString("net use Z: /delete /y");
    int result = QProcess::execute(disconnectCommand);
    if (result == 0) {
        qDebug() << "Disconnected from server successfully.";
        emit sendDebugMessage("Disconnected from server successfully");
    } else {
        qDebug() << "Failed to disconnect from server";
        emit sendDebugMessage("Failed to disconnect from server");
    }
}

void ServerAUCOperations::updateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, std::vector<int> rowID) {
    QSettings settings(settingsPath, QSettings::IniFormat);

    emit sendDebugMessage("Attempting to clear the path Z:\\ to connect to the server");
    disconnectFromNetworkShare();
    int i = 0;
    // Цикл для каждого IP-адреса
    for (const QString& ipAddress : ipAddresses) {
        int row = rowID[i];

        QString tempDirForIP = "Z:\\temp";

        connectToNetworkShare(ipAddress, settings.value("serverUpdatePath", "").toString(), username, password);

        // Копирование файлов на IP-адрес
        copyFilesToTemp(sourcePath, tempDirForIP);

        // Копирование содержимого временной папки в поддиректории
        copyTempToAllSubdirectories(tempDirForIP, "Z:\\");

        // Обновление версионного файла
        if (updateVersionFile(sourcePath, "Z:\\")) {
            qDebug() << "File information updated or created successfully.";
            emit sendDebugMessage("File information updated or created successfully.");
            int check = checkVersionMatch(sourcePath);
            if (check == 1) {
                emit sendTableValue(row, 6, "match", Qt::darkGreen);
            } else if (check == 0) {
                emit sendTableValue(row, 6, "unmatch", Qt::red);
            } else if (check == 2) {
               emit sendTableValue(row, 6, "error", Qt::red);
            }
        } else {
            qDebug() << "Failed to update or create file information.";
            emit sendDebugMessage("Failed to update or create file information.");
            emit sendTableValue(row, 6, "error", Qt::red);
        }

        // Удаление временной папки
        removeTempDirectory(tempDirForIP);

        disconnectFromNetworkShare();

        emit sendDebugMessage(ipAddress + " update completed");
        if (i < rowID.size()) {
            i++;
        }
    }
}

void ServerAUCOperations::closeAndUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, std::vector<int> rowID) {
    QSettings settings(settingsPath, QSettings::IniFormat);

    emit sendDebugMessage("Attempting to clear the path Z:\\ to connect to the server");
    disconnectFromNetworkShare();
    int i = 0;
    // Цикл для каждого IP-адреса
    for (const QString& ipAddress : ipAddresses) {
        int row = rowID[i];

        QString tempDirForIP = "Z:\\temp";

        connectToNetworkShare(ipAddress, settings.value("serverUpdatePath", "").toString(), username, password);

        // Копирование файлов на IP-адрес
        copyFilesToTemp(sourcePath, tempDirForIP);

        closeDrugProcesses(ipAddress, username, password);

        // Копирование содержимого временной папки в поддиректории
        copyTempToAllSubdirectories(tempDirForIP, "Z:\\");

        // Обновление версионного файла
        if (updateVersionFile(sourcePath, "Z:\\")) {
            qDebug() << "File information updated or created successfully.";
            emit sendDebugMessage("File information updated or created successfully.");
            int check = checkVersionMatch(sourcePath);
            if (check == 1) {
               emit sendTableValue(row, 6, "match", Qt::darkGreen);
            } else if (check == 0) {
               emit sendTableValue(row, 6, "unmatch", Qt::red);
            } else if (check == 2) {
               emit sendTableValue(row, 6, "error", Qt::red);
            }
        } else {
            qDebug() << "Failed to update or create file information.";
            emit sendDebugMessage("Failed to update or create file information.");
            emit sendTableValue(row, 6, "error", Qt::red);
        }

        // Удаление временной папки
        removeTempDirectory(tempDirForIP);

        disconnectFromNetworkShare();

        emit sendDebugMessage(ipAddress + " update completed");
        if (i < rowID.size()) {
            i++;
        }
    }
}

void ServerAUCOperations::refreshInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, std::vector<int> rowID) {
    QSettings settings(settingsPath, QSettings::IniFormat);

    int i = 0;

    qDebug() << "i = " << i;

    for (const QString& ipAddress : ipAddresses) {
        int row = rowID[i];
        qDebug() << "row = " << row;
        disconnectFromNetworkShare();

        int serverAvailability = checkServerAvailability(ipAddress);
        qDebug() << "serverAvailability = " << serverAvailability;
        if (serverAvailability == 1) {
            emit sendTableValue(row, 4, "✔", Qt::darkGreen);
            qDebug() << "ping working";
        }

        if (serverAvailability == 0 || serverAvailability == 2) {
            // добавить приколов для отображения
            emit sendTableValue(row, 3, "err", Qt::red);
            emit sendTableValue(row, 4, "✖", Qt::red);
            emit sendTableValue(row, 5, "err", Qt::red);
            emit sendTableValue(row, 6, "error", Qt::red);
            qDebug() << "ping not working";
            continue; // Пропускаем остальные проверки, если сервер недоступен или есть ошибка
        }

        connectToNetworkShare(ipAddress, settings.value("serverUpdatePath", "").toString(), username, password);

        int folderCount = countFolders();
        qDebug() << "folderCount = " << folderCount;
        if (folderCount == 0) {
            emit sendTableValue(row, 3, "error", Qt::red);
        } else {
            emit sendTableValue(row, 3, QString::number(folderCount), Qt::black);
        }

        int versionMatch = checkVersionMatch(sourcePath);
        qDebug() << "versionMatch = " << versionMatch;
        if (versionMatch == 1) {
            emit sendTableValue(row, 6, "match", Qt::darkGreen);
        } else if (versionMatch == 0) {
            emit sendTableValue(row, 6, "unmatch", Qt::red);
        } else if (versionMatch == 2) {
            emit sendTableValue(row, 6, "error", Qt::red);
        }

        int processCheck = checkForProcess(username, password, ipAddress);
        qDebug() << "processCheck = " << processCheck;
        if (processCheck == 0) {
            emit sendTableValue(row, 5, "closed", Qt::darkGreen);
        } else if (processCheck == 1) {
            emit sendTableValue(row, 5, "opened", Qt::red);
        } else if (processCheck == 2) {
            emit sendTableValue(row, 5, "error", Qt::red);
        }

        disconnectFromNetworkShare();

        if (i < rowID.size()) {
            i++;
            qDebug() << i;
        }
    }
        //1 yes 0 no 2 error

}

int ServerAUCOperations::checkServerAvailability(QString ipAddress) {
    QProcess pingProcess;
    pingProcess.start("ping", QStringList() << "-n" << "1" << ipAddress);
    pingProcess.waitForFinished(-1);

    QString pingOutput = pingProcess.readAllStandardOutput();
    if (pingOutput.contains("Reply from")) {
        return 1;
    } else if (pingOutput == "") {
        return 2;
    } else {
        return 0;
    }
}

int ServerAUCOperations::checkVersionMatch(const QString& updateSourcePath) {
    QString versionFilePath = "Z:\\version.txt";
    QFile versionFile(versionFilePath);

    if (!versionFile.exists()) {
        qDebug() << "Version file does not exist.";
        return 0;
    }
    if (!versionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open version file for reading.";
        emit sendDebugMessage("Failed to open version file for reading.");
        return 2;
    }

    QTextStream in(&versionFile);
    QMap<QString, QString> versionInfo; // Теперь используем QMap<QString, QString>

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(" - Last modified: ");
        if (parts.length() == 2) {
            QString fileName = parts[0];
            QString lastModified = parts[1]; // Сохраняем дату как строку
            versionInfo[fileName] = lastModified;
        }
    }

    versionFile.close();

    QDir sourceDir(updateSourcePath);
    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist.";
        emit sendDebugMessage("Source directory does not exist.");
        return 2;
    }

    QStringList fileNames = sourceDir.entryList(QDir::Files);
    foreach (const QString &fileName, fileNames) {
        QString filePath = sourceDir.filePath(fileName);
        QFileInfo fileInfo(filePath);
        QString lastModified = fileInfo.lastModified().toString(); // Сохраняем дату как строку

        if (versionInfo.contains(fileName)) {
            qDebug() << "Comparing file:" << fileName;
            QString versionDateString = versionInfo[fileName];
            qDebug() << versionDateString << " and " << lastModified;

            if (versionDateString != lastModified) {
                qDebug() << "Mismatch for file:" << fileName;
                return 0;
            }
        } else {
            qDebug() << "File not found in version.txt:" << fileName;
            return 0; // Если хотя бы один файл отсутствует в version.txt, считаем несовпадением
        }
    }

    return 1;
}


int ServerAUCOperations::countFolders() {
    QDir directory("Z:\\");
    int folderCount = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot).count();
    return folderCount;
}

int ServerAUCOperations::checkForProcess(const QString& username, const QString& password, QString ipAddress) {

    QString command = "tasklist /S " + ipAddress + " /U " + username + " /P " + password;
    QProcess process;
    process.start("cmd.exe", QStringList() << "/C" << command);
    process.waitForFinished(-1);

    QString processOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
    if (processOutput.contains("ERROR:") || processOutput.contains("Access is denied")) {
        qDebug() << "Incorrect password";
        emit sendDebugMessage("Incorrect password");
        return 2;
    }

    if (processOutput.toLower().contains("drug.exe") || processOutput.toLower().contains("drugsys.exe")) { // i hate useless warnings about unneeded allocation
        return 1;
    } else {
        return 0;
    }
}

void ServerAUCOperations::closeDrugProcesses(const QString& ipAddress, const QString& username, const QString& password) {
    QString processName = "drug.exe"; // Имя процесса для завершения
    QString command = "taskkill /S " + ipAddress + " /U " + username + " /P " + password + " /IM " + processName + " /F";

    QProcess process;
    process.start("cmd.exe", QStringList() << "/C" << command);
    process.waitForFinished(-1);
    //обработать ошибки
}



