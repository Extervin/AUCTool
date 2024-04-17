#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class settings;
}

class settings : public QDialog
{
    Q_OBJECT

public:
    explicit settings(QWidget *parent = nullptr);
    ~settings();

    QString encryptPassword(const QString& password, const QString& key);
    void saveAll();

signals:
    void okButtonClicked();

private slots:
    void on_okButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::settings *ui;
    QString key = "balls";
};

#endif // SETTINGS_H
