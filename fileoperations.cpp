#include "fileoperations.h"
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QDateTime>

FileOperations::FileOperations(QObject *parent) : QObject(parent) {

}

void FileOperations::connectToNetworkShare(const QString& ipAddress, const QString& share, const QString& username, const QString& password, const bool closeFlag, const QString source) {
    // Проверяем состояние подключения к сетевому ресурсу
    QString checkConnectionCommand = QString("net use \\\\%1\\%2").arg(ipAddress, share);
    QProcess checkProcess;
    checkProcess.start(checkConnectionCommand);
    checkProcess.waitForFinished(-1); // Ждем завершения выполнения процесса

    int checkResult = checkProcess.exitCode(); // Получаем код завершения

    if (checkResult == 0) { // Код 0 означает успешное выполнение команды
        qDebug() << "Already connected to " + ipAddress + ".";
        emit debugInfo(ipAddress + ": already connected.");

        // Выполняем действия, например, копирование файлов
        if (closeFlag) {
            closeDrugProcess(ipAddress, share, username, password);
            copyFilesToTemp(source, QString("\\\\%1\\%2\\DRUG\\Users").arg(ipAddress, share), ipAddress, share);
        } else {
            copyFilesToTemp(source, QString("\\\\%1\\%2\\DRUG\\Users").arg(ipAddress, share), ipAddress, share);
        }
    } else {
        // Если не подключены, выполняем команду подключения
        QString connectCommand = QString("net use \\\\%1\\%2 /user:%3 %4").arg(ipAddress, share, username, password);

        QProcess process;
        process.start(connectCommand);
        process.waitForFinished(-1); // Ждем завершения выполнения процесса

        int result = process.exitCode(); // Получаем код завершения

        if (result == 0) { // Код 0 означает успешное выполнение команды
            qDebug() << "Connected to " + ipAddress + " successfully.";
            emit debugInfo(ipAddress + ": connected successfully.");

            // Выполняем действия после успешного подключения
            if (closeFlag) {
                closeDrugProcess(ipAddress, share, username, password);
                copyFilesToTemp(source, QString("\\\\%1\\%2\\DRUG\\Users").arg(ipAddress, share), ipAddress, share);
            } else {
                copyFilesToTemp(source, QString("\\\\%1\\%2\\DRUG\\Users").arg(ipAddress, share), ipAddress, share);
            }
        } else {
            qDebug() << "Failed to connect to " + ipAddress + ".";
            emit debugInfo(ipAddress + ": failed to connect.");
            emit recieveError(ipAddress, "Connection error", "Can't connect to the server");
            emit copyFinished(ipAddress, 1);
        }
    }
}


void FileOperations::closeDrugProcess(const QString& ipAddress, const QString& share, const QString& username, const QString& password) {
    QString processName = "drug.exe"; // Имя процесса для завершения
    QString command = "taskkill /S " + ipAddress + " /U " + username + " /P " + password + " /IM " + processName + " /F";

    QProcess process;
    process.start("cmd.exe", QStringList() << "/C" << command);
    process.waitForFinished(-1);

    QString resultMessage;

    if (process.exitCode() == 0) {
        resultMessage = ipAddress + ": drug.exe process closed successfully.";
    } else if (process.exitCode() == 128) {
        resultMessage = ipAddress + ": drug.exe process not found.";
    } else {
        resultMessage = ipAddress + ": drug.exe process can't be closed.";
        emit recieveError(ipAddress, "drug.exe error", "Can't close drug.exe");
    }

    emit debugInfo(resultMessage);
}


