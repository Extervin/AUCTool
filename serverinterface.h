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


private:
    Ui::ServerInterface *ui;
    ObjectsTable *objectsTableModel;

    int cityFilterIndex = 0;
    int statusFilterIndex = 0;
    int tagFilterIndex = 0;
    int filterIndex = 0;

    QMap<QWidget*, QString> appliedFilters;
    QString currentQuery;
};

#endif // SERVERINTERFACE_H
