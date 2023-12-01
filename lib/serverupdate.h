// serverupdate.h

#ifndef SERVERUPDATE_H
#define SERVERUPDATE_H

#include <QCoreApplication>
#include <QMainWindow>
#include <QTableView>
#include <QKeyEvent>
#include <map>
#include <quazip.h>

namespace Ui {
class ServerUpdate;
}

class ServerUpdate : public QWidget
{
    Q_OBJECT

signals:
    void closed();

    void destroyed(QObject *obj = nullptr);


public:
    explicit ServerUpdate(QWidget *parent = nullptr);
    ~ServerUpdate();

public slots:
    void saveAndClose();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_tableWidget_cellChanged(int row, int column);

    void on_addButton_clicked();

    void closeEvent(QCloseEvent *event);

    void on_serverUpdateButton_clicked();

    void on_refreshButton_clicked();

    void on_toolButton_clicked();

private:
    void saveToFile(const QString &filename);

    void loadFromFile(const QString &filePath);

    QStringList askCredentials();

    void countFolders();

    void checkServerAvailability();

    void checkForProcess();

    void closeDrugProcesses(const QString& ipAddress, const QString& username, const QString& password);

    void updateInBackground(const QString& settingsPath);

    void createUpdateArchive(const QString &sourceDir);

    void addFilesToArchive(const QString &sourceDir, QuaZip &zip, const QString &dirPath = "");

    void killtask();

    void debugmessage(const QString& message);

    void copyTempToAllSubdirectories(const QString &tempDirPath, const QString &targetDirPath);

    void copyFilesToTemp(const QString &sourceDirPath, const QString &tempDirPath);

    void removeTempDirectory(const QString &tempDirPath);

    void updateVersionFile(const QString &sourceDirPath, const QString &targetDirPath);

    void checkVersionMatch(const QString &updateSourcePath);

    void startUpdate();

private:
    Ui::ServerUpdate *ui;
    std::map<QString, QStringList> tableData;

    QString appDirPath = QCoreApplication::applicationDirPath();
    QString ipStoragePath = appDirPath + "/cfg/iplist.txt";
    QString settingsPath = appDirPath + "/cfg/settings.ini";
    QStringList credentials;
};

#endif // SERVERUPDATE_H
