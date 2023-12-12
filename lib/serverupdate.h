// serverupdate.h

#ifndef SERVERUPDATE_H
#define SERVERUPDATE_H

#include <QCoreApplication>
#include <QMainWindow>
#include "ui_serverupdate.h"
#include <QKeyEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QStringList>
#include <map>
#include "serveraucoperations.h"


namespace Ui {
class ServerUpdate;
}

class ServerUpdate : public QWidget
{
    Q_OBJECT

public:
    explicit ServerUpdate(QWidget *parent = nullptr);
    ~ServerUpdate();

public slots:
    void receiveDebugMessage(const QString& message);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_tableWidget_cellChanged(int row, int column);

    void on_addButton_clicked();

    void on_serverUpdateButton_clicked();

    void on_refreshButton_clicked();

    void on_toolButton_clicked();

    void on_loginButton_clicked();

    void on_chooseAll_stateChanged(int arg1);

private:
    void saveToFile(const QString &filename);

    void loadFromFile(const QString &filePath);

    QStringList askCredentials();

    void startserverupdate();

    void adddebugmessage(const QString& message);

    void refresh();

private:
    Ui::ServerUpdate *ui;
    std::map<QString, QStringList> tableData;

    QString appDirPath = QCoreApplication::applicationDirPath();
    QString ipStoragePath = appDirPath + "/cfg/iplist.txt";
    QString settingsPath = appDirPath + "/cfg/settings.ini";
    QStringList credentials;
    ServerAUCOperations *operation;
};

#endif // SERVERUPDATE_H
