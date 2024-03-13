#include <QFile>
#include <QVector>

#include "ExternalData.h"
#include "SBHtmlAlbumsAll.h"
#include "SBHtmlPerformersAll.h"
#include "SBHtmlSongsAll.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "SBIDSongPerformance.h"
#include "SBSqlQueryModel.h"

static const QString albums=QString("___SB_ALBUMS___");
static const QString performers=QString("___SB_PERFORMERS___");
static const QString songs=QString("___SB_SONGS___");

SBHtmlPerformersAll::SBHtmlPerformersAll() {}

QString
SBHtmlPerformersAll::performerDetail(QString html, const QString& key)
{
    QString contents;
    html.replace('\n',"");
    html.replace('\t'," ");

    SBKey performerKey=SBKey(key.toLatin1());

    if(performerKey.validFlag())
    {
        SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(performerKey);

        if(pPtr)
        {
            QString table;
            QString iconLocation;
            QString playerControlHTML;

            //  Related performers
            QVector<SBIDPerformerPtr> relatedPerformers=pPtr->relatedPerformers();
            table=QString();

            if(relatedPerformers.count())
            {
                QVectorIterator<SBIDPerformerPtr> pIT(relatedPerformers);
                table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Related Performers:</P></TD></TR>");
                while(pIT.hasNext())
                {
                    SBIDPerformerPtr rpPtr=pIT.next();

                    if(rpPtr)
                    {
                        iconLocation=_getIconLocation(rpPtr, SBKey::Performer);
                        playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                .arg(rpPtr->key().toString());
                        const QString row=QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" >"
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBItemMajor\"  onclick=\"open_page('%3','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\" >"
                                    "%4"
                                "</TD>"
                            "</TR>"
                        )
                            .arg(iconLocation)
                            .arg(Common::escapeQuotesHTML(rpPtr->performerName()))
                            .arg(rpPtr->key().toString())
                            .arg(playerControlHTML)
                        ;
                        table+=row;
                    }
                }
            }
            html.replace(performers,table);

            //  Songs
            QVector<SBIDSongPerformancePtr> songPerformances=pPtr->songPerformances();
            table=QString();

            if(songPerformances.count())
            {
                QVector<SBKey> processedSong;

                table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Songs:</P></TD></TR>");
                QVectorIterator<SBIDSongPerformancePtr> spIT(songPerformances);
                while(spIT.hasNext())
                {
                    SBIDSongPerformancePtr spPtr=spIT.next();
                    QString iconLocation;

                    if(spPtr)
                    {
                        if(processedSong.indexOf(spPtr->key())==-1)
                        {
                            SBIDOnlinePerformancePtr opPtr=spPtr->preferredOnlinePerformancePtr();
                            QString playerControlHTML;
                            SBKey keyToUse;

                            if(opPtr)
                            {
                                iconLocation=SBHtmlSongsAll::_getIconLocation(opPtr, SBKey::OnlinePerformance);
                                playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                    .arg(opPtr->key().toString());
                            }
                            else
                            {
                                iconLocation=ExternalData::getDefaultIconPath(SBKey::Performer);
                            }

                            const QString row=QString(
                                "<TR>"
                                    "<TD class=\"SBIconCell\" >"
                                        "<img class=\"SBIcon\" src=\"%1\"></img>"
                                    "</TD>"
                                    "<TD class=\"SBItemMajor\"  onclick=\"open_page('%3','%2');\">%2</TD>"
                                    "<TD class=\"playercontrol_button\" >"
                                        "%4"
                                    "</TD>"
                                "</TR>"
                            )
                                .arg(iconLocation)
                                .arg(Common::escapeQuotesHTML(spPtr->songTitle()))
                                .arg(spPtr->songKey().toString())
                                .arg(playerControlHTML)
                            ;
                            table+=row;
                            processedSong.append(spPtr->key());
                        }
                    }
                }
            }
            html.replace(songs,table);

            //  Albums
            QVector<SBIDAlbumPtr> allAlbums=pPtr->albumList();
            table=QString();

            if(allAlbums.count())
            {
                table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Albums:</P></TD></TR>")+table;

                QVectorIterator<SBIDAlbumPtr> apIT(allAlbums);
                while(apIT.hasNext())
                {
                    const SBIDAlbumPtr aPtr=apIT.next();
                    if(aPtr)
                    {
                        QString playerControlHTML;
                        playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                .arg(aPtr->key().toString());

                        QString row=QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" >"
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBItemMajor\"  onclick=\"open_page('%4','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\" >"
                                    "%3"
                                "</TD>"
                            "</TR>"
                        )
                            .arg(SBHtmlAlbumsAll::_getIconLocation(aPtr, SBKey::Album))
                            .arg(Common::escapeQuotesHTML(aPtr->albumTitle()))
                            .arg(playerControlHTML)
                            .arg(aPtr->key().toString())
                        ;
                        qDebug() << SB_DEBUG_INFO << row;
                        table+=row;
                    }
                }
            }
            html.replace(albums,table);

            // //  Charts
            // QMap<SBIDChartPerformancePtr, SBIDChartPtr> allCharts=pPtr->charts(SBIDSong::retrieve_qmap());
            // table=QString();
            // if(allCharts.count())
            // {
            //     table=QString("<TR><TD colspan=\"3\" class=\"SBItemSection\">Charts:</TD></TR>");
            //     table+=QString("<TR><TD colspan=\"3\" class=\"SBItemMinor\">");
            //     QMapIterator<SBIDChartPerformancePtr, SBIDChartPtr> it(allCharts);
            //     while(it.hasNext())
            //     {
            //         it.next();
            //         SBIDChartPtr cPtr=it.value();
            //         if(cPtr)
            //         {
            //             table+=QString("<LI><A class=\"SBItemMinor\">%1</A></LI>").arg(cPtr->chartName());
            //         }
            //     }
            //     table+=QString("</TD></TR>");
            // }
            // html.replace(charts,table);
        }
    }
    return html;

}

