#ifndef METADATA_H
#define METADATA_H

#include <taglib/fileref.h>

#include <QString>


#include "Duration.h"

class MetaData
{
public:
    MetaData(const QString& path);
    ~MetaData();

    inline QString albumTitle() const { return _albumTitle; }
    inline QString genre() const { return _genre; }
    inline QString notes() const { return _notes; }
    inline QString songPerformerName() const { return _songPerformerName; }
    inline QString songTitle() const { return _songTitle; }
    inline int albumPosition() const { return _albumPosition; }
    inline int year() const { return _year; }
    inline Duration duration() const { return _duration; }

private:
    //TagLib::FileRef _f;

    QString         _albumTitle;
    int             _albumPosition;
    Duration        _duration;
    QString         _genre;
    QString         _notes;
    QString         _songPerformerName;
    QString         _songTitle;
    int             _year;

    void _parse();
};

#endif // METADATA_H
