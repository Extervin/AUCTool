#ifndef SERVERAUCOPERATIONS_H
#define SERVERAUCOPERATIONS_H

#include <QObject>
#include <QCoreApplication>
#include <QTableWidget>
#include <QThread>

class ServerAUCOperations : public QObject
{
    Q_OBJECT
public:
    explicit ServerAUCOperations(QObject *parent = nullptr);

    void updateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, std::vector<int> rowID);

    int countFolders();

    int checkServerAvailability(QString ipAddress);

    int checkForProcess(const QString& username, const QString& password, QString ipAddress);

    int checkVersionMatch(const QString& updateSourcePath);

    void closeDrugProcesses(const QString& ipAddress, const QString& username, const QString& password);

    std::vector<int> refreshInBackground(const QString& sourcePath, const QString &ipAddresses, const QString& username, const QString& password);

    void closeAndUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, std::vector<int> rowID);

public slots:

    void performUpdateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password, std::vector<int> rowID) {

        moveToThread(QThread::currentThread());

        updateInBackground(sourcePath, ipAddresses, username, password, rowID);

        emit finished();
    }

signals:

    void sendDebugMessage(const QString& message);

    void finished();

    void sendTableValue(int row, int col, QString value, const QColor &color);

private:

    void copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath);

    void copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath);

    void removeTempDirectory(const QString &tempDirPath);

    bool updateVersionFile(const QString &sourceDirPath, const QString &targetDirPath);

    void startUpdate();

    void connectToNetworkShare(const QString& server, const QString& share, const QString& username, const QString& password);

    void disconnectFromNetworkShare();

public:
    QString appDirPath = QCoreApplication::applicationDirPath();
    QString ipStoragePath = appDirPath + "/cfg/iplist.txt";
    QString settingsPath = appDirPath + "/cfg/settings.ini";
    QStringList credentials;

};



#endif // SERVERAUCOPERATIONS_H
