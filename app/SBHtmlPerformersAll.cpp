#include <QFile>

#include "ExternalData.h"
#include "SBHtmlPerformersAll.h"
#include "SBSqlQueryModel.h"

SBHtmlPerformersAll::SBHtmlPerformersAll() {}

QString
SBHtmlPerformersAll::retrieveAllPerformers(const QChar& startsWith, qsizetype offset, qsizetype size)
{
    QString table;

    //  Let's retrieve size+1 songs to see if there is anything left after the
    //  current batch.I
    SBSqlQueryModel* sm=SBIDPerformer::retrieveAllPerformers(startsWith, offset, size+1);

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
        const SBKey performerKey(sm->record(i).value(0).toByteArray());
        SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(performerKey);

        if(pPtr)
        {
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
                .arg(Common::escapeQuotesHTML(pPtr->performerName()))
                .arg(pPtr->key().toString())
                .arg(_getIconLocation(pPtr))
            ;
            table+=row;
        }
    }
    table+=QString("<DIV id=\"sb_paging_prev_ind\"><P>%1</P></DIV><DIV id=\"sb_paging_next_ind\"><P>%2</P></DIV>")
                 .arg(moreSongsPrev)
                 .arg(moreSongsNext);
    sm->deleteLater();
    return table;
}

QString
SBHtmlPerformersAll::_getIconLocation(const SBIDPerformerPtr& pPtr)
{
    SB_RETURN_IF_NULL(pPtr,ExternalData::getDefaultIconPath());

    QString iconLocation;
    SBKey iconKey;

    const SBKey performerKey=pPtr->key();
    iconLocation=ExternalData::getCachePath(performerKey);

    if(QFile::exists(iconLocation))
    {
        iconKey=performerKey;
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
