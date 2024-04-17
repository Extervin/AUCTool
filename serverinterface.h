#ifndef SERVERINTERFACE_H
#define SERVERINTERFACE_H

#include <QMainWindow>
#include "switchbutton.h"
#include "objects_table.h"
#include <QWidget>
#include <QString>
#include <QMap>
#include <QSet>
#include <QCheckBox>
#include <QSettings>

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
    void handleCopyFinished(const QString &ipAddress, const int &finishCode);

    void receiveData(const QString& newLogin, const QString &newPassword, const bool newFlag, const QString source, const QString target, const bool straightCopyFlag);

    void receiveDebugInfo(const QString& info);

    void openProgressWindow();

    void recieveError(const QString& ipAddress, const QString& errorType, const QString& errorMessage);

    void spawnTable();

signals:
    void pingResult(const QString& ipAddress, bool pingSuccess);

    void sendDebugInfoInProgress(const QString& info);

    void sendUpdateProgressBar();

    void sendListSize(const int& listSize);

private slots:
    void onSwitchToggled(bool checked);

    void spawnMenu();

    void cancelFilter(QWidget *filterWidget);
    void applyFilter(const QString &columnName, const QString &value, const QString &displayText, const bool chosenOperator);
    void updateFilter();

    void applyCityFilter(const QString &city);
    void applyTagFilter(const QString &tag);
    void applyStatusFilter(const QString &status);

    void checkPingInMainThread(const QString &ipAddress);

    void on_searchButton_clicked();

    void updateServerFiles(const bool closeFlag, const QString &login, const QString &password, const QString source, const QString target, const bool straightCopyFlag);

    void on_markSetManual_stateChanged(int arg1);
    bool eventFilter(QObject *obj, QEvent *event);

    void on_serverUpdateButton_clicked();

    void on_checkAll_stateChanged(int arg1);

    void applyNameFilter(const QString &name);
    void removeNameFilter(const QString &name);
    void refreshTable();
    QString buildNameFilterQuery();
    void on_printButton_clicked();
    QString decryptPassword(const QString& encryptedPassword, const QString& key);
    void on_databaseButton_clicked();

    void on_refreshTableButton_clicked();

private:
    Ui::ServerInterface *ui;
    ObjectsTable *model = new ObjectsTable(this);

    int cityFilterIndex = 0;
    int statusFilterIndex = 0;
    int tagFilterIndex = 0;
    int filterIndex = 0;

    QSet<QString> leadersSet;
    QSet<QString> selectedNamesSet;
    int currentSearchIndex = -1;
    QMap<QWidget*, QString> appliedFilters;
    QString currentQuery = "";

    QMap<QString, QCheckBox*> leaderCheckboxes;
    QString key = "balls";
    QString login = "";
    QString password = "";
    bool closeDrugFlag = false;
    QList<QString> fileList;
    bool filterInclude = true;
    QString stupidTechnicans = "";
};

#endif // SERVERINTERFACE_H
