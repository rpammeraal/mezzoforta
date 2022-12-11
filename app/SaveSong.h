#ifndef SAVESONG_H
#define SAVESONG_H

#include "SBIDSong.h"

class SaveSong
{
public:
    SaveSong();
    int save(SBIDSongPtr orgSongPtr, const QString& editTitle, const QString& editPerformerName, int editYearOfRelease, const QString& editNotes, const QString& editLyrics, QString& updateText);
};

#endif // SAVESONG_H