void FileOperations::copyFilesToTemp(const QString &sourceDirPath, const QString &targetDirPath, const QString& ipAddress, const QString& share) {
    QString tempDirPath = targetDirPath + "\\temp";
    QDir sourceDir(sourceDirPath);
    QDir tempDir(tempDirPath);
    bool noncriticalError = false;

    if (!sourceDir.exists()) {
        qDebug() << "Source directory does not exist: " << sourceDirPath;
        emit debugInfo(ipAddress + ": source directory does not exist.");
        emit recieveError(ipAddress, "Source error", "Can't find source directory");
        emit copyFinished(ipAddress, 1);
        disconnectFromNetworkShare(ipAddress, share);
        return;
    }

    // Если временная папка уже существует, удаляем её
    if (tempDir.exists()) {
        if (!tempDir.removeRecursively()) {
            qDebug() << "Failed to remove existing temp directory: " << tempDirPath;
            emit debugInfo(ipAddress + ": failed to remove existing temp directory.");
            emit recieveError(ipAddress, "Temp error", "Can't remove temp directory");
            emit copyFinished(ipAddress, 1);
            disconnectFromNetworkShare(ipAddress, share);
            return;
        }
    }

    // Создаем временную папку
    if (!tempDir.mkpath(".")) {
        qDebug() << "Failed to create temp directory: " << tempDirPath;
        emit debugInfo(ipAddress + ": failed to create temp directory.");
        emit recieveError(ipAddress, "Temp error", "Can't create temp directory");
        emit copyFinished(ipAddress, 1);
        disconnectFromNetworkShare(ipAddress, share);
        return;
    }

    // Получаем список файлов из исходной директории
    QStringList files = sourceDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    foreach (const QString &file, files) {
        QString sourceFilePath = sourceDirPath + "/" + file;
        QString destinationFilePath = tempDirPath + "/" + file;

        // Копируем файлы из источника во временную папку
        if (!QFile::copy(sourceFilePath, destinationFilePath)) {
            qDebug() << "Failed to copy file: " << sourceFilePath << " to " << destinationFilePath;
            emit debugInfo(ipAddress + ": failed to copy file " + sourceFilePath + " to " + destinationFilePath);
            QFileInfo fileInfo(sourceFilePath);
            QString fileName = fileInfo.fileName();
            emit recieveError(ipAddress, "Copy error", "Can't copy file " + fileName + " to temp."); // ДОПОЛНИТЬ НАЗВАНИЯМИ ФАЙЛОВ
            emit copyFinished(ipAddress, 2);
            noncriticalError = true;
            continue;
        }

        qDebug() << "Copied file: " << sourceFilePath << " to " << destinationFilePath;
        emit debugInfo(ipAddress + ": copied file " + sourceFilePath + " to " + destinationFilePath + " successfully.");
    }
    copyTempToAllSubdirectories(sourceDirPath, tempDirPath, targetDirPath, ipAddress, share, noncriticalError);
}

void FileOperations::copyTempToAllSubdirectories(const QString &sourceDirPath, const QString &tempDirPath, const QString &targetDirPath, const QString& ipAddress, const QString& share, bool noncriticalError) {
    QDir tempDir(tempDirPath);
    QDir targetDir(targetDirPath);

    if (!tempDir.exists()) {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        emit debugInfo(ipAddress + ": temp directory does not exist.");
        emit recieveError(ipAddress, "Temp error", "Can't find temp directory");
        emit copyFinished(ipAddress, 1);
        return;
    }

    if (!targetDir.exists()) {
        qDebug() << "Target directory does not exist: " << targetDirPath;
        emit debugInfo(ipAddress + ": target directory does not exist.");
        emit recieveError(ipAddress, "Target error", "Can't find target folder");
        emit copyFinished(ipAddress, 1);
        return;
    }

    QStringList subDirectories = targetDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const QString &subDirectory, subDirectories) {
        if (subDirectory == "temp") {
            qDebug() << "Skipping 'temp' directory: " << targetDir.absoluteFilePath(subDirectory);
            emit debugInfo(ipAddress + ": skipping 'temp' directory.");
            continue;
        }

        QString destinationPath = targetDir.absoluteFilePath(subDirectory);

        QDir destinationDir(destinationPath);
        if (!destinationDir.exists()) {
            qDebug() << "Destination directory does not exist: " << destinationPath;
            emit debugInfo(ipAddress + ": destination directory does not exist.");
            emit recieveError(ipAddress, "Target error", "Can't find subdirectory for copy");
            noncriticalError = true;
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
                    emit debugInfo(ipAddress + ": failed to remove existing file " + destinationFilePath);
                    QFileInfo fileInfo(destinationFilePath);
                    QString fileName = fileInfo.fileName();
                    emit recieveError(ipAddress, "Copy error", "Can't remove existing file " + fileName);
                    noncriticalError = true;
                    continue;
                }
            }

            if (!QFile::copy(sourceFilePath, destinationFilePath)) {
                qDebug() << "Failed to copy file:" << sourceFilePath << "to" << destinationFilePath;
                emit debugInfo(ipAddress + ": failed to copy file " + sourceFilePath + " to " + destinationFilePath);
                noncriticalError = true;
                QFileInfo fileInfo(sourceFilePath);
                QString fileName = fileInfo.fileName();
                emit recieveError(ipAddress, "Copy error", "Can't copy file " + fileName);
                continue;
            }

            qDebug() << "Copied file:" << sourceFilePath << "to" << destinationFilePath;
            emit debugInfo(ipAddress + ": copied file " + sourceFilePath + " to " + destinationFilePath + " successfully.");
        }
    }
    removeTempDirectory(sourceDirPath, tempDirPath, ipAddress, share, noncriticalError);
}

