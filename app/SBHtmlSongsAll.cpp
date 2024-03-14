#include <QFile>
#include <QString>
#include <QVectorIterator>

#include "ExternalData.h"
#include "SBSqlQueryModel.h"
#include "SBHtmlSongsAll.h"
#include "SBIDAlbum.h"
#include "SBIDChart.h"
#include "SBIDChartPerformance.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "SBHtmlAlbumsAll.h"

SBHtmlSongsAll::SBHtmlSongsAll()
{
}

static const QString albums=QString("___SB_ALBUMS___");
static const QString playlists=QString("___SB_PLAYLISTS___");
static const QString charts=QString("___SB_CHARTS___");
static const QString lyrics=QString("___SB_LYRICS___");
static const QString empty;

QString
SBHtmlSongsAll::songDetail(QString html, const QString& key)
{
    QString contents;
    html.replace('\n',"");
    html.replace('\t'," ");

    SBKey songKey=SBKey(key.toLatin1());

    if(songKey.validFlag())
    {
        SBIDSongPtr sPtr=SBIDSong::retrieveSong(songKey);

        if(sPtr)
        {
            QString table;

            //  Create list of song instances (e.g. all instances on an album)
            QVector<SBIDAlbumPerformancePtr> allAlbumPerformances=sPtr->allPerformances();
            table=QString();

            if(allAlbumPerformances.count())
            {
                table=QString("<TR><TD colspan=\"5\"><P class=\"SBItemSection\">Albums:</P></TD></TR>");
                QVectorIterator<SBIDAlbumPerformancePtr> apIt(allAlbumPerformances);
                while(apIt.hasNext())
                {
                    const SBIDAlbumPerformancePtr apPtr=apIt.next();
                    if(apPtr)
                    {
                        QString iconLocation;
                        QString playerControlHTML;
                        SBIDOnlinePerformancePtr opPtr=apPtr->preferredOnlinePerformancePtr();
                        if(opPtr)
                        {
                            iconLocation=_getIconLocation(opPtr, SBKey::Album);
                            playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                    .arg(opPtr->key().toString())
                            ;
                        }
                        else
                        {
                            iconLocation=ExternalData::getDefaultIconPath(SBKey::Song);
                            playerControlHTML=empty;
                        }

                        QString row=QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" rowspan=\"2\">"
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBItemMajor\" colspan=\"3\" onclick=\"open_page('%5','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\" rowspan=\"2\">"
                                    "%3"
                                "</TD>"
                            "</TR>"
                            "<TR>"
                                "<TD pos=\"84\" colspan=\"3\" class=\"SBItemMinor\" >%4</TD>"
                            "</TR>"
                        )
                            .arg(iconLocation)
                            .arg(Common::escapeQuotesHTML(opPtr->albumTitle()))
                            .arg(playerControlHTML)
                            .arg(Common::escapeQuotesHTML(opPtr->songPerformerName()))
                            .arg(apPtr->albumKey().toString())
                        ;
                        table+=row;
                    }
                }
            }
            html.replace(albums,table);

            //  Playlists
            QVector<SBIDSong::PlaylistOnlinePerformance> allPlaylists=sPtr->playlists(Common::retrieve_qvector());
            table=QString();
            if(allPlaylists.count())
            {
                table=QString("<TR><TD colspan=\"5\" class=\"SBItemSection\">Playlists:</TD></TR>");
                QVectorIterator<SBIDSong::PlaylistOnlinePerformance> it(allPlaylists);
                while(it.hasNext())
                {
                    SBIDSong::PlaylistOnlinePerformance pop=it.next();
                    SBIDPlaylistPtr plPtr=pop.plPtr;

                    if(plPtr)
                    {
                        QString  playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%1');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                        .arg(plPtr->key().toString());
                            ;
                        QString row=QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" >"
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBItemMajor\" colspan=\"3\" onclick=\"open_page('%4','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\" >"
                                    "%3"
                                "</TD>"
                            "</TR>"
                        )
                            .arg(ExternalData::getDefaultIconPath(SBKey::PlaylistDetail))
                            .arg(Common::escapeQuotesHTML(plPtr->playlistName()))
                            .arg(playerControlHTML)
                            .arg(plPtr->key().toString())
                        ;
                        qDebug() << SB_DEBUG_INFO << row;
                        table+=row;
                    }
                }
            }
            html.replace(playlists,table);

            //  Charts
            QMap<SBIDChartPerformancePtr, SBIDChartPtr> allCharts=sPtr->charts(Common::retrieve_qmap());
            table=QString();
            if(allCharts.count())
            {
                //  Need to reorder the list by chart position and then chart name.
                QMultiMap<int, SBIDChartPerformancePtr> mm;
                QMapIterator<SBIDChartPerformancePtr, SBIDChartPtr> it(allCharts);
                while(it.hasNext())
                {
                    it.next();
                    SBIDChartPerformancePtr cpPtr=it.key();

                    if(cpPtr)
                    {
                        mm.insert(cpPtr->chartPosition(),cpPtr);
                    }
                }

                //  Create html
                table=QString("<TR><TD colspan=\"5\" class=\"SBItemSection\">Playlists:</TD></TR>");
                QMultiMapIterator<int,SBIDChartPerformancePtr> mmIT(mm);
                while(mmIT.hasNext())
                {
                    mmIT.next();

                    SBIDChartPerformancePtr cpPtr=mmIT.value();
                    SBIDChartPtr cPtr=allCharts[cpPtr];

                    if(cpPtr && cPtr)
                    {
                        int chartPosition=cpPtr->chartPosition();
                        QString iconLocation;
                        SBKey performerKey;
                        QString performerName;

                        SBIDSongPerformancePtr spPtr=cpPtr->songPerformancePtr();
                        if(spPtr)
                        {
                            SBIDOnlinePerformancePtr opPtr=spPtr->preferredOnlinePerformancePtr();
                            iconLocation=SBHtmlSongsAll::_getIconLocation(opPtr,SBKey::ChartPerformance);
                            performerKey=spPtr->songPerformerKey();
                            performerName=spPtr->songPerformerName();
                        }
                        if(!iconLocation.size())
                        {
                            iconLocation=ExternalData::getDefaultIconPath(SBKey::ChartPerformance);
                        }

                        QString  playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%1');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                        .arg(cPtr->key().toString());
                            ;
                        QString row=QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" >"
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBIconCell\" >"
                                    "%5"
                                "</TD>"
                                "<TD class=\"SBItemMajorSplit\" onclick=\"open_page('%6','%7');\">%7</TD>"
                                "<TD class=\"SBItemMajorSplit\" onclick=\"open_page('%4','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\" >"
                                    "%3"
                                "</TD>"
                            "</TR>"
                        )
                            .arg(iconLocation)
                            .arg(Common::escapeQuotesHTML(cPtr->chartName()))
                            .arg(playerControlHTML)
                            .arg(cPtr->key().toString())
                            .arg(chartPosition)
                            .arg(performerKey.toString())
                            .arg(Common::escapeQuotesHTML(performerName))
                        ;
                        table+=row;
                    }
                }
            }
            html.replace(charts,table);

            //  Lyrics
            table=QString();
            const QString songLyrics=sPtr->lyrics().replace("\n","<BR>");
            if(songLyrics.size())
            {
                table=QString("<TR><TD colspan=\"5\"><P class=\"SBItemSection\">Lyrics:</P></TD></TR>"
                              "<TR><TD colspan=\"5\"><P class=\"SBLyrics\">%1              </P></TD></TR>").arg(songLyrics);
            }
            html.replace(lyrics,table);
        }
    }
    return html;
}

