#include "confirmation.h"
#include "serverinterface.h"
#include "ui_confirmation.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QProcess>

Confirmation::Confirmation(QWidget *parent, const QMap<QString, QString> &ipMap)
    : QDialog(parent),
    ui(new Ui::Confirmation),
    ipMap(ipMap)
{
    connect(this, &Confirmation::sendData, dynamic_cast<ServerInterface*>(parent), &ServerInterface::receiveData);
    connect(this, &Confirmation::openProgress, dynamic_cast<ServerInterface*>(parent), &ServerInterface::openProgressWindow);
    ui->setupUi(this); // Инициализация интерфейса
    ui->password->setEchoMode(QLineEdit::Password);
    ipListWidgetSpawn(ipMap);
    // Передача данных через сигнал
}

void Confirmation::ipListWidgetSpawn(const QMap<QString, QString>& ipMap) {
    // Очищаем список виджетов
    ui->ipListWidget->clear();

    // Добавляем IP-адреса и соответствующие имена объектов из ipMap
    for (auto it = ipMap.begin(); it != ipMap.end(); ++it) {
        QString ipAddress = it.key();
        QString objectName = it.value();
        QString listItem = objectName + " (" + ipAddress + ")";

        // Создаем элемент списка для каждой пары IP-адрес и имя объекта
        QListWidgetItem* item = new QListWidgetItem(listItem);

        // Устанавливаем флаги, чтобы элемент можно было выбирать и удалять
        item->setFlags(item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

        // Добавляем элемент в QListWidget
        ui->ipListWidget->addItem(item);
    }

    // Добавляем функциональность удаления элементов по нажатию кнопки Delete
    connect(ui->ipListWidget, &QListWidget::itemActivated, [=](QListWidgetItem* item){
        delete ui->ipListWidget->takeItem(ui->ipListWidget->row(item));
    });
}


Confirmation::~Confirmation()
{
    delete ui;
}

void Confirmation::on_confirmButton_clicked()
{
    // Получение данных из полей ввода
    QString login = ui->login->text();
    QString password = ui->password->text();
    bool closeDrugFlag = ui->checkDrug->isChecked();
    QString source = ui->sourcePath->text();

    // Проверяем наличие логина
    if (login.isEmpty()) {
        // Если логин отсутствует, выдаем сообщение об ошибке
        QMessageBox::critical(this, tr("Error"), tr("Please enter a login."));
        return;
    }

    // Проверяем наличие пароля
    if (password.isEmpty()) {
        // Если пароль отсутствует, выдаем сообщение об ошибке
        QMessageBox::critical(this, tr("Error"), tr("Please enter a password."));
        return;
    }

    // Проверяем наличие пути к источнику
    if (source.isEmpty()) {
        // Если путь к источнику отсутствует, выдаем сообщение об ошибке
        QMessageBox::critical(this, tr("Error"), tr("Please enter the source path."));
        return;
    }

    // Проверяем, можем ли мы использовать логин и пароль для подключения к первому IP-адресу в ipMap
    if (!ipMap.isEmpty()) {
        auto firstIP = ipMap.begin().key(); // Получаем первый IP-адрес из ipMap
        if (!canConnectToShare(firstIP, login, password)) {
            // Если не можем подключиться, выдаем сообщение об ошибке
            QMessageBox::critical(this, tr("Error"), tr("Cannot connect to the first selected IP address with the provided credentials."));
            return;
        }
    }

    // Отправка данных через сигнал
    emit openProgress();
    emit sendData(login, password, closeDrugFlag, source);
    accept();
}

bool Confirmation::canConnectToShare(const QString& ipAddress, const QString& username, const QString& password) {
    // Создаем команду для проверки подключения к сетевому ресурсу
    QString checkCommand = QString("net use \\\\%1\\IPC$ /user:%2 %3").arg(ipAddress, username, password);

    QProcess process;
    process.start(checkCommand);
    process.waitForFinished(-1); // Ждем завершения выполнения процесса

    int result = process.exitCode(); // Получаем код завершения

    return (result == 0); // Возвращаем true, если результат выполнения команды успешный (код 0)
}



void Confirmation::on_openButton_clicked()
{
    QString selectedFolder = QFileDialog::getExistingDirectory(this, tr("Select Folder to Update"), "C:/");
    // Проверяем, была ли выбрана папка
    if (!selectedFolder.isEmpty()) {

        source = selectedFolder;

        updateFileListWidget();
    }
}

void Confirmation::updateFileListWidget() {
    ui->sourcePath->clear();
    ui->sourcePath->setText(source);
}

void Confirmation::on_declineButton_clicked()
{

}


void Confirmation::on_sourcePath_textChanged(const QString &arg1)
{
    source = ui->sourcePath->text();
}

