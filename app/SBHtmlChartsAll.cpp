#include <QSqlRecord>

#include "ExternalData.h"
#include "SBIDChart.h"
#include "SBIDChartPerformance.h"
#include "SBIDSong.h"
#include "SBHtmlChartsAll.h"
#include "SBSqlQueryModel.h"

static const QString songs=QString("___SB_SONGS___");

SBHtmlChartsAll::SBHtmlChartsAll() {}

QString
SBHtmlChartsAll::chartDetail(QString html, const QString& key)
{
    QString contents;
    html.replace('\n',"");
    html.replace('\t'," ");

    SBKey chartKey=SBKey(key.toLatin1());

    if(chartKey.validFlag())
    {
        SBIDChartPtr cPtr=SBIDChart::retrieveChart(chartKey);

        if(cPtr)
        {
            QString table;

            //  Create list of song instances (e.g. all instances on an album)
            QMap<int, SBIDChartPerformancePtr> allItems=cPtr->items();
            table=QString();

            if(allItems.count())
            {
                QMapIterator<int, SBIDChartPerformancePtr> apIt(allItems);
                //  Remap so we can display songs in order of appearance on album
                QMap<qsizetype,qsizetype> itemOrderMap;

                while(apIt.hasNext())
                {
                    apIt.next();
                    const SBIDChartPerformancePtr cpPtr=apIt.value();
                    if(cpPtr)
                    {
                        itemOrderMap[cpPtr->chartPosition()]=apIt.key();
                    }
                }

                for(qsizetype i=0; i<itemOrderMap.size();i++)
                {
                    const SBIDChartPerformancePtr cpPtr=allItems.value(itemOrderMap[i+1]);
                    if(cpPtr)
                    {
                        SBKey itemKey;
                        QString iconLocation;

                        const SBIDSongPtr sPtr=cpPtr->songPtr();
                        if(sPtr)
                        {
                            itemKey=sPtr->key();
                            //  iconLocation=SBHtmlSongsAll::_getIconLocation(sPtr);    //  CWIP
                        }


                        QString  playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%1');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                        .arg(sPtr->key().toString());
                            ;

                        QString row=QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" "
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBIconCell\">%5</TD>"
                                "<TD class=\"SBItemMajor\"  onclick=\"open_page('%4','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\">"
                                    "%3"
                                "</TD>"
                            "</TR>"
                        )
                            .arg(ExternalData::getDefaultIconPath(SBKey::Chart))
                            .arg(Common::escapeQuotesHTML(cpPtr->genericDescription()))
                            .arg(playerControlHTML)
                            .arg(itemKey.toString())
                            .arg(cpPtr->chartPosition())
                        ;
                        table+=row;
                    }
                }
                table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Contains:</P></TD></TR>")+table;
            }
            html.replace(songs,table);
        }
    }
    return html;

}

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
    }

    const static QString iconLocation=ExternalData::getDefaultIconPath(SBKey::Chart);
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