QString
SBHtmlPerformersAll::retrieveAllPerformers(const QChar& startsWith, qsizetype offset, qsizetype size)
{
    QString table;

    //  Let's retrieve size+1 songs to see if there is anything left after the
    //  current batch.I
    SBSqlQueryModel* sm=SBIDPerformer::retrieveAllPerformers(startsWith, offset, size+1);

    bool moreSongsNext=0;
    bool moreSongsPrev=(offset>0)?1:0;

    qsizetype availableCount=sm->rowCount();
    if(availableCount>size)
    {
        moreSongsNext=1;
        availableCount=size;
    }

    for(int i=0;i<size;i++)
    {
        const SBKey performerKey(sm->record(i).value(0).toByteArray());
        SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(performerKey);

        if(pPtr)
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
                .arg(Common::escapeQuotesHTML(pPtr->performerName()))
                .arg(pPtr->key().toString())
                .arg(_getIconLocation(pPtr,SBKey::Performer))
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

QString
SBHtmlPerformersAll::_getIconLocation(const SBIDPerformerPtr& pPtr, const SBKey::ItemType& defaultType)
{
    SB_RETURN_IF_NULL(pPtr,ExternalData::getDefaultIconPath(defaultType));

    QString iconLocation;
    SBKey iconKey;

    const SBKey performerKey=pPtr->key();
    iconLocation=ExternalData::getCachePath(performerKey);

    if(QFile::exists(iconLocation))
    {
        iconKey=performerKey;
    }

    if(!iconKey.validFlag())
    {
        //	Retrieve std song icon
        iconLocation=ExternalData::getDefaultIconPath(defaultType);
    }
    else
    {
        iconLocation=QString("/icon/%1").arg(iconKey.toString());
    }
    return iconLocation;
}
