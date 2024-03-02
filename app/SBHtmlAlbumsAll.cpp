#include <QFile>



#include "ExternalData.h"
#include "SBHtmlAlbumsAll.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBSqlQueryModel.h"

SBHtmlAlbumsAll::SBHtmlAlbumsAll() {}

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
        availableCount=size;
    }

    qDebug() << SB_DEBUG_INFO << availableCount << size;
    for(int i=0;i<size;i++)
    {
        const SBKey albumKey(sm->record(i).value(0).toByteArray());
        const SBKey performerKey(sm->record(i).value(1).toByteArray());
        SBIDAlbumPtr aPtr=SBIDAlbum::retrieveAlbum(albumKey);
        qDebug() << SB_DEBUG_INFO << albumKey << sm->record(i).value(0).toByteArray();

        if(aPtr)
        {
            qDebug() << SB_DEBUG_INFO << aPtr->genericDescription();
            //	Find icon to display
            QString iconLocation;
            SBKey iconKey;

            iconLocation=_getIconLocation(aPtr);

            //	Start table row
            const QString albumTitle=aPtr->albumTitle().replace("'","&rsquo;").replace("\"","&quot;");
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
                .arg(albumTitle)
                .arg(albumKey.toString())
                .arg(aPtr->albumPerformerName())
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
