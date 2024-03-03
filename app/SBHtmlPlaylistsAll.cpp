#include <QString>

#include "ExternalData.h"
#include "SBHtmlPlaylistsAll.h"
#include "SBIDPlaylist.h"
#include "SBSqlQueryModel.h"

SBHtmlPlaylistsAll::SBHtmlPlaylistsAll() {}

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
                        "<TD class=\"SBItemMajor\" onclick=\"open_page('playlist_detail','%2','%1');\">%1</TD>"
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

