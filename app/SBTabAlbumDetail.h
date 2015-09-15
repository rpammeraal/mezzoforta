#ifndef SBTABALBUMDETAIL_H
#define SBTABALBUMDETAIL_H

#include "SBTab.h"

class SBTabAlbumDetail : public SBTab
{
    Q_OBJECT

public:
    SBTabAlbumDetail();

    virtual SBID populate(const SBID& id);

private slots:
    void refreshAlbumReviews();
    void setAlbumImage(const QPixmap& p);
    void setAlbumReviews(const QList<QString>& reviews);
    void setAlbumWikipediaPage(const QString& url);

private:
    QList<QString> currentReviews;

    void init();
};

#endif // SBTABALBUMDETAIL_H
