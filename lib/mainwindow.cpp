#include "auctooloperations.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serverupdate.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QApplication>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    settingsDialog = new SettingsDialog(this);

    connect(&tool, &AUCToolOperations::operationStatus, this, &MainWindow::updateDebugConsole);

    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::on_actionSettings_triggered);

    connect(settingsDialog, &SettingsDialog::settingsChanged, this, &MainWindow::updateinfo);

    updateinfo();

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateinfo()
{
    loadTextFile(archiveListPath, ui->archiveTXTMain);
    loadTextFile(ignoreListPath, ui->ignoreTXTMain);
}

void MainWindow::loadTextFile(const QString &filePath, QTextBrowser *textBrowser)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        textBrowser->setPlainText(in.readAll());
        file.close();
    }
    else
    {
        qDebug() << "Unable to open file: " << filePath;
    }
}

void MainWindow::on_clearButton_clicked()
{
    QSettings settings(settingsPath, QSettings::IniFormat);

    QString clearPath = settings.value("cleaningPath").toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Confirmation",
                                  "Do you want to start archivation before clearing?",
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Close);
    if (reply == QMessageBox::Yes) {
        on_archiveButton_clicked();
        tool.clear(clearPath, ignoreListPath);
    } else if (reply == QMessageBox::No) {
       tool.clear(clearPath, ignoreListPath);
    } else if (QMessageBox::Close) {
       return;
    }
}


void MainWindow::on_updateButton_clicked()
{
    QSettings settings(settingsPath, QSettings::IniFormat);

    QString sourceDir = settings.value("updateSourcePath").toString();
    QString destinationDir = settings.value("updateTargetPath").toString();

    tool.update(sourceDir, destinationDir);
}

void MainWindow::on_archiveButton_clicked()
{

    QSettings settings(settingsPath, QSettings::IniFormat);

    QString sourceDirectory = settings.value("archivatingPath").toString();
    QString archiveListFile = appDirPath + "/cfg/archive.txt";
    QString outputDirectory = settings.value("archiveSavingPath").toString();

    updateDebugConsole("Archiving from " + sourceDirectory);
    updateDebugConsole(archiveListFile + " found");
    updateDebugConsole("Trying to save archive in " + outputDirectory);

    QProgressDialog progressDialog("Archiving...", "Cancel", 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setAutoClose(true);
    progressDialog.setAutoReset(true);

    connect(&progressDialog, &QProgressDialog::canceled, [&]() {
        emit cancelArchiving();
    });

    QEventLoop loop;
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);

    connect(watcher, &QFutureWatcher<void>::finished, [&]() {
        progressDialog.hide();
        ui->clearButton->setEnabled(true);
        watcher->deleteLater();
        loop.quit();
    });

    connect(watcher, &QFutureWatcher<void>::progressValueChanged, [&](int progress) {
        progressDialog.setValue(progress);
    });

    progressDialog.show();

    QFuture<void> future = QtConcurrent::run([&, sourceDirectory, archiveListFile, outputDirectory, &progressDialog]() {
        connect(this, &MainWindow::cancelArchiving, &tool, &AUCToolOperations::cancelArchivingOperation);

        tool.addTo7zFromDirectory(sourceDirectory, archiveListFile, outputDirectory, &progressDialog);
    });

    watcher->setFuture(future);

    loop.exec();
}

void MainWindow::on_actionSettings_triggered()
{
    settingsDialog->show();
}




void MainWindow::on_actionExit_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionServer_Update_triggered()
{
    ServerUpdate *serverUpdateWidget = new ServerUpdate(this);

    QMainWindow *serverUpdateWindow = new QMainWindow(this);
    serverUpdateWindow->setCentralWidget(serverUpdateWidget);

    serverUpdateWidget->setAttribute(Qt::WA_DeleteOnClose);

    serverUpdateWindow->resize(791, 460);
    serverUpdateWindow->setWindowTitle("Server Manager");

    connect(serverUpdateWidget, &ServerUpdate::destroyed, this, [serverUpdateWindow]() {
        serverUpdateWindow->deleteLater();
    });

    serverUpdateWindow->show();
}
