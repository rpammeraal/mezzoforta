#ifndef PRELOADER_H
#define PRELOADER_H

#include "SBIDBase.h"

class Preloader
{
public:
    static QMap<int,SBIDPtr> playlistItems(int playlistID,bool showProgressDialogFlag);
};

#endif // PRELOADER_H
