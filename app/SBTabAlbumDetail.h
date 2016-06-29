#ifndef SBTABALBUMDETAIL_H
#define SBTABALBUMDETAIL_H

#include "SBTab.h"

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
    void setAlbumImage(const QPixmap& p);
    void setAlbumReviews(const QList<QString>& reviews);
    void setAlbumWikipediaPage(const QString& url);

private:
    QList<QString> _currentReviews;

    void _init();
    virtual QTableView* _determineViewCurrentTab() const;
    virtual SBID _populate(const SBID& id);
};

#endif // SBTABALBUMDETAIL_H
