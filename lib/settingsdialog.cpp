#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTextEdit>
#include <QListView>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connect(ui->archivatingPath, &QLineEdit::textChanged, this, [=]() { updateApplyButton(ui->archivatingPath, "archivatingPath"); });
    connect(ui->archiveSavingPath, &QLineEdit::textChanged, this, [=]() { updateApplyButton(ui->archiveSavingPath, "archiveSavingPath"); });
    connect(ui->updateSourcePath, &QLineEdit::textChanged, this, [=]() { updateApplyButton(ui->updateSourcePath, "updateSourcePath"); });
    connect(ui->updateTargetPath, &QLineEdit::textChanged, this, [=]() { updateApplyButton(ui->updateTargetPath, "updateTargetPath"); });
    connect(ui->cleaningPath, &QLineEdit::textChanged, this, [=]() { updateApplyButton(ui->cleaningPath, "cleaningPath"); });
    loadSettings();
    QString filePath = appDirPath + "/cfg/archive.txt";
    QString ignoredPath = appDirPath + "/cfg/ignore.txt";
    loadArchiveFile(filePath);
    loadIgnoredFile(ignoredPath);
    ui->applyButton->setEnabled(false);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::loadArchiveFile(const QString &filePath)
{
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString fileContents = in.readAll();
        file.close();

        ui->archiveTXT->setText(fileContents);
    }

}

void SettingsDialog::loadSettings() {
    QSettings settings(settingsPath, QSettings::IniFormat);

    ui->archivatingPath->setText(settings.value("archivatingPath", "").toString());
    ui->archiveSavingPath->setText(settings.value("archiveSavingPath", "").toString());
    ui->updateSourcePath->setText(settings.value("updateSourcePath", "").toString());
    ui->updateTargetPath->setText(settings.value("updateTargetPath", "").toString());
    ui->cleaningPath->setText(settings.value("cleaningPath", "").toString());
    ui->serverUpdateTargetPath->setText(settings.value("serverUpdatePath", "").toString());
}

void SettingsDialog::applySetting(QLineEdit* lineEdit, const QString& settingsKey) {
    QSettings settings(settingsPath, QSettings::IniFormat);

    QString selectedPath = lineEdit->text();
    QString existingPath = settings.value(settingsKey).toString();

    if (selectedPath != existingPath) {
        settings.setValue(settingsKey, selectedPath);
    }
}

void SettingsDialog::updateApplyButton(QLineEdit* lineEdit, const QString& settingsKey) {
    QSettings settings(settingsPath, QSettings::IniFormat);

    QString selectedPath = lineEdit->text();
    QString existingPath = settings.value(settingsKey).toString();

    ui->applyButton->setEnabled(selectedPath != existingPath);
}

void SettingsDialog::on_chooseSourceButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, "Select Directory", QDir::homePath());
        if (!selectedDirectory.isEmpty()) {
            ui->updateSourcePath->setText(selectedDirectory);
        }
}


void SettingsDialog::on_chooseTargetButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, "Select Directory", QDir::homePath());

    if (!selectedDirectory.isEmpty()) {
        ui->updateTargetPath->setText(selectedDirectory);
    }
}


void SettingsDialog::on_addArchivatedFileButton_clicked()
{
    QString customPath = ui->archivatingPath->text();

    if (!customPath.isEmpty()) {
        QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Files to Archive", customPath);

        for (const QString &fileName : fileNames)
        {
            QFileInfo fileInfo(fileName);
            QString baseName = fileInfo.fileName();

            ui->archiveTXT->insertPlainText(baseName + "\n");
        }
    } else {
        QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Files to Archive", QDir::homePath());

        for (const QString &fileName : fileNames)
        {
            QFileInfo fileInfo(fileName);
            QString baseName = fileInfo.fileName();

            ui->archiveTXT->insertPlainText(baseName + "\n");
        }
    }
}


void SettingsDialog::on_editArchiveCheckBox_stateChanged(int arg1)
{
    if (ui->archiveTXT) {
        ui->archiveTXT->setEnabled(arg1 == Qt::Checked);
    } else {
        qDebug() << "Error: clearButton is not set.";
    }
}


void SettingsDialog::on_applyButton_clicked()
{
    applySetting(ui->archivatingPath, "archivatingPath");
    applySetting(ui->archiveSavingPath, "archiveSavingPath");
    applySetting(ui->updateSourcePath, "updateSourcePath");
    applySetting(ui->updateTargetPath, "updateTargetPath");
    applySetting(ui->cleaningPath, "cleaningPath");
    applySetting(ui->serverUpdateTargetPath, "serverUpdatePath");
    saveArchiveToFile();
    saveIgnoredToFile();
    QMessageBox::information(this, "Settings Applied", "Settings applied successfully!");
    ui->applyButton->setEnabled(false);
    emit settingsChanged();
}


void SettingsDialog::on_closeButton_clicked()
{
    close();
    emit settingsChanged();
}


