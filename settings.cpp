#include "settings.h"
#include "ui_settings.h"
#include <QMessageBox>

settings::settings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::settings)
{
    ui->setupUi(this);
    QString appDir = QCoreApplication::applicationDirPath();

    // Создаем путь к файлу настроек в корневой папке приложения
    QString settingsFilePath = appDir + "/settings.ini";

    // Создаем объект QSettings с указанием пути к файлу настроек
    QSettings mysettings(settingsFilePath, QSettings::IniFormat);

    ui->hostName->setText(mysettings.value("server").toString());
    ui->databaseName->setText(mysettings.value("database").toString());
    ui->username->setText(mysettings.value("username").toString());
    ui->SQLQuery->setText(mysettings.value("query").toString());
}

QString settings::encryptPassword(const QString& password, const QString& key) {
    QString encryptedPassword;
    for (int i = 0; i < password.length(); ++i) {
        encryptedPassword.append(QChar(password.at(i).unicode() ^ key.at(i % key.length()).unicode()));
    }
    return encryptedPassword;
}

void settings::saveAll() {
    QString appDir = QCoreApplication::applicationDirPath();

    // Создаем путь к файлу настроек в корневой папке приложения
    QString settingsFilePath = appDir + "/settings.ini";

    // Создаем объект QSettings с указанием пути к файлу настроек
    QSettings mysettings(settingsFilePath, QSettings::IniFormat);

    QString server = ui->hostName->text();
    QString database = ui->databaseName->text();
    QString username = ui->username->text();
    QString query = ui->SQLQuery->toPlainText();

    if (server.isEmpty() || database.isEmpty() || username.isEmpty() || query.isEmpty()) {
        // Если хотя бы одно поле не заполнено, выводим предупреждение и завершаем функцию
        QMessageBox::warning(this, "Предупреждение", "Пожалуйста, заполните все поля перед сохранением.");
        return;
    }

    mysettings.setValue("server", ui->hostName->text());
    mysettings.setValue("database", ui->databaseName->text());
    mysettings.setValue("username", ui->username->text());
    mysettings.setValue("password", encryptPassword(ui->password->text(), key));
    mysettings.setValue("query", ui->SQLQuery->toPlainText());
}

settings::~settings()
{
    delete ui;
}

void settings::on_okButton_clicked()
{
    saveAll();
    emit okButtonClicked();
    close();
}


void settings::on_cancelButton_clicked()
{
    close();
}

