#include "auctooloperations.h"
#include <QString>
#include <QMessageBox>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <memory>
#include <Windows.h>
#include <tlhelp32.h>
#include <QDirIterator>
#include <zlib.h>
#include <fstream>
#include <string>
#include <ctime>
#include <quazip.h>
#include <quazipfile.h>
#include <QDate>
#include <QTextStream>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrent>
#include "mainwindow.h"

AUCToolOperations::AUCToolOperations() {

}


// Update related functions

bool AUCToolOperations::is_running (const std::wstring& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &processEntry)) {
        do {
            if (_wcsicmp(processEntry.szExeFile, processName.c_str()) == 0) {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32Next(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
    return false;
}

void AUCToolOperations::copy_recursively(const QString &sourceDir, const QString &destinationBaseDir) {
    emit operationStatus("Starting the copying process...");

    QDir source(sourceDir);
    QDir destinationBase(destinationBaseDir);

    if (!source.exists()) {
        qDebug() << "Source directory does not exist.";
        return;
    }

    QStringList subDirs = destinationBase.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Confirmation",
                                  "Do you want to copy files to all subdirectories in the destination directory?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        qDebug() << "Copying canceled by user.";
        return;
    }

    QDirIterator it(sourceDir, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        it.next();

        QString sourcePath = it.filePath();
        QString relativePath = source.relativeFilePath(sourcePath);

        for (const QString &subDir : qAsConst(subDirs)) {
            QDir destinationDir = destinationBase.filePath(subDir);
            QString destinationPath = destinationDir.filePath(relativePath);

            if (it.fileInfo().isDir()) {
                QDir(destinationPath).mkpath(".");
            } else if (it.fileInfo().isFile()) {
                QFile sourceFile(sourcePath);

                QString fileName = sourceFile.fileName();

                if (QFile::exists(destinationPath)) {
                    if (!QFile::remove(destinationPath)) {
                        qDebug() << "Error removing existing file: " << destinationPath;
                        emit operationStatus("Error removing existing file: " + fileName);
                        continue;  // Пропускаем текущий файл и идем к следующему
                    }
                    emit operationStatus("Replacing file: " + fileName);
                } else {
                    emit operationStatus("Copying file: " + fileName);
                }

                if (sourceFile.copy(destinationPath)) {
                    qDebug() << "Copying file: " << sourcePath << " to " << destinationPath;
                    emit operationStatus("File copied successfully: " + fileName);
                } else {
                    qDebug() << "Error during copying: " << sourcePath;
                    emit operationStatus("Error copying file: " + fileName);
                }
            }
        }
    }

    emit operationStatus("Copying process completed successfully.");
}

void AUCToolOperations::update(const QString& sourceDir, const QString& destinationDir)
{
    if (is_running(L"DRUG.EXE") || is_running(L"DRUGSYS.EXE") || is_running(L"drug.exe") || is_running(L"drugsys.exe")) {
        std::cout << "Error: DRUG or DRUGSYS processes are running. Cannot proceed with copying." << std::endl;
        QMessageBox drugErr;
        drugErr.setText("Error: DRUG or DRUGSYS processes are running. Cannot proceed with copying.");
        drugErr.exec();
        return;
    } else {
        copy_recursively(sourceDir, destinationDir);
    }

}

void AUCToolOperations::clear(const QString &directoryPath, const QString &ignoreListFilePath) {
    // Проверяем существование файла с игнорируемым списком
    if (!QFile::exists(ignoreListFilePath)) {
        qDebug() << "Error: " << ignoreListFilePath << " not found.";
        return;
    }

    // Проверяем существование директории
    QDir directory(directoryPath);
    if (!directory.exists()) {
        qDebug() << "Error: Directory " << directoryPath << " does not exist.";
        return;
    }

    // Загружаем имена файлов и папок для игнорирования из файла ignore.txt
    QStringList ignoreList;
    QFile ignoreFile(ignoreListFilePath);
    if (ignoreFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&ignoreFile);
        while (!in.atEnd()) {
            ignoreList << in.readLine();
        }
        ignoreFile.close();
    } else {
        qDebug() << "Error: Failed to open " << ignoreListFilePath << ".";
        return;
    }

    // Перебираем файлы и папки в указанной директории
    QStringList entryList = directory.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (const QString &entryName : entryList) {
        // Проверяем, находится ли имя файла или папки в списке игнорируемых
        if (!ignoreList.contains(entryName)) {
            // Этот файл или папка не находится в списке игнорируемых, удаляем его
            QString entryPath = directory.filePath(entryName);
            try {
                if (QFileInfo(entryPath).isDir()) {
                    // Если это директория, удаляем рекурсивно
                    QDir(entryPath).removeRecursively();
                    emit operationStatus("Deleted directory: " + entryName);
                } else {
                    // Если это файл, удаляем его
                    QFile(entryPath).remove();
                    emit operationStatus("Deleted file: " + entryName);
                }
            } catch (const std::exception &e) {
                qDebug() << "Error: Failed to delete " << entryName << ". " << e.what();
                emit operationStatus("Error deleting " + entryName + ": " + e.what());
            }
        }
    }

    // Добавляем сообщение о завершении очистки
    emit operationStatus("Directory clearing process completed.");
}

void AUCToolOperations::addTo7zFromDirectory(const QString &sourceDir, const QString &archiveListFile, const QString &outputDir, QProgressDialog *progressDialog) {
    // Путь к исполняемому файлу 7-Zip (пример для Windows)
    QString sevenZipProgram = QCoreApplication::applicationDirPath() + "/7z/7z.exe";
    emit operationStatus("Starting archivation...");

    QString timestamp = QDateTime::currentDateTime().toString("dd.MM.yyyy-hh.mm");
    QString archiveFileName = timestamp + ".7z";
    QString fullOutputPath = outputDir + "/" + archiveFileName;

    // Создаем временную директорию
    QString tempDirPath = outputDir + "/temp";
    QDir tempDir(tempDirPath);
    if (!tempDir.exists()) {
        tempDir.mkpath(".");
    }

    // Читаем список файлов из файла
    QStringList filesToCopy;
    QFile listFile(archiveListFile);
    if (listFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&listFile);
        while (!in.atEnd()) {
            QString fileName = in.readLine();
            filesToCopy.append(fileName);
        }
        listFile.close();
    } else {
        // Обработка ошибок при открытии файла списка
        emit operationStatus("Error opening archive list file: " + archiveListFile);
        return;
    }

    // Копируем только файлы из списка во временную директорию
    foreach (const QString &fileName, filesToCopy) {
        QString sourceFilePath = sourceDir + "/" + fileName;
        QString tempFilePath = tempDirPath + "/" + fileName;

        if (!QFile::copy(sourceFilePath, tempFilePath)) {
            // Обработка ошибок при копировании файла
            emit operationStatus("Error copying file: " + fileName);
            return;
        }
    }

    // Команда для создания архива 7z из временной директории
    QString command = QString("\"%1\" a \"%2\" %3").arg(sevenZipProgram, fullOutputPath, tempDirPath);

    QProcess process;
    process.start(command);
    process.waitForFinished(-1);

    if (process.exitCode() == 0) {
        // Архивация успешно завершена
        emit operationStatus("7z Archiving completed successfully. Archive created at: " + fullOutputPath + ". Size: " + QString::number(QFileInfo(fullOutputPath).size() / (1024 * 1024)) + " MB");
    } else {
        // Произошла ошибка при архивации
        emit operationStatus("Error during 7z archiving. Exit code: " + QString::number(process.exitCode()));
        qDebug() << "Error output: " << process.readAllStandardError();
    }

    // Удаляем временную директорию после архивации
    tempDir.removeRecursively();
    emit operationStatus("Temporary files were deleted successfully");
}




