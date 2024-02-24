#include <QFile>


#include "ExternalData.h"
#include "SBSqlQueryModel.h"
#include "SBHtmlSongsAll.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"

SBHtmlSongsAll::SBHtmlSongsAll()
{
}


QString
SBHtmlSongsAll::retrieveAllSongs(const QChar& startsWith)
{
    const static QString defaultIconPath("/images/SongIcon.png");
    QString table;
    SBSqlQueryModel* sm=SBIDSong::retrieveAllSongs(startsWith);

    for(int i=0;i<sm->rowCount();i++)
    {
        const SBKey sKey(sm->record(i).value(1).toByteArray());
        SBIDSongPtr sPtr=SBIDSong::retrieveSong(sKey);
        if(sPtr)
        {
            //	Find icon to display
            QString iconLocation;
            SBKey iconKey;

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
            const QString row=QString(
                "<THEAD>\n\t"
                    "<TR>\n\t\t"
                        "<TD class=\"SBIconDiv\" rowspan=\"2\">"
                            "<img class=\"SBIcon\" src=\"%4\"></img>\n\t\t\t"
                        "</TD>\n\t\t\t"
                        "<TD class=\"SBItem\">%1</td>\n\t\t\t"
                        "<TD class=\"playercontrol_button\" rowspan=\"2\">\n\t\t\t"
                            "<IMG class=\"play_ctrl_button\" onclick=\"controlPlayer('play',%2)\"></img>\n\t\t\t\t"
                        "</TD>\n\t\t"
                    "</TR>\n\t\t"
                    "<TR>\n\t\t\t"
                        "<TD class=\"album\">%3</TD>"
                    "</tr>\n"
                "</THEAD>"
            )
                .arg(sPtr->songTitle())
                .arg(sPtr->key().toString())
                .arg(sPtr->commonPerformerName())
                .arg(iconLocation)
            ;

            table+=row;

        }
    }
    return table;
}
