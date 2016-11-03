#ifndef METADATA_H
#define METADATA_H

#include <taglib/fileref.h>

#include <QString>

#include "SBIDSong.h"

class MetaData
{
public:
    MetaData(const QString& path);
    ~MetaData();

    SBIDSongPtr parse();

private:
    TagLib::FileRef _f;
};

#endif // METADATA_H
