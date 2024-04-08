#ifndef CONFIRMATION_H
#define CONFIRMATION_H

#include <QDialog>
#include <QMap>

namespace Ui {
class Confirmation;
}

class Confirmation : public QDialog
{
    Q_OBJECT
signals:
    void sendData(const QString& newLogin, const QString &newPassword, const bool newFlag, const QString source);

    void openProgress();

public:
    explicit Confirmation(QWidget *parent = nullptr, const QMap<QString, QString>& ipMap = QMap<QString, QString>());
    ~Confirmation();


private slots:

    void ipListWidgetSpawn(const QMap<QString, QString>& ipMap);

    void on_confirmButton_clicked();

    void on_openButton_clicked();

    void on_declineButton_clicked();

    void on_sourcePath_textChanged(const QString &arg1);

private:

    void updateFileListWidget();

    bool canConnectToShare(const QString& ipAddress, const QString& username, const QString& password);

    Ui::Confirmation *ui;

    QString login = "";
    QString password = "";
    bool closeDrugFlag = false;

    QList<QString> *ipListPtr;
    QMap<QString, QString> ipMap;
    QString source;
};

#endif // CONFIRMATION_H
