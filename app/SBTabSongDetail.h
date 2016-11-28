#ifndef SBTABSONGDETAIL_H
#define SBTABSONGDETAIL_H

#include "SBTab.h"

#include "SBIDAlbum.h"
#include "SBIDSong.h"

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
    static SBIDAlbumPerformancePtr selectPerformanceFromSong(SBIDSongPtr& songOnUnknownAlbumPtr, bool playableOnlyFlag);	//	CWIP: move to SBDIalogSelectItem

public slots:
    virtual void playNow(bool enqueueFlag=0);
    void showContextMenuLabel(const QPoint &p);
    void showContextMenuView(const QPoint &p);

private slots:
    void setSongLyricsPage(const QString& url);
    void setSongWikipediaPage(const QString& url);

private:
    QList<QWidget *> _alsoPerformedBy;

    virtual QTableView* _determineViewCurrentTab() const;
    void _init();
    virtual ScreenItem _populate(const ScreenItem& id);
};

#endif // SBTABSONGDETAIL_H
