#ifndef SERVERINTERFACE_H
#define SERVERINTERFACE_H

#include <QMainWindow>
#include "switchbutton.h" // Добавьте это

QT_BEGIN_NAMESPACE
namespace Ui { class ServerInterface; }
QT_END_NAMESPACE

class ServerInterface : public QMainWindow
{
    Q_OBJECT

public:
    ServerInterface(QWidget *parent = nullptr);
    ~ServerInterface();

private slots:
    void onSwitchToggled(bool checked);
    void spawnTable();
    void spawnMenu();

    void cancelFilter();

    void applyCityFilter(const QString &city);
    void applyTagFilter(const QString &tag);
    void applyStatusFilter(const QString &status);


private:
    Ui::ServerInterface *ui;
    int cityFilterIndex = 0;
    int statusFilterIndex = 0;
    int tagFilterIndex = 0;
};

#endif // SERVERINTERFACE_H
