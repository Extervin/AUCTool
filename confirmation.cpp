#include "confirmation.h"
#include "serverinterface.h"
#include "ui_confirmation.h"
#include <QFileDialog>
#include <QDir>

Confirmation::Confirmation(QWidget *parent, QList<QString> ipList)
    : QDialog(parent),
    ui(new Ui::Confirmation),
    ipList(ipList)
{
    connect(this, &Confirmation::sendData, dynamic_cast<ServerInterface*>(parent), &ServerInterface::receiveData);
    ui->setupUi(this); // Инициализация интерфейса
    ui->password->setEchoMode(QLineEdit::Password);
    ipListWidgetSpawn(ipList);
    // Передача данных через сигнал
}

void Confirmation::ipListWidgetSpawn(QList<QString> ipList) {
    // Очищаем список виджетов
    ui->ipListWidget->clear();

    // Добавляем IP-адреса из ipList
    for (const QString& ipAddress : ipList) {
        // Создаем элемент списка для каждого IP-адреса
        QListWidgetItem* item = new QListWidgetItem(ipAddress);

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
    QStringList fileList; // Добавьте код для получения данных из списка файлов

    // Отправка данных через сигнал
    emit sendData(login, password, closeDrugFlag, source);
    accept();
}


void Confirmation::on_openButton_clicked()
{
    QString selectedFolder = QFileDialog::getExistingDirectory(this, tr("Select Folder to Update"), "C:/");
    source = selectedFolder;
    // Проверяем, была ли выбрана папка
    if (!selectedFolder.isEmpty()) {
        // Очищаем список файлов перед добавлением новой папки
        fileList.clear();

        // Добавляем выбранную папку в список файлов
        fileList.append(selectedFolder);

        // Обновляем отображение списка файлов в QListWidget
        updateFileListWidget();
    }
}

void Confirmation::updateFileListWidget() {
    ui->fileListWidget->clear();
    ui->fileListWidget->addItems(fileList);
}

void Confirmation::on_declineButton_clicked()
{

}