/* void AUCToolOperations::addToZipFromDirectory(const QString &sourceDir, const QString &archiveListFile, const QString &outputDir, QProgressDialog *progressDialog) {

    emit operationStatus("Starting the archiving process...");

    const int bufferSize = 4 * 1024 * 1024;
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString zipPath = outputDir + "/" + today + ".zip";

    QFile archiveFile(archiveListFile);
    if (!archiveFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open archive list file";
        return;
    }

    QTextStream in(&archiveFile);
    QStringList filesInArchive;
    while (!in.atEnd()) {
        QString line = in.readLine();
        filesInArchive.append(line);
    }
    archiveFile.close();

    QuaZip zip(zipPath);
    if (!zip.open(QuaZip::mdCreate)) {
        qWarning() << "Could not create ZIP archive";
        return;
    }

    QuaZipFile outFile(&zip);

    QDir directory(sourceDir);
    directory.setFilter(QDir::Files | QDir::NoSymLinks);

    int totalFiles = directory.entryList().size();
    int filesProcessed = 0;

    foreach (const QString &file, directory.entryList()) {
        if (filesInArchive.contains(file)) {
            QString filePath = sourceDir + "/" + file;
            QFile fileToAdd(filePath);

            emit operationStatus("Archiving file: " + file);

            if (!fileToAdd.open(QIODevice::ReadOnly)) {
                qWarning() << "Could not open file: " << filePath;
                continue;
            }

            QuaZipNewInfo info(file);
            if (!outFile.open(QIODevice::WriteOnly, info)) {
                qWarning() << "Could not write file to ZIP: " << filePath;
                fileToAdd.close();
                continue;
            }

            // Буферизованная запись файла в архив
            QByteArray buffer;
            while (!fileToAdd.atEnd()) {
                buffer = fileToAdd.read(bufferSize);
                outFile.write(buffer);
            }

            outFile.close();
            fileToAdd.close();

            ++filesProcessed;
            int progress = (filesProcessed * 100) / totalFiles;
            progressDialog->setValue(progress);
            qApp->processEvents(); // Обновляем интерфейс
        }
    }
    emit operationStatus("Archiving completed successfully. Archive created at: " + zipPath + ". Size: " + QString::number(QFileInfo(zipPath).size() / (1024 * 1024)) + " MB");
    zip.close();
} */


