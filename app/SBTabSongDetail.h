#ifndef SBTABSONGDETAIL_H
#define SBTABSONGDETAIL_H

#include "SBTab.h"

class QMenu;

class SBTabSongDetail : public SBTab
{
    Q_OBJECT

    enum sb_tab
    {
        sb_tab_albums =0,
        sb_tab_playlists,
        sb_tab_charts,
        sb_tab_lyrics,
        sb_tab_wikipedia
    };

public:
    SBTabSongDetail(QWidget* parent=0);
    virtual QTableView* subtabID2TableView(int subtabID) const;
    virtual QTabWidget* tabWidget() const;
    static SBID selectSongFromAlbum(const SBID& song);

public slots:
    void enqueue();
    void playNow(bool enqueueFlag=0);
    void showContextMenuView(const QPoint &p);
    void showContextMenuLabel(const QPoint &p);

private slots:
    void setSongLyricsPage(const QString& url);
    void setSongWikipediaPage(const QString& url);

private:
    QMenu* _menu;
    QAction* _playNowAction;
    QAction* _enqueueAction;
    QList<QWidget *> _alsoPerformedBy;

    void _init();
    virtual SBID _populate(const SBID& id);
    QTableView* _determineViewCurrentTab() const;
};

#endif // SBTABSONGDETAIL_H
