#include <QFile>

#include <QVectorIterator>

#include "ExternalData.h"
#include "SBSqlQueryModel.h"
#include "SBHtmlSongsAll.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDAlbum.h"
#include "SBIDChart.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"

SBHtmlSongsAll::SBHtmlSongsAll()
{
}

static const QString orgPerformer=QString("___SB_ORIGINAL_PERFORMER___");
static const QString otherPerformers=QString("___SB_OTHER_PERFORMERS___");
static const QString albums=QString("___SB_ALBUMS___");
static const QString playlists=QString("___SB_PLAYLISTS___");
static const QString charts=QString("___SB_CHARTS___");

QString
SBHtmlSongsAll::songDetail(QString html, const QString& key)
{
    QString contents;
    qDebug() << SB_DEBUG_INFO << key;

    SBKey songKey=SBKey(key.toLatin1());

    if(songKey.validFlag())
    {
        SBIDSongPtr sPtr=SBIDSong::retrieveSong(songKey);

        if(sPtr)
        {
            QString table;

            //  Original performer
            SBKey originalPerformerKey=sPtr->songOriginalPerformerKey();
            table=QString("<TR><TD class=\"SBItemMajorTitleOnly\">Original Performer</TD></TR>"
                          "<TR><TD><LI><A class=\"SBItemMinor\">%1</A></LI></TD></TR>")
                        .arg(sPtr->songOriginalPerformerName());
            html.replace(orgPerformer,table);

            //  Other performers
            QMap<int,SBIDSongPerformancePtr> allPerformances=sPtr->songPerformances();
            table=QString();
            if(allPerformances.count())
            {
                qsizetype count=0;
                QMapIterator<int,SBIDSongPerformancePtr> it(allPerformances);
                while(it.hasNext())
                {
                    it.next();
                    SBIDSongPerformancePtr spPtr=it.value();
                    if(spPtr)
                    {
                        if(originalPerformerKey!=spPtr->songPerformerKey())
                        {
                            qDebug() << SB_DEBUG_INFO << originalPerformerKey << spPtr->songPerformerKey();
                            //  Avoid original performer
                            table+=QString("<LI><A class=\"SBItemMinor\">%1</A></LI>").arg(spPtr->songPerformerName());
                            count++;
                        }
                    }
                }
                if(count)
                {
                    table=QString("<TR><TD class=\"SBItemMajorTitleOnly\">Also performed by:</TD></TR><TR><TD>")
                            +table;
                }
            }
            html.replace(otherPerformers,table);

            //  Albums
            QVector<SBIDAlbumPerformancePtr> allAlbums=sPtr->allPerformances();
            table=QString();
            if(allAlbums.count())
            {
                table=QString("<TR><TD class=\"SBItemMajorTitleOnly\">Albums:</TD></TR><TR><TD>");
                QVectorIterator<SBIDAlbumPerformancePtr> it(allAlbums);
                while(it.hasNext())
                {
                    SBIDAlbumPerformancePtr apPtr=it.next();
                    if(apPtr)
                    {
                        table+=QString("<LI><A class=\"SBItemMinor\">%1</A></LI>").arg(apPtr->albumTitle());
                    }
                }
            }
            html.replace(albums,table);

            //  Playlists
            QVector<SBIDSong::PlaylistOnlinePerformance> allPlaylists=sPtr->playlists(SBIDSong::retrieve_qvector());
            table=QString();
            if(allPlaylists.count())
            {
                table=QString("<TR><TD>Playlists:</TD></TR><TR><TD class=\"SBItemMinor\">");
                QVectorIterator<SBIDSong::PlaylistOnlinePerformance> it(allPlaylists);
                while(it.hasNext())
                {
                    SBIDSong::PlaylistOnlinePerformance pop=it.next();
                    SBIDPlaylistPtr plPtr=pop.plPtr;

                    if(plPtr)
                    {
                        table+=QString("<LI><A class=\"SBItemMinor\">%1</A></LI>").arg(plPtr->playlistName());
                    }
                }
            }
            html.replace(playlists,table);

            //  Charts
            QMap<SBIDChartPerformancePtr, SBIDChartPtr> allCharts=sPtr->charts(SBIDSong::retrieve_qmap());
            qDebug() << SB_DEBUG_INFO;
            table=QString();
            if(allCharts.count())
            {
                table=QString("<TR><TD class=\"SBItemMajorTitleOnly\">Charts:</TD></TR><TR><TD>");
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
            }
            qDebug() << SB_DEBUG_INFO << table;
            qDebug() << SB_DEBUG_INFO << charts;
            qDebug() << SB_DEBUG_INFO << html;
            html.replace(charts,table);
            qDebug() << SB_DEBUG_INFO << html;
        }
    }
    return html;
}

