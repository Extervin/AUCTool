#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <QObject>

class FileOperations : public QObject {
    Q_OBJECT

signals:
    void copyFinished(const QString &ipAddress, const int &finishCode);

    void debugInfo(const QString& info);

    void recieveError(const QString& ipAddress, const QString& errorType, const QString& errorMessage);

public:
    FileOperations(QObject *parent = nullptr);

    void closeAndUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, QVector<int> rowID);

    void connectToNetworkShare(const QString& server, const QString& share, const QString& username, const QString& password, const bool closeFlag, const QString source, const QString target, const bool straightCopyFlag);
    void disconnectFromNetworkShare(const QString &ipAddress, const QString &share);

private:
    void copyTempToAllSubdirectories(const QString &sourceDirPath, const QString &tempDirPath, const QString &targetDirPath, const QString &ipAddress, const QString &share, bool noncriticalError);
    void copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath, const QString &ipAddress, const QString &share);
    void removeTempDirectory(const QString &sourceDirPath, QString tempDirPath, const QString &ipAddress, const QString &share, bool noncriticalError);
    void updateVersionFile(const QString &sourceDirPath, const QString &targetDirPath, const QString &ipAddress, const QString &share, bool noncriticalError);
    void closeDrugProcess(const QString& ipAddress, const QString& share, const QString& username, const QString& password);
    bool copyDirectory(const QString &sourceDirPath, const QString &destinationDirPath, const QString &ipAddress);
    bool copyDirectoryContents(const QString &sourceDirPath, const QString &destinationDirPath, const QString &ipAddress, const QString &share);
};

#endif // FILEOPERATIONS_H
