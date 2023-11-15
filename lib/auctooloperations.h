#ifndef AUCTOOLOPERATIONS_H
#define AUCTOOLOPERATIONS_H

#include <QString>
#include <QProgressDialog>
#include <QTextBrowser>
#include "qobject.h"
#include "qobjectdefs.h"
#include <QCoreApplication>

class AUCToolOperations : public QObject {
    Q_OBJECT
public:
    AUCToolOperations();
signals:
    void operationStatus(const QString& status);

public:
    void update(const QString& sourceDir, const QString& destinationDir);
    bool is_running (const std::wstring& processNam);
    void copy_recursively(const QString &sourceDir, const QString &destinationBaseDir);

    void addTo7zFromDirectory(const QString &sourceDir, const QString &archiveListFile, const QString &outputDir, QProgressDialog *progressDialog);

    // void addToZipFromDirectory(const QString &sourceDir, const QString &archiveListFile, const QString &outputDir, QProgressDialog *progressDialog);
    void clear(const QString& directoryPath, const QString& ignoreListFilePath);

private:
    QString appDirPath = QCoreApplication::applicationDirPath();
    QString settingsPath = appDirPath + "/cfg/settings.ini";
};

#endif // AUCTOOLOPERATIONS_H
