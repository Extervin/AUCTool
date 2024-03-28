#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <QObject>
#include <QCoreApplication>
#include <QTableWidget>
#include <QThread>

class FileOperations
{
public:
    FileOperations();
    void closeDrugProcesses(const QString& ipAddress, const QString& username, const QString& password);
    void closeAndUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, std::vector<int> rowID);

    QString appDirPath = QCoreApplication::applicationDirPath();
    QString ipStoragePath = appDirPath + "/cfg/iplist.txt";
    QString settingsPath = appDirPath + "/cfg/settings.ini";
    QStringList credentials;
public slots:
    void performUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, std::vector<int> rowID) {

        moveToThread(QThread::currentThread());

        updateInBackground(sourcePath, ipAddresses, username, password, rowID);

        emit finished();
    }
signals:
    void finished();
    void sendTableValue(int row, int col, QString value, const QColor &color);

private:
    bool copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath);
    void copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath);
    void removeTempDirectory(const QString &tempDirPath);
    bool updateVersionFile(const QString &sourceDirPath, const QString &targetDirPath);
    void startUpdate();
    void connectToNetworkShare(const QString& server, const QString& share, const QString& username, const QString& password);
    void disconnectFromNetworkShare();
};

#endif // FILEOPERATIONS_H