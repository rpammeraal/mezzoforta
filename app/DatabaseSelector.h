#ifndef DATABASESELECTOR_H
#define DATABASESELECTOR_H


#include "ui_DatabaseSelector.h"

#include "DBManager.h"

///
/// \brief The DatabaseSelector class
///
///	DatabaseSelector lets the user select another database.
/// Opening itself of the database is done by DBManager.
class DatabaseSelector: public QDialog
{
    Q_OBJECT

public:
    DatabaseSelector(const struct DBManager::DatabaseCredentials& dc,QWidget* parent=0);
    ~DatabaseSelector();

    inline struct DBManager::DatabaseCredentials databaseCredentials() const { return _dc; }

protected:

private:
    Ui::DatabaseSelector* ui;
    bool _sqliteDriverAvailable;
    bool _postgresDriverAvailable;
    struct DBManager::DatabaseCredentials _dc;

    void _populateUI(const struct DBManager::DatabaseCredentials& dc);
    void _determineAvailableDBTypes();

private slots:
    void _browseFile();
    void _acceptInput();
};

#endif // DATABASESELECTOR_H
