#ifndef SBTABSONGDETAIL_H
#define SBTABSONGDETAIL_H

#include "SBTab.h"

class SBTabSongDetail : public SBTab
{
    Q_OBJECT

public:
    SBTabSongDetail();
    virtual SBID populate(const SBID& id);

private slots:
    void setSongLyricsPage(const QString& url);
    void setSongWikipediaPage(const QString& url);
};

#endif // SBTABSONGDETAIL_H
