#include "ExternalData.h"
#include "SBSqlQueryModel.h"
#include "SBHtmlSongsAll.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDSong.h"

SBHtmlSongsAll::SBHtmlSongsAll()
{
}


QString
SBHtmlSongsAll::retrieveAllSongs(const QChar& startsWith)
{
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
            QString prefix("");

            //	Find icon and
            SBKey opKey;
            QMapIterator<int,SBIDOnlinePerformancePtr> it=sPtr->onlinePerformances();
            while(it.hasNext() && !opKey.validFlag())
            {
                it.next();
                const SBIDOnlinePerformancePtr opPtr=it.value();
                if(opPtr)
                {
                    opKey=opPtr->key();
                    SBKey albumKey=opPtr->albumKey();
                    //	iconLocation=ExternalData::getCachePath(albumKey);	Need to fix calls to API first
                    //	prefix=QString("cache/");
                }
            }
            if(iconLocation.size()==0)
            {
                //	Retrieve std song icon
                iconLocation=SBIDSong::iconResourceLocationStatic().mid(2);
            }

            //	Start table row
            const QString row=QString(
                "<THEAD>\n\t"
                    "<TR>\n\t\t"
                        "<TD class=\"SongListIcon\" rowspan=\"2\">"
                            "<img class=\"icon\"></img>\n\t\t\t"
                        "</TD>\n\t\t\t"
                        "<TD class=\"song\">%1</td>\n\t\t\t"
                        "<TD class=\"SongListPlayButton\" rowspan=\"2\">\n\t\t\t"
                            "<IMG class=\"play_ctrl_button\" onclick=\"controlPlayer('play',%2)\"></img>\n\t\t\t\t"
                        "</TD>\n\t\t"
                    "</TR>\n\t\t"
                    "<TR>\n\t\t\t"
                        "<TD class=\"album\">%3</TD>"
                    "</tr>\n"
                "</THEAD>"
            )
                .arg(sPtr->songTitle())
                .arg(opKey.toString())
                .arg(sPtr->commonPerformerName())
            ;

            table+=row;

        }
    }

    //	Alphabet
    // table+=QString("<TD class=\"alphabet_array\"><TABLE class=\"alphabet_array\"><TBODY>");
    // for(int a='A';a<='Z';a++)
    // {
    //     table+=QString("<TR class=\"alpabet_array\" ><TD><A HREF=\"/song_detail.html?start=%1\">%1</A></TD></TR>").arg(QChar(a));
    // }
    // table+=QString("</TBODY></TABLE>");
    // table+=QString("</TD></TR>");
    // table+=QString("</TBODY></TABLE>");

    return table;
}
