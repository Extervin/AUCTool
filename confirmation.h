#ifndef CONFIRMATION_H
#define CONFIRMATION_H

#include <QDialog>

namespace Ui {
class Confirmation;
}

class Confirmation : public QDialog
{
    Q_OBJECT
signals:
    void sendData(const QString& newLogin, const QString &newPassword, const bool newFlag, const QString source);
public:
    explicit Confirmation(QWidget *parent = nullptr, QList<QString> ipList = QStringList());
    ~Confirmation();

private slots:

    void ipListWidgetSpawn(QList<QString> ipList);

    void on_confirmButton_clicked();

    void on_openButton_clicked();

    void on_declineButton_clicked();

private:

    void updateFileListWidget();

    Ui::Confirmation *ui;

    QString login = "";
    QString password = "";
    bool closeDrugFlag = false;
    QStringList fileList;

    QList<QString> *ipListPtr;
    QList<QString> ipList;
    QString source;
};

#endif // CONFIRMATION_H
