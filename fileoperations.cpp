#include "fileoperations.h"
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

FileOperations::FileOperations(QObject *parent) : QObject(parent) {}

void FileOperations::closeAndUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, std::vector<int> rowID) {
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
        bool copyFlag = copyTempToAllSubdirectories(tempDirForIP, "Z:\\");

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

        if (copyFlag == false) {
            emit sendDebugMessage("Update wasn't fully completed.");
            emit sendTableValue(row, 6, "not full", Qt::red);
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

bool FileOperations::copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath) {
    QDir tempDir(tempDirPath);
    QDir targetDir(targetDirPath);
    bool flag = true;

    if (!tempDir.exists()) {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        emit sendDebugMessage("Temp directory does not exist: " + tempDirPath);
        return true;
    }

    if (!targetDir.exists()) {
        qDebug() << "Target directory does not exist: " << targetDirPath;
        emit sendDebugMessage("Target directory does not exist: " + targetDirPath);
        return true;
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
            flag = false;
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
                    flag = false;
                    continue;
                }
            }

            if (!QFile::copy(sourceFilePath, destinationFilePath)) {
                qDebug() << "Failed to copy file:" << sourceFilePath << "to" << destinationFilePath;
                emit sendDebugMessage("Failed to copy file: " + sourceFilePath + " to " + destinationFilePath);
                flag = false;
                continue;
            }

            qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
            emit sendDebugMessage("Copied file: " + sourceFilePath + " to " + destinationFilePath);
        }
    }
    return flag;
}




void FileOperations::removeTempDirectory(const QString &tempDirPath) {
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

void FileOperations::copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath) {
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



bool FileOperations::updateVersionFile(const QString &sourceDirPath, const QString &destinationDirPath) {
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

void FileOperations::connectToNetworkShare(const QString& server, const QString& share, const QString& username, const QString& password) {

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

void FileOperations::disconnectFromNetworkShare() {
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
