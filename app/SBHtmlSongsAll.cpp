#include <QFile>
#include <QString>
#include <QVectorIterator>

#include "ExternalData.h"
#include "SBSqlQueryModel.h"
#include "SBHtmlSongsAll.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDAlbum.h"
#include "SBIDChart.h"
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
            int numPlayables=0;

            if(allAlbumPerformances.count())
            {
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
                            iconLocation=_getIconLocation(opPtr);
                            playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                    .arg(opPtr->key().toString())
                            ;
                            numPlayables++;
                        }
                        else
                        {
                            iconLocation=ExternalData::getDefaultIconPath();
                            playerControlHTML=empty;
                        }

                        QString row=QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" rowspan=\"2\">"
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBItemMajor\"  onclick=\"open_page('%5','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\" rowspan=\"2\">"
                                    "%3"
                                "</TD>"
                            "</TR>"
                            "<TR>"
                                "<TD pos=\"84\" class=\"SBItemMinor\" >%4</TD>"
                            "</TR>"
                        )
                            .arg(iconLocation)
                            .arg(Common::escapeQuotesHTML(opPtr->albumTitle()))
                            .arg(playerControlHTML)
                            .arg(Common::escapeQuotesHTML(opPtr->songPerformerName()))
                            .arg(apPtr->albumKey().toString())
                        ;
                        qDebug() << SB_DEBUG_INFO << row;
                        table+=row;
                    }
                }
            }
            if(numPlayables)
            {
                table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Albums:</P></TD></TR>")+table;
            }
            html.replace(albums,table);

            //  Playlists
            QVector<SBIDSong::PlaylistOnlinePerformance> allPlaylists=sPtr->playlists(SBIDSong::retrieve_qvector());
            table=QString();
            if(allPlaylists.count())
            {
                table=QString("<TR><TD colspan=\"3\" class=\"SBItemSection\">Playlists:</TD></TR>");
                table+=QString("<TR><TD colspan=\"3\" class=\"SBItemMinor\">");
                QVectorIterator<SBIDSong::PlaylistOnlinePerformance> it(allPlaylists);
                while(it.hasNext())
                {
                    SBIDSong::PlaylistOnlinePerformance pop=it.next();
                    SBIDPlaylistPtr plPtr=pop.plPtr;

                    if(plPtr)
                    {
                        table+=QString("<LI><A class=\"SBItemMajor\">%1</A></LI>").arg(plPtr->playlistName());
                    }
                }
                table+=QString("</TD></TR>");
            }
            html.replace(playlists,table);

            //  Charts
            QMap<SBIDChartPerformancePtr, SBIDChartPtr> allCharts=sPtr->charts(SBIDSong::retrieve_qmap());
            table=QString();
            if(allCharts.count())
            {
                table=QString("<TR><TD colspan=\"3\" class=\"SBItemSection\">Charts:</TD></TR>");
                table+=QString("<TR><TD colspan=\"3\" class=\"SBItemMinor\">");
                QMapIterator<SBIDChartPerformancePtr, SBIDChartPtr> it(allCharts);
                while(it.hasNext())
                {
                    it.next();
                    SBIDChartPtr cPtr=it.value();
                    if(cPtr)
                    {
                        table+=QString("<LI><A class=\"SBItemMinor\">%1</A></LI>").arg(cPtr->chartName());
                    }
                }
                table+=QString("</TD></TR>");
            }
            html.replace(charts,table);

            //  Lyrics
            table=QString();
            const QString songLyrics=sPtr->lyrics().replace("\n","<BR>");
            if(songLyrics.size())
            {
                table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Lyrics:</P></TD></TR>"
                              "<TR><TD colspan=\"3\"><P class=\"SBLyrics\">%1              </P></TD></TR>").arg(songLyrics);
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
                    iconLocation=_getIconLocation(it.value());
                }
                if(!iconLocation.size())
                {
                    iconLocation=ExternalData::getDefaultIconPath();
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
SBHtmlSongsAll::_getIconLocation(const SBIDOnlinePerformancePtr& opPtr)
{
    QString iconLocation;
    SB_RETURN_IF_NULL(opPtr,ExternalData::getDefaultIconPath());
    SBIDAlbumPtr aPtr=opPtr->albumPtr();
    return SBHtmlAlbumsAll::_getIconLocation(aPtr);
}