void SettingsDialog::on_okButton_clicked()
{
    if (ui->applyButton->isEnabled()){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Unapplied Changes",
                                      "You have unapplied changes, refusing will roll them back. Do you want to apply them now?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            on_applyButton_clicked();
            on_closeButton_clicked();
        } else if (reply == QMessageBox::No) {
            QString filePath = appDirPath + "/cfg/archive.txt";
            QString ignoredPath = appDirPath + "/cfg/ignore.txt";
            loadArchiveFile(filePath);
            loadIgnoredFile(ignoredPath);
            loadSettings();
            on_closeButton_clicked();
        }
    } else {
        on_closeButton_clicked();
    }
    emit settingsChanged();
}


void SettingsDialog::on_chooseCleaningPathButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, "Select Directory", QDir::homePath());

        if (!selectedDirectory.isEmpty()) {
            ui->cleaningPath->setText(selectedDirectory);
        }
}


void SettingsDialog::on_addIgnoredFileButton_clicked()
{
        QString customPath = ui->cleaningPath->text();

        if (!customPath.isEmpty()) {
            QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Ignored Files", customPath);

            for (const QString &fileName : fileNames)
            {
            QFileInfo fileInfo(fileName);
            QString baseName = fileInfo.fileName();

            ui->ignoreTXT->insertPlainText(baseName + "\n");
            }
        } else {
            QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Ignored Files", QDir::homePath());

            for (const QString &fileName : fileNames)
            {
            QFileInfo fileInfo(fileName);
            QString baseName = fileInfo.fileName();

            ui->ignoreTXT->insertPlainText(baseName + "\n");
            }
        }
}


void SettingsDialog::on_editIgnoredCheckBox_stateChanged(int arg1)
{
        if (ui->ignoreTXT) {
            ui->ignoreTXT->setEnabled(arg1 == Qt::Checked);
        } else {
            qDebug() << "Error: clearButton is not set.";
        }
}


void SettingsDialog::on_chooseArchiveSaveingPathButton_clicked() //grammar mistake
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, "Select Directory", QDir::homePath());

        if (!selectedDirectory.isEmpty()) {
            ui->archiveSavingPath->setText(selectedDirectory);
        }
}


void SettingsDialog::on_chooseArchivatedFolderPathButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, "Select Directory", QDir::homePath());

    if (!selectedDirectory.isEmpty()) {
        ui->archivatingPath->setText(selectedDirectory);
    }
}


void SettingsDialog::on_archiveTXT_textChanged()
{
    ui->applyButton->setEnabled(true);
}

void SettingsDialog::saveArchiveToFile() {
    QString filePath = appDirPath + "/cfg/archive.txt";

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << ui->archiveTXT->toPlainText();
        file.close();
    }
}

void SettingsDialog::saveIgnoredToFile() {
    QString ignoreFilePath = appDirPath + "/cfg/ignore.txt";

    QFile file(ignoreFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << ui->ignoreTXT->toPlainText();
        file.close();
    } else {
        QMessageBox::warning(this, "Error", "Unable to save ignore.txt");
    }
}

void SettingsDialog::loadIgnoredFile(const QString &filePath) {
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString fileContents = in.readAll();
        file.close();

        ui->ignoreTXT->setText(fileContents);
    }
}

void SettingsDialog::on_ignoreTXT_textChanged()
{
    ui->applyButton->setEnabled(true);
}


void SettingsDialog::on_addIgnoredFolderButton_clicked()
{
    QString customPath = ui->cleaningPath->text();

    if (!customPath.isEmpty()) {
        QString directory = QFileDialog::getExistingDirectory(this, "Select Ignored Directories", customPath, QFileDialog::ShowDirsOnly);

        if (!directory.isEmpty())
        {
            QDir dir(directory);
            QString folderName = dir.dirName();
            ui->ignoreTXT->insertPlainText(folderName + "\n");
        }
    } else {
        QString directory = QFileDialog::getExistingDirectory(this, "Select Ignored Directories", QDir::homePath(), QFileDialog::ShowDirsOnly);

        if (!directory.isEmpty())
        {
            QDir dir(directory);
            QString folderName = dir.dirName();
            ui->ignoreTXT->insertPlainText(folderName + "\n");
        }
    }
}



void SettingsDialog::on_serverUpdateTargetPath_textChanged(const QString &arg1)
{
    ui->applyButton->setEnabled(true);
}

void SettingsDialog::on_checkServerTargetPath_clicked()
{
    QSettings settings(settingsPath, QSettings::IniFormat);
    QString currentPath = "\\\\0.0.0.0\\" + settings.value("serverUpdatePath", "").toString();
    QString message = tr("Your current path is %1\nExample: \\\\0.0.0.0\\d$\\Test\\targetFolder").arg(currentPath);
    QMessageBox::information(this, tr("Path example"), message);
}


