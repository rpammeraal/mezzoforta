#include <QString>

#include "ExternalData.h"
#include "SBHtmlPlaylistsAll.h"
#include "SBIDPlaylist.h"
#include "SBIDOnlinePerformance.h"
#include "SBSqlQueryModel.h"

static const QString songs=QString("___SB_SONGS___");

SBHtmlPlaylistsAll::SBHtmlPlaylistsAll() {}

QString
SBHtmlPlaylistsAll::playlistDetail(QString html, const QString& key)
{
    QString contents;
    html.replace('\n',"");
    html.replace('\t'," ");

    SBKey playlistKey=SBKey(key.toLatin1());

    if(playlistKey.validFlag())
    {
        SBIDPlaylistPtr pPtr=SBIDPlaylist::retrievePlaylist(playlistKey);

        if(pPtr)
        {
            QString table;


            //  Items

            //  Create list of song instances (e.g. all instances on an album)
            QMap<int, SBIDPlaylistDetailPtr> allItems=pPtr->items();
            table=QString();
            int numItems=0;

            if(allItems.count())
            {
                QMapIterator<int, SBIDPlaylistDetailPtr> apIt(allItems);
                //  Remap so we can display songs in order of appearance on album
                QMap<qsizetype,qsizetype> itemOrderMap;

                while(apIt.hasNext())
                {
                    apIt.next();
                    const SBIDPlaylistDetailPtr pdPtr=apIt.value();
                    if(pdPtr)
                    {
                        itemOrderMap[pdPtr->playlistPosition()]=apIt.key();
                    }
                }

                for(qsizetype i=0; i<itemOrderMap.size();i++)
                {
                    const SBIDPlaylistDetailPtr pdPtr=allItems.value(itemOrderMap[i+1]);
                    if(pdPtr)
                    {
                        SBKey itemKey;


                        //  Handle type of playlist detail
                        qDebug() << SB_DEBUG_INFO << pdPtr->itemType();
                        if(pdPtr->consistOfItemType()==SBKey::OnlinePerformance)
                        {
                            const SBIDOnlinePerformancePtr opPtr=pdPtr->onlinePerformancePtr();
                            if(opPtr)
                            {
                                itemKey=opPtr->songKey();
                            }
                        }
                        else
                        {
                            itemKey=pdPtr->childKey();
                        }
                        qDebug() << SB_DEBUG_INFO << itemKey;


                        QString  playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%1');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                        .arg(pdPtr->key().toString());
                            ;
                            numItems++;

                        QString row=QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" "
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBItemMajor\"  onclick=\"open_page('%4','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\">"
                                    "%3"
                                "</TD>"
                            "</TR>"
                        )
                            .arg(ExternalData::getDefaultIconPath())
                            .arg(Common::escapeQuotesHTML(pdPtr->genericDescription()))
                            .arg(playerControlHTML)
                            .arg(itemKey.toString())
                        ;
                        qDebug() << SB_DEBUG_INFO << row;
                        table+=row;
                    }
                }
            }
            if(numItems)
            {
                table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Contains:</P></TD></TR>")+table;
            }
            html.replace(songs,table);
        }
    }
    return html;
}


QString
SBHtmlPlaylistsAll::retrieveAllPlaylists(const QChar& startsWith, qsizetype offset, qsizetype size)
{
    QString table;

    //  Let's retrieve size+1 albums to see if there is anything left after the
    //  current batch.I
    SBSqlQueryModel* qm=SBIDPlaylist::allPlaylists(startsWith, offset, size+1);

    bool moreNext=0;
    bool morePrev=(offset>0)?1:0;

    qsizetype availableCount=qm->rowCount();
    if(availableCount>size)
    {
        moreNext=1;
    }
    else if(size>availableCount)
    {
        size=availableCount;
    }

    for(int i=0;i<size;i++)
    {
        const SBKey playlistKey(qm->record(i).value(0).toByteArray());
        SBIDPlaylistPtr pPtr=SBIDPlaylist::retrievePlaylist(playlistKey);

        if(pPtr)
        {
            //	Find icon to display
            SBKey iconKey;

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
                .arg(Common::escapeQuotesHTML(pPtr->playlistName()))
                .arg(pPtr->key().toString())
                .arg(ExternalData::getDefaultIconPath())
            ;
            table+=row;
        }
    }
    table+=QString("<DIV id=\"sb_paging_prev_ind\"><P>%1</P></DIV><DIV id=\"sb_paging_next_ind\"><P>%2</P></DIV>")
                 .arg(morePrev)
                 .arg(moreNext);
    qm->deleteLater();
    return table;
}

