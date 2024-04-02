#include "fileoperations.h"
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QDateTime>

FileOperations::FileOperations(QObject *parent) : QObject(parent) {

}

void FileOperations::closeAndUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, QVector<int> rowID) {
    // Отключаемся от сетевой шары перед началом работы
    //disconnectFromNetworkShare();

    QString tempDirForIP = "Z:\\temp"; // Создаем временную директорию для конкретного IP-адреса

    // Подключаемся к сетевой шаре с использованием переданных учетных данных и IP-адреса
    //connectToNetworkShare(ipAddress, "serverupdatepath", username, password);

    // Копируем файлы из исходной директории во временную директорию для текущего IP-адреса
    //copyFilesToTemp(sourcePath, tempDirForIP);

    // Закрываем процессы drug.exe на удаленной машине

    // Копируем содержимое временной директории в поддиректории на удаленной машине
    //copyTempToAllSubdirectories(tempDirForIP, "Z:\\");

    // Обновляем версионный файл на удаленной машине
    //updateVersionFile(sourcePath, "Z:\\");

    // Удаляем временную директорию для текущего IP-адреса
    //removeTempDirectory(tempDirForIP);

    //disconnectFromNetworkShare();
}

void FileOperations::connectToNetworkShare(const QString& ipAddress, const QString& share, const QString& username, const QString& password, const bool closeFlag, const QString source) {
    QString connectCommand = QString("net use \\\\%1\\%2 /user:%3 %4").arg(ipAddress, share, username, password);
    int result = QProcess::execute(connectCommand);
        qDebug() << "Connected to " + ipAddress + " successfully.";
        emit debugInfo("Connected to " + ipAddress + " successfully.");
        if (closeFlag == true) {
            closeDrugProcess(ipAddress, share, username, password);
            copyFilesToTemp(source, QString("\\\\%1\\%2\\DRUG\\Users").arg(ipAddress, share), ipAddress, share);
        } else {
            copyFilesToTemp(source, QString("\\\\%1\\%2\\DRUG\\Users").arg(ipAddress, share), ipAddress, share);
        }
}

void FileOperations::closeDrugProcess(const QString& ipAddress, const QString& share, const QString& username, const QString& password) {
    QString processName = "drug.exe"; // Имя процесса для завершения
    QString command = "taskkill /S " + ipAddress + " /U " + username + " /P " + password + " /IM " + processName + " /F";

    QProcess process;
    process.start("cmd.exe", QStringList() << "/C" << command);
    process.waitForFinished(-1);
}

void FileOperations::copyFilesToTemp(const QString &sourceDirPath, const QString &targetDirPath, const QString& ipAddress, const QString& share) {
    QString tempDirPath = targetDirPath + "\\temp";
    QDir sourceDir(sourceDirPath);
    QDir tempDir(tempDirPath);

    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist: " << sourceDirPath;
        return;
    }

    // Если временная папка уже существует, удаляем её
    if (tempDir.exists()) {
        if (!tempDir.removeRecursively()) {
            qDebug() << "Failed to remove existing temp directory: " << tempDirPath;
            return;
        }
    }

    // Создаем временную папку
    if (!tempDir.mkpath(".")) {
        qDebug() << "Failed to create temp directory: " << tempDirPath;
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
            continue;
        }

        qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
        emit debugInfo("Copied file:" + sourceFilePath + "to" + destinationFilePath);
    }
    copyTempToAllSubdirectories(tempDirPath, targetDirPath, ipAddress, share);
}

void FileOperations::copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath, const QString& ipAddress, const QString& share) {
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
        if (subDirectory == "temp") {
            qDebug() << "Skipping 'temp' directory: " << targetDir.absoluteFilePath(subDirectory);
            emit debugInfo("Skipping 'temp' directory");
            continue;
        }

        QString destinationPath = targetDir.absoluteFilePath(subDirectory);

        QDir destinationDir(destinationPath);
        if (!destinationDir.exists()) {
            qDebug() << "Destination directory does not exist: " << destinationPath;
            //Вставить еррор кейс
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
                    // Вставить еррор кейс
                    continue;
                }
            }

            if (!QFile::copy(sourceFilePath, destinationFilePath)) {
                qDebug() << "Failed to copy file:" << sourceFilePath << "to" << destinationFilePath;
                // Вставить еррор кейс
                continue;
            }

            qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
            emit debugInfo("Copied file:" + sourceFilePath + "to" + destinationFilePath);
        }
    }
    removeTempDirectory(tempDirPath, ipAddress, share);
}

void FileOperations::removeTempDirectory(QString tempDirPath, const QString& ipAddress, const QString& share) {
    QDir tempDir(tempDirPath);

    if (tempDir.exists()) {
        if (!tempDir.removeRecursively()) {
            qDebug() << "Failed to remove temp directory: " << tempDirPath;
            // Добавить еррор кейс
        }
        qDebug() << "Removed temp directory:" << tempDirPath;
        emit debugInfo("Removed temp directory");
    } else {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        // Добавить еррор кейс
    }
    updateVersionFile("C:\\Test\\copy\\source", tempDirPath.remove("\\temp"), ipAddress, share);
}

void FileOperations::updateVersionFile(const QString &sourceDirPath, const QString &destinationDirPath, const QString& ipAddress, const QString& share) {
    QString filePath = destinationDirPath + "/version.txt";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing.";
        return;
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
        file.close();
        return;
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
            }
        } else {
            // Если файла не было в списке, добавляем его
            out << fileName << " - Last modified: " << lastModified.toString() << "\n";
            qDebug() << "Added file information for" << fileName;
        }
    }

    file.close();
    qDebug() << "Finished updating file information.";
    emit debugInfo("Finished updating file information.");
    disconnectFromNetworkShare(ipAddress, share);
    emit copyFinished(ipAddress, share);
}

void FileOperations::disconnectFromNetworkShare(const QString& ipAddress, const QString& share) {
    QString disconnectCommand = QString("net use \\\\%1\\%2 /delete /y").arg(ipAddress, share);
    int result = QProcess::execute(disconnectCommand);
    if (result == 0) {
        qDebug() << "Disconnected from server successfully.";
        emit debugInfo("Disconnected from server successfully.");
    } else {
        qDebug() << "Failed to disconnect from server";
    }
}
