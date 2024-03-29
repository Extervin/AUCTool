#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <QObject>

class FileOperations : public QObject {
    Q_OBJECT
public:
    FileOperations(QObject *parent = nullptr);

    void closeAndUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, QVector<int> rowID);

private:
    void copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath);
    void copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath);
    void removeTempDirectory(const QString &tempDirPath);
    bool updateVersionFile(const QString &sourceDirPath, const QString &targetDirPath);
    void connectToNetworkShare(const QString& server, const QString& share, const QString& username, const QString& password);
    void disconnectFromNetworkShare();
};

#endif // FILEOPERATIONS_H
