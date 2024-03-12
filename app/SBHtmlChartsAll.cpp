#include <QSqlRecord>

#include "ExternalData.h"
#include "SBIDChart.h"
#include "SBHtmlChartsAll.h"
#include "SBSqlQueryModel.h"

SBHtmlChartsAll::SBHtmlChartsAll() {}

QString
SBHtmlChartsAll::retrieveAllCharts(const QChar& startsWith, qsizetype offset, qsizetype size)
{
    QString table;

    //  Let's retrieve size+1 songs to see if there is anything left after the
    //  current batch.I
    SBSqlQueryModel* sm=SBIDChart::allCharts(startsWith, offset, size+1);

    bool moreSongsNext=0;
    bool moreSongsPrev=(offset>0)?1:0;

    qsizetype availableCount=sm->rowCount();
    if(availableCount>size)
    {
        moreSongsNext=1;
        availableCount=size;
    }

    const static QString iconLocation=ExternalData::getDefaultIconPath();
    for(int i=0;i<size;i++)
    {
        const SBKey chartKey(sm->record(i).value(0).toByteArray());
        SBIDChartPtr cPtr=SBIDChart::retrieveChart(chartKey);

        if(cPtr)
        {
            //	Start table row
            const QString row=QString(
                "<THEAD>"
                    "<TR>"
                        "<TD class=\"SBIconDiv\" >"
                            "<img class=\"SBIcon\" src=\"%3\"></img>"
                        "</TD>"
                        "<TD class=\"SBItemMajor\" onclick=\"open_page('%2','%1');\">%1</TD>"
                        "<TD class=\"playercontrol_button\" >"
                            "<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>"
                        "</TD>"
                    "</TR>"
                "</THEAD>"
            )
                .arg(Common::escapeQuotesHTML(cPtr->chartName()))
                .arg(cPtr->key().toString())
                .arg(iconLocation)
            ;
            table+=row;
        }
    }
    table+=QString("<DIV id=\"sb_paging_prev_ind\"><P>%1</P></DIV><DIV id=\"sb_paging_next_ind\"><P>%2</P></DIV>")
                 .arg(moreSongsPrev)
                 .arg(moreSongsNext);
    sm->deleteLater();
    return table;
}
