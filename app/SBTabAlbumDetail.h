#ifndef SBTABALBUMDETAIL_H
#define SBTABALBUMDETAIL_H

#include "ExternalData.h"
#include "SBTab.h"

#include "SBIDAlbum.h"

class SBTabAlbumDetail : public SBTab
{
    Q_OBJECT

    enum sb_tab
    {
        sb_tab_contents =0,
        sb_tab_reviews,
        sb_tab_wikipedia
    };

public:
    SBTabAlbumDetail();

    virtual QTableView* subtabID2TableView(int subtabID) const;
    virtual QTabWidget* tabWidget() const;

public slots:
    virtual void playNow(bool enqueueFlag=0);
    void showContextMenuLabel(const QPoint &p);
    void showContextMenuView(const QPoint &p);

private slots:
    void refreshAlbumReviews();
    void setAlbumImage(const QPixmap& p, const SBKey& key=SBKey());
    void setAlbumReviews(const QList<QString>& reviews);
    void setAlbumWikipediaPage(const QString& url);

private:
    ExternalData 		_ed;
    QList<QString> 		_currentReviews;

    void _init();
    virtual QTableView* _determineViewCurrentTab() const;
    virtual ScreenItem 	_populate(const ScreenItem& id);
};

#endif // SBTABALBUMDETAIL_H
