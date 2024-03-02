#ifndef SBHTMLPERFORMERSALL_H
#define SBHTMLPERFORMERSALL_H

#include <QString>

#include "SBIDPerformer.h"

class SBHtmlPerformersAll
{
public:
    SBHtmlPerformersAll();

protected:
    friend class SBHtmlAlbumsAll;

    static QString _getIconLocation(const SBIDPerformerPtr& pPtr);
};

#endif // SBHTMLPERFORMERSALL_H
