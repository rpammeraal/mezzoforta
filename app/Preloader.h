#ifndef PRELOADER_H
#define PRELOADER_H

#include "SBIDBase.h"
#include "SBIDPerformance.h"

class Preloader
{
public:
    static QVector<SBIDPerformancePtr> performances(QString query, bool showProgressDialogFlag);
    static QMap<int,SBIDPtr> playlistItems(int playlistID,bool showProgressDialogFlag);
};

#endif // PRELOADER_H
