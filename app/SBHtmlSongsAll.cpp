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
    QString table("<TABLE><TBODY><TR><TD>\n");
    SBSqlQueryModel* sm=SBIDSong::retrieveAllSongs(startsWith);

    table+=QString("<TABLE><TBODY>");
    for(int i=0;i<sm->rowCount();i++)
    {
        const SBKey sKey(sm->record(i).value(1).toByteArray());
        SBIDSongPtr sPtr=SBIDSong::retrieveSong(sKey);
        if(sPtr)
        {
            //	Find icon to display
            QString row;
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
            row+=QString("<TR>");

            //	1st column: icon
            QString icon=QString("<TD><IMG src=\"%1/%2\" class=\"icon\"></IMG></TD>")
                               .arg(prefix)
                               .arg(iconLocation);
            row+=icon;

            //	2nd column: title and performer
            row+=QString("<TD><TABLE><TBODY><TR><TD class=\"song\">%1</TD></TR><TR><TD class=\"album\">%2</TD></TR></TBODY></TABLE></TD>")
                       .arg(sPtr->songTitle())
                       .arg(sPtr->commonPerformerName())
                ;

            //	3rd column: play button
            QString playbutton=QString("<TD><BUTTON class=\"play_ctrl_button\" onclick=\"controlPlayer('play','%1')\">&gt</BUTTON></TD>")
                               .arg(opKey.toString());
            row+=playbutton;

            //	End table row
            row+=QString("</TR>\n");

            table+=row;

        }
    }
    table+=QString("</TBODY></TABLE></TD>");

    //	Alphabet
    table+=QString("<TD class=\"alphabet_array\"><TABLE class=\"alphabet_array\"><TBODY>");
    for(int a='A';a<='Z';a++)
    {
        table+=QString("<TR class=\"alpabet_array\" ><TD><A HREF=\"/all_song.html?start=%1\">%1</A></TD></TR>").arg(QChar(a));
    }
    table+=QString("</TBODY></TABLE>");
    table+=QString("</TD></TR>");
    table+=QString("</TBODY></TABLE>");

    return table;
}
