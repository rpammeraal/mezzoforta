#include <QFile>



#include "ExternalData.h"
#include "SBHtmlAlbumsAll.h"
#include "SBHtmlPerformersAll.h"
#include "SBIDAlbum.h"
#include "SBIDAlbumPerformance.h"
#include "SBIDBase.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDPerformer.h"
#include "SBSqlQueryModel.h"

static const QString performer=QString("___SB_PERFORMER___");
static const QString songs=QString("___SB_SONGS___");

SBHtmlAlbumsAll::SBHtmlAlbumsAll() {}

QString
SBHtmlAlbumsAll::albumDetail(QString html, const QString& key)
{
    QString contents;
    html.replace('\n',"");
    html.replace('\t'," ");

    SBKey albumKey=SBKey(key.toLatin1());

    if(albumKey.validFlag())
    {
        SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(albumKey);

        if(aPtr)
        {
            QString table;

            SBIDPerformerPtr pPtr=aPtr->albumPerformerPtr();

            if(pPtr)
            {
                QString playerControlHTML;

                if(pPtr->itemID()!=SBIDPerformer::retrieveVariousPerformers()->itemID())
                {
                    playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%1');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                            .arg(pPtr->key().toString());

                }
                //  Performer
                table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Performed By:</P></TD></TR>")+
                                QString(
                                    "<TR>"
                                        "<TD class=\"SBIconCell\" >"
                                            "<img class=\"SBIcon\" src=\"%1\"></img>"
                                        "</TD>"
                                        "<TD class=\"SBItemMajor\" >%2</TD>"
                                        "<TD class=\"playercontrol_button\" >"


                                        "</TD>"
                                    "</TR>"
                                    )
                                    .arg(SBHtmlPerformersAll::_getIconLocation(pPtr))
                                    .arg(Common::escapeQuotesHTML(pPtr->performerName()))
                    ;
            }
            html.replace(performer,table);

            //  Songs

            //  Create list of song instances (e.g. all instances on an album)
            QMap<int, SBIDAlbumPerformancePtr> allAlbumPerformances=aPtr->albumPerformances();
            table=QString();
            int numPlayables=0;

            if(allAlbumPerformances.count())
            {
                QMapIterator<int, SBIDAlbumPerformancePtr> apIt(allAlbumPerformances);
                //  Remap so we can display songs in order of appearance on album
                QMap<qsizetype,qsizetype> albumOrderMap;

                while(apIt.hasNext())
                {
                    apIt.next();
                    const SBIDAlbumPerformancePtr apPtr=apIt.value();
                    if(apPtr)
                    {
                        albumOrderMap[apPtr->albumPosition()]=apIt.key();
                    }
                }

                for(qsizetype i=0; i<albumOrderMap.size();i++)
                {
                    const SBIDAlbumPerformancePtr apPtr=allAlbumPerformances.value(albumOrderMap[i+1]);
                    if(apPtr)
                    {
                        QString playerControlHTML;
                        SBIDOnlinePerformancePtr opPtr=apPtr->preferredOnlinePerformancePtr();
                        if(opPtr)
                        {
                            playerControlHTML=QString("<P class=\"item_play_button\" onclick=\"control_player('play','%1');\"><BUTTON type=\"button\">&gt;</BUTTON></P>")
                                                    .arg(opPtr->key().toString())
                            ;
                            numPlayables++;
                        }

                        QString row=QString(
                            "<TR>"
                                "<TD class=\"SBIconCell\" rowspan=\"2\">"
                                    "<img class=\"SBIcon\" src=\"%1\"></img>"
                                "</TD>"
                                "<TD class=\"SBItemMajor\"  onclick=\"open_page('song_detail','%5','%2');\">%2</TD>"
                                "<TD class=\"playercontrol_button\" rowspan=\"2\">"
                                    "%3"
                                "</TD>"
                            "</TR>"
                            "<TR>"
                                "<TD pos=\"84\" class=\"SBItemMinor\" onclick=\"open_page('song_detail','%5','%2');\">%4</TD>"
                            "</TR>"
                        )
                            .arg(ExternalData::getDefaultIconPath())
                            .arg(Common::escapeQuotesHTML(apPtr->songTitle()))
                            .arg(playerControlHTML)
                            .arg(Common::escapeQuotesHTML(apPtr->songPerformerName()))
                            .arg(apPtr->songKey().toString())
                        ;
                        table+=row;
                    }
                }
            }
            if(numPlayables)
            {
                table=QString("<TR><TD colspan=\"3\"><P class=\"SBItemSection\">Contains:</P></TD></TR>")+table;
            }
            html.replace(songs,table);
        }
    }
    return html;
}