QString
SBHtmlSongsAll::retrieveAllSongs(const QChar& startsWith, qsizetype offset, qsizetype size)
{
    QString table;

    //  Let's retrieve size+1 songs to see if there is anything left after the
    //  current batch.I
    SBSqlQueryModel* sm=SBIDSong::retrieveAllSongs(startsWith, offset, size+1);

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
        const SBKey songKey(sm->record(i).value(1).toByteArray());
        const SBKey performerKey(sm->record(i).value(3).toByteArray());
        SBIDSongPtr sPtr=SBIDSong::retrieveSong(songKey);

        if(sPtr)
        {

            //  Find out if we have the entry with original performer as the query
            //  includes everything.
            if(performerKey==sPtr->songOriginalPerformerKey())
            {
                //	Find icon to display
                QString iconLocation;
                SBKey iconKey;

                //	Find icon. If found pass corresponding SBKey as /icon/<sbkey value>.
                //  Otherwise pass default icon for songs.
                //  Find album icon first, if not exists, find performer icon.
                SBKey opKey;
                QMapIterator<int,SBIDOnlinePerformancePtr> it=sPtr->onlinePerformances();
                while(it.hasNext() && !iconLocation.size())
                {
                    it.next();
                    iconLocation=_getIconLocation(it.value(), SBKey::Performer);
                }
                if(!iconLocation.size())
                {
                    iconLocation=ExternalData::getDefaultIconPath(SBKey::Song);
                }

                //	Start table row
                const QString row=QString(
                    "<THEAD>"
                        "<TR>"
                            "<TD class=\"SBIconDiv\" rowspan=\"2\">"
                                "<img class=\"SBIcon\" src=\"%4\"></img>"
                            "</TD>"
                            "<TD class=\"SBItemMajor\" onclick=\"open_page('%2','%1');\">%1</TD>"
                            "<TD class=\"playercontrol_button\" rowspan=\"2\">"
                                "<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>"
                            "</TD>"
                        "</TR>"
                        "<TR>"
                            "<TD class=\"SBItemMinor\" onclick=\"open_page('%2','%1');\">%3</TD>"
                        "</TR>"
                    "</THEAD>"
                )
                    .arg(Common::escapeQuotesHTML(sPtr->songTitle()))
                    .arg(songKey.toString())
                    .arg(Common::escapeQuotesHTML(sPtr->commonPerformerName()))
                    .arg(iconLocation)
                ;
                table+=row;
            }
        }
    }
    table+=QString("<DIV id=\"sb_paging_prev_ind\"><P>%1</P></DIV><DIV id=\"sb_paging_next_ind\"><P>%2</P></DIV>")
                 .arg(moreSongsPrev)
                 .arg(moreSongsNext);
    sm->deleteLater();
    return table;
}

QString
SBHtmlSongsAll::_getIconLocation(const SBIDOnlinePerformancePtr& opPtr, const SBKey::ItemType& defaultType)
{
    QString iconLocation;
    SB_RETURN_IF_NULL(opPtr,ExternalData::getDefaultIconPath(defaultType));
    SBIDAlbumPtr aPtr=opPtr->albumPtr();
    return SBHtmlAlbumsAll::_getIconLocation(aPtr,defaultType);
}
