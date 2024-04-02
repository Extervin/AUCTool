#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <QObject>

class FileOperations : public QObject {
    Q_OBJECT

signals:
    void copyFinished(const QString &ipAddress, const QString &share);

    void debugInfo(const QString& info);

public:
    FileOperations(QObject *parent = nullptr);

    void closeAndUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, QVector<int> rowID);

    void connectToNetworkShare(const QString& server, const QString& share, const QString& username, const QString& password, const bool closeFlag, const QString source);
    void disconnectFromNetworkShare(const QString &ipAddress, const QString &share);

private:
    void copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath, const QString &ipAddress, const QString &share);
    void copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath, const QString &ipAddress, const QString &share);
    void removeTempDirectory(QString tempDirPath, const QString &ipAddress, const QString &share);
    void updateVersionFile(const QString &sourceDirPath, const QString &targetDirPath, const QString &ipAddress, const QString &share);
    void closeDrugProcess(const QString& ipAddress, const QString& share, const QString& username, const QString& password);

};

#endif // FILEOPERATIONS_H
