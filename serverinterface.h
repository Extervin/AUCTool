#ifndef SERVERINTERFACE_H
#define SERVERINTERFACE_H

#include <QMainWindow>
#include "switchbutton.h"
#include "objects_table.h"
#include <QWidget>
#include <QString>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class ServerInterface; }
QT_END_NAMESPACE

class ServerInterface : public QMainWindow
{
    Q_OBJECT

public:
    explicit ServerInterface(QWidget *parent = nullptr);
    ~ServerInterface();
public slots:
    void handleCopyFinished(const QString &ipAddress, const QString &share);

signals:
    void pingResult(const QString& ipAddress, bool pingSuccess);

private slots:
    void onSwitchToggled(bool checked);
    void spawnTable();
    void spawnMenu();

    void cancelFilter(QWidget *filterWidget);
    void applyFilter(const QString &columnName, const QString &value, const QString &displayText, const bool chosenOperator);
    void updateFilter();

    void applyCityFilter(const QString &city);
    void applyTagFilter(const QString &tag);
    void applyStatusFilter(const QString &status);

    void checkPingInMainThread(const QString &ipAddress);

    void on_searchButton_clicked();

    void updateServerFiles(const bool closeFlag);

    void on_markSetManual_stateChanged(int arg1);
    bool eventFilter(QObject *obj, QEvent *event);

    void on_serverUpdateButton_clicked();

private:
    Ui::ServerInterface *ui;
    ObjectsTable *model = new ObjectsTable(this);

    int cityFilterIndex = 0;
    int statusFilterIndex = 0;
    int tagFilterIndex = 0;
    int filterIndex = 0;

    QMap<QWidget*, QString> appliedFilters;
    QString currentQuery = "SELECT Obekt, IT, IP, a1, a2 FROM acc_blank";
};

#endif // SERVERINTERFACE_H
