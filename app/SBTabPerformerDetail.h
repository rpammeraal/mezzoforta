#ifndef SBTABPERFORMERDETAIL_H
#define SBTABPERFORMERDETAIL_H

#include "SBTab.h"

#include "ExternalData.h"
#include "SBIDPerformer.h"

class SBTabPerformerDetail : public SBTab
{
    Q_OBJECT

    enum sb_tab
    {
        sb_tab_performances =0,
        sb_tab_albums,
        sb_tab_charts,
        sb_tab_news,
        sb_tab_wikipedia,
        sb_tab_homepage
    };

public:
    SBTabPerformerDetail(QWidget* parent=0);

    virtual QTableView* subtabID2TableView(int subtabID) const;
    virtual QTabWidget* tabWidget() const;

public slots:
    virtual void playNow(bool enqueueFlag=0);
    void showContextMenuLabel(const QPoint &p);
    void showContextMenuView(const QPoint &p);
    void updatePerformerHomePage(const SBIDBasePtr& ptr);
    void updatePerformerMBID(const SBIDBasePtr& ptr);

private slots:
    void refreshPerformerNews();
    void setPerformerHomePage(const QString& url);
    void setPerformerImage(const QPixmap& p);
    void setPerformerNews(const QList<NewsItem>& news);
    void setPerformerWikipediaPage(const QString& url);

private:
    QList<NewsItem>  _currentNews;
    QList<QWidget *> _relatedItems;

    virtual QTableView* _determineViewCurrentTab() const;
    void _init();
    virtual ScreenItem _populate(const ScreenItem& id);
};

#endif // SBTABPERFORMERDETAIL_H
