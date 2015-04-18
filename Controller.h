#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QStringList>
#include <QModelIndex>

class QItemSelection;
class QSortFilterProxyModel;
class QKeyEvent;
class QStandardItemModel;

class MainWindow;
class DataAccessLayer;

#define SB_TAB_UNDEF    -1
#define SB_TAB_PLAYLIST  0
#define SB_TAB_GENRE     1

class Controller : public QObject
{
    Q_OBJECT

    friend class MainWindow;

public:
    explicit Controller(MainWindow* mw,DataAccessLayer *dal);
    ~Controller();

signals:

public slots:

    //	Apply filters and selections
    void applySongListFilter(const QString& filter="");
    void applyPlaylistSelection(const QItemSelection &selected, const QItemSelection &deselected);
    void applyGenreSelection(const QItemSelection &selected, const QItemSelection &deselected);
    void changeCurrentTab(int index);

    //	Data Updates
    void updateGenre(QModelIndex i,QModelIndex j);

protected:
    virtual void keyPressEvent(QKeyEvent * event);

private:
    MainWindow* mw;
    DataAccessLayer* dal;
    Controller* c;
    QSortFilterProxyModel* songListFilter;
    bool doExactSearch;

    //	Keep track of what is selected
    int selectedPlaylistID;     //	-1 indicates none
    QStringList selectedGenres; //	speaks for itself
    QString currentFilter;      //	"" indicates no filter

    //	Handle filters and selections
    void updateCurrentSongList();

    //	Handle reset of filters and selections
    void resetAllFiltersAndSelections();
    void clearPlaylistSelection();
    void clearGenreSelection();
    void clearSearchFilter();

    //	UI related
    void populateUI();
    int getSelectedTab();

    //	Data config
    QStandardItemModel* configGenreData();
};

#endif // CONTROLLER_H
