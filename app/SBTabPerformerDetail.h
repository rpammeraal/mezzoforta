#ifndef SBTABPERFORMERDETAIL_H
#define SBTABPERFORMERDETAIL_H

#include "SBTab.h"

#include "ExternalData.h"

class SBTabPerformerDetail : public SBTab
{
    Q_OBJECT

public:
    SBTabPerformerDetail(QWidget* parent=0);
    virtual QTableView* subtabID2TableView(int subtabID) const;
    virtual QTabWidget* tabWidget() const;

private slots:
    void refreshPerformerNews();
    void setPerformerHomePage(const QString& url);
    void setPerformerImage(const QPixmap& p);
    void setPerformerNews(const QList<NewsItem>& news);
    void setPerformerWikipediaPage(const QString& url);

private:
    QList<NewsItem> currentNews;
    QList<QWidget *> relatedItems;

    void init();
    virtual SBID _populate(const SBID& id);
};

#endif // SBTABPERFORMERDETAIL_H
