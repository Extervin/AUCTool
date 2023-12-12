#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QString>
#include <QLineEdit>
#include <QCoreApplication>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
signals:
    void settingsChanged();

private slots:
    void on_chooseSourceButton_clicked();

    void on_chooseTargetButton_clicked();

    void on_addArchivatedFileButton_clicked();

    void on_editArchiveCheckBox_stateChanged(int arg1);

    void on_applyButton_clicked();

    void on_closeButton_clicked();

    void on_okButton_clicked();

    void on_chooseCleaningPathButton_clicked();

    void on_editIgnoredCheckBox_stateChanged(int arg1);

    void on_chooseArchiveSaveingPathButton_clicked();

    void on_chooseArchivatedFolderPathButton_clicked();

    void applySetting(QLineEdit* lineEdit, const QString& settingsKey);

    void updateApplyButton(QLineEdit* lineEdit, const QString& settingsKey);

    void loadSettings();

    void loadArchiveFile(const QString &filePath);

    void saveArchiveToFile();

    void on_archiveTXT_textChanged();

    void saveIgnoredToFile();

    void loadIgnoredFile(const QString &filePath);

    void on_ignoreTXT_textChanged();

    void on_addIgnoredFolderButton_clicked();

    void on_addIgnoredFileButton_clicked();

    void on_serverUpdateTargetPath_textChanged(const QString &arg1);

    void on_checkServerTargetPath_clicked();

private:
    Ui::SettingsDialog *ui;
    QString appDirPath = QCoreApplication::applicationDirPath();
    QString settingsPath = appDirPath + "/cfg/settings.ini";

};

#endif // SETTINGSDIALOG_H
