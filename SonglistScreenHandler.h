#ifndef SONGLISTSCREENHANDLER_H
#define SONGLISTSCREENHANDLER_H

#include <QObject>

#include "ScreenStack.h"

class QTableView;
class SBID;

class SonglistScreenHandler : public QObject
{
    Q_OBJECT

public:
    SonglistScreenHandler();
    SonglistScreenHandler(SonglistScreenHandler const&);
    void operator=(SonglistScreenHandler const&);
    ~SonglistScreenHandler();

    SBID activateTab(const SBID& id);
    void moveTab(int direction);
    void pushScreen(SBID& id);
    SBID populateSongDetail(const SBID& id);
    int populateTableView(QTableView* tv, QString& query, int initialSortColumn);


public slots:
    void showSonglist();
    void songlistCellDoubleClicked(const QModelIndex& i);
    void tabBackward();
    void tabForward();

private:
    ScreenStack st;
};

#endif // SONGLISTSCREENHANDLER_H