QString
SBHtmlAlbumsAll::retrieveAllAlbums(const QChar& startsWith, qsizetype offset, qsizetype size)
{
    QString table;

    //  Let's retrieve size+1 albums to see if there is anything left after the
    //  current batch.I
    SBSqlQueryModel* sm=SBIDAlbum::allAlbums(startsWith, offset, size+1);

    bool moreAlbumsNext=0;
    bool moreAlbumsPrev=(offset>0)?1:0;

    qsizetype availableCount=sm->rowCount();
    if(availableCount>size)
    {
        moreAlbumsNext=1;
    }

    for(int i=0;i<size;i++)
    {
        const SBKey albumKey(sm->record(i).value(0).toByteArray());
        const SBKey performerKey(sm->record(i).value(1).toByteArray());
        SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(albumKey);

        if(aPtr)
        {
            //	Find icon to display
            QString iconLocation;
            SBKey iconKey;

            iconLocation=_getIconLocation(aPtr);

            //	Start table row
            const QString row=QString(
                "<THEAD>"
                    "<TR>"
                        "<TD class=\"SBIconDiv\" rowspan=\"2\">"
                            "<img class=\"SBIcon\" src=\"%4\"></img>"
                        "</TD>"
                        "<TD class=\"SBItemMajor\" onclick=\"open_page('album_detail','%2','%1');\">%1</TD>"
                        "<TD class=\"playercontrol_button\" rowspan=\"2\">"
                            "<P class=\"item_play_button\" onclick=\"control_player('play','%2');\"><BUTTON type=\"button\">&gt;</BUTTON></P>"
                        "</TD>"
                    "</TR>"
                    "<TR>"
                        "<TD class=\"SBItemMinor\" onclick=\"open_page('album_detail','%2','%1');\">%3</TD>"
                    "</TR>"
                "</THEAD>"
            )
                .arg(Common::escapeQuotesHTML(aPtr->albumTitle()))
                .arg(albumKey.toString())
                .arg(Common::escapeQuotesHTML(aPtr->albumPerformerName()))
                .arg(iconLocation)
            ;
            table+=row;
        }
    }
    table+=QString("<DIV id=\"sb_paging_prev_ind\"><P>%1</P></DIV><DIV id=\"sb_paging_next_ind\"><P>%2</P></DIV>")
                 .arg(moreAlbumsPrev)
                 .arg(moreAlbumsNext);
    return table;
}


QString
SBHtmlAlbumsAll::_getIconLocation(const SBIDAlbumPtr& aPtr)
{
    SB_RETURN_IF_NULL(aPtr,ExternalData::getDefaultIconPath());
    QString iconLocation;
    SBKey iconKey;

    const SBKey albumKey=aPtr->key();
    iconLocation=ExternalData::getCachePath(albumKey);
    if(QFile::exists(iconLocation))
    {
        iconKey=albumKey;
    }
    else
    {
        SBIDPerformerPtr pPtr=aPtr->albumPerformerPtr();
        iconLocation=SBHtmlPerformersAll::_getIconLocation(pPtr);
    }

    if(!iconKey.validFlag())
    {
        //	Retrieve std song icon
        iconLocation=ExternalData::getDefaultIconPath();
    }
    else
    {
        iconLocation=QString("/icon/%1").arg(iconKey.toString());
    }
    return iconLocation;
}
