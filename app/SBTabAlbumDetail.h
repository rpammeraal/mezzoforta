#ifndef SBTABALBUMDETAIL_H
#define SBTABALBUMDETAIL_H

#include "SBTab.h"

class SBTabAlbumDetail : public SBTab
{
    Q_OBJECT

public:
    SBTabAlbumDetail();
    virtual QTableView* subtabID2TableView(int subtabID) const;
    virtual QTabWidget* tabWidget() const;


private slots:
    void refreshAlbumReviews();
    void setAlbumImage(const QPixmap& p);
    void setAlbumReviews(const QList<QString>& reviews);
    void setAlbumWikipediaPage(const QString& url);

private:
    QList<QString> currentReviews;

    void init();
    virtual SBID _populate(const SBID& id);
};

#endif // SBTABALBUMDETAIL_H