QString
SBHtmlSongsAll::retrieveAllSongs(const QChar& startsWith)
{
    const static QString defaultIconPath("/images/SongIcon.png");
    QString table;
    SBSqlQueryModel* sm=SBIDSong::retrieveAllSongs(startsWith); //  this give us all songs. Only want songs with org performer

    for(int i=0;i<sm->rowCount();i++)
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
                qDebug() << SB_DEBUG_INFO << sPtr->key() << sPtr->genericDescription();

                //	Find icon. If found pass corresponding SBKey as /icon/<sbkey value>.
                //  Otherwise pass default icon for songs.
                //  Find album icon first, if not exists, find performer icon.
                SBKey opKey;
                QMapIterator<int,SBIDOnlinePerformancePtr> it=sPtr->onlinePerformances();
                while(it.hasNext() && !opKey.validFlag())
                {
                    it.next();
                    const SBIDOnlinePerformancePtr opPtr=it.value();
                    if(opPtr)
                    {
                        opKey=opPtr->key();
                        SBIDAlbumPtr aPtr=opPtr->albumPtr();
                        if(aPtr)
                        {
                            const SBKey albumKey=aPtr->key();
                            iconLocation=ExternalData::getCachePath(albumKey);
                            if(QFile::exists(iconLocation))
                            {
                                iconKey=albumKey;
                            }
                            else
                            {
                                //  Try performer

                                SBIDPerformerPtr pPtr=aPtr->albumPerformerPtr();
                                if(pPtr)
                                {
                                    const SBKey performerKey=pPtr->key();
                                    iconLocation=ExternalData::getCachePath(performerKey);

                                    if(QFile::exists(iconLocation))
                                    {
                                        iconKey=performerKey;
                                    }
                                }
                            }
                        }
                    }
                }
                if(!iconKey.validFlag())
                {
                    //	Retrieve std song icon
                    iconLocation=defaultIconPath;
                }
                else
                {
                    iconLocation=QString("/icon/%1").arg(iconKey.toString());
                }

                //	Start table row
                const QString songTitle=sPtr->songTitle().replace("'","&rsquo;").replace("\"","&quot;");
                const QString row=QString(
                    "<THEAD>"
                        "<TR>"
                            "<TD class=\"SBIconDiv\" rowspan=\"2\">"
                                "<img class=\"SBIcon\" src=\"%4\"></img>"
                            "</TD>"
                            "<TD class=\"SBItemMajor\" onclick=\"open_page('song_detail','%2','%1');\">%1</TD>"
                            "<TD class=\"playercontrol_button\" rowspan=\"2\">"
                                "<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>"
                            "</TD>"
                        "</TR>"
                        "<TR>"
                            "<TD class=\"SBItemMinor\" onclick=\"open_page('song_detail','%2','%1');\">%3</TD>"
                        "</TR>"
                    "</THEAD>"
                )
                    .arg(songTitle)
                    .arg(songKey.toString())
                    .arg(sPtr->commonPerformerName())
                    .arg(iconLocation)
                ;
                table+=row;
                qDebug() << SB_DEBUG_INFO << row;
            }
        }
    }
    return table;
}
