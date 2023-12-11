#ifndef SERVERAUCOPERATIONS_H
#define SERVERAUCOPERATIONS_H

#include <QObject>
#include <QCoreApplication>
#include <QTableWidget>

class ServerAUCOperations : public QObject
{
    Q_OBJECT
public:

    ServerAUCOperations();

    void updateInBackground(const QString& sourcePath, const QStringList& ipAddresses, const QString& username, const QString& password);

    void countFolders(QTableWidget* tableWidget);

    void checkServerAvailability(QTableWidget* tableWidget);

    void checkForProcess(const QString& username, const QString& password, QTableWidget* tableWidget);

    void checkVersionMatch(const QString& updateSourcePath, QTableWidget* tableWidget);

    void closeDrugProcesses(const QString& ipAddress, const QString& username, const QString& password);

signals:
    void setdebugmessage(const QString& message);

private:

    void copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath);

    void copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath);

    void removeTempDirectory(const QString &tempDirPath);

    void updateVersionFile(const QString &sourceDirPath, const QString &targetDirPath);

    void startUpdate();

    void connectToNetworkShare(const QString& server, const QString& share, const QString& username, const QString& password);

    void disconnectFromNetworkShare();

public:

    QString settingsPath = appDirPath + "/cfg/settings.ini";
    QStringList credentials;

private:

    QString appDirPath = QCoreApplication::applicationDirPath();
    QString ipStoragePath = appDirPath + "/cfg/iplist.txt";


};



#endif // SERVERAUCOPERATIONS_H
