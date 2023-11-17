#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCoreApplication>
#include "auctooloperations.h"
#include "settingsdialog.h"
#include "ui_mainwindow.h"
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void cancelArchiving();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void updateDebugConsole(const QString& status) {
        QMetaObject::invokeMethod(ui->debugConsole, "append", Qt::QueuedConnection, Q_ARG(QString, status));
    }

private slots:

    void on_clearButton_clicked();

    void on_updateButton_clicked();

    void on_archiveButton_clicked();

    void on_actionSettings_triggered();

    void on_actionExit_triggered();

    void loadTextFile(const QString &filePath, QTextBrowser *textBrowser);

    void updateinfo();

private:
    Ui::MainWindow *ui;
    AUCToolOperations tool;
    //locating where app is placed
    QString appDirPath = QCoreApplication::applicationDirPath();
    QString settingsPath = appDirPath + "/cfg/settings.ini";
    SettingsDialog *settingsDialog;
    //variables for deleting
    QString ignoreListPath = appDirPath + "/cfg/ignore.txt";
    QString archiveListPath = appDirPath + "/cfg/archive.txt";
};
#endif // MAINWINDOW_H
