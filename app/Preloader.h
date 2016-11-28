#ifndef PRELOADER_H
#define PRELOADER_H

#include "SBIDBase.h"
#include "SBIDAlbumPerformance.h"

class Preloader
{
public:
    static QVector<SBIDAlbumPerformancePtr> performances(QString query, bool showProgressDialogFlag);
    static QMap<int,SBIDPtr> playlistItems(int playlistID,bool showProgressDialogFlag);
};

#endif // PRELOADER_H