void FileOperations::removeTempDirectory(const QString &sourceDirPath, QString tempDirPath, const QString& ipAddress, const QString& share, bool noncriticalError) {
    QDir tempDir(tempDirPath);

    if (tempDir.exists()) {
        if (!tempDir.removeRecursively()) {
            qDebug() << "Failed to remove temp directory: " << tempDirPath;
            emit debugInfo(ipAddress + ": failed to removed temp directory.");
            emit recieveError(ipAddress, "Temp error", "Can't remove temp directory");
            noncriticalError = true;
            // Добавить еррор кейс
        }
        qDebug() << "Removed temp directory:" << tempDirPath;
        emit debugInfo(ipAddress + ": removed temp directory successfully.");
    } else {
        qDebug() << "Temp directory does not exist: " << tempDirPath;
        emit debugInfo(ipAddress + ": temp directory does not exist.");
        emit recieveError(ipAddress, "Temp error", "Can't find temp directory after copy");
        noncriticalError = true;
        // Добавить еррор кейс
    }
    updateVersionFile(sourceDirPath, tempDirPath.remove("\\temp"), ipAddress, share, noncriticalError);
}

void FileOperations::updateVersionFile(const QString &sourceDirPath, const QString &destinationDirPath, const QString& ipAddress, const QString& share, bool noncriticalError) {
    QString filePath = destinationDirPath + "/version.txt";
    QFile file(filePath);

    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing.";
        emit debugInfo(ipAddress + ": failed to open version file for writing.");
        emit recieveError(ipAddress, "Version control error", "Can't open file for writing new version");
        emit copyFinished(ipAddress, 2);
        disconnectFromNetworkShare(ipAddress, share);
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
        emit debugInfo(ipAddress + ": source directory " + sourceDirPath + " does not exist.");
        emit recieveError(ipAddress, "Version control error", "Can't locate source directory for updating version file");
        file.close();
        emit copyFinished(ipAddress, 2);
        disconnectFromNetworkShare(ipAddress, share);
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
                emit debugInfo(ipAddress + ": updated file information for " + fileName + " successfully.");
            }
        } else {
            // Если файла не было в списке, добавляем его
            out << fileName << " - Last modified: " << lastModified.toString() << "\n";
            qDebug() << "Added file information for" << fileName;
            emit debugInfo(ipAddress + ": added file information for " + fileName + " successfully.");
        }
    }

    file.close();
    qDebug() << "Finished updating file information.";
    emit debugInfo(ipAddress + ": finished updating file information.");
    if (noncriticalError == false) {
        emit copyFinished(ipAddress, 0);
    } else {
        emit copyFinished(ipAddress, 2);
    }

    disconnectFromNetworkShare(ipAddress, share);
}

void FileOperations::disconnectFromNetworkShare(const QString& ipAddress, const QString& share) {
    QString disconnectCommand = QString("net use \\\\%1\\%2 /delete /y").arg(ipAddress, share);
    int result = QProcess::execute(disconnectCommand);
    if (result == 0) {
        qDebug() << "Disconnected from server successfully.";
        emit debugInfo(ipAddress + ": disconnected from server successfully.");
    } else {
        qDebug() << "Failed to disconnect from server";
        emit debugInfo(ipAddress + ": failed to disconnect from server.");
    }
}
