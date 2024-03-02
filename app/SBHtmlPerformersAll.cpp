#include <QFile>
#include "ExternalData.h"
#include "SBHtmlPerformersAll.h"

SBHtmlPerformersAll::SBHtmlPerformersAll() {}

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
