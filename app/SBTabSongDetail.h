#ifndef SBTABSONGDETAIL_H
#define SBTABSONGDETAIL_H

#include "SBTab.h"

class SBTabSongDetail : public SBTab
{
    Q_OBJECT

public:
    SBTabSongDetail(QWidget* parent=0);
    virtual QTableView* subtabID2TableView(int subtabID) const;
    virtual QTabWidget* tabWidget() const;

private slots:
    void setSongLyricsPage(const QString& url);
    void setSongWikipediaPage(const QString& url);

private:
    void init();
    virtual SBID _populate(const SBID& id);
};

#endif // SBTABSONGDETAIL_H
