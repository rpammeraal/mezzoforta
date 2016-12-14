#ifndef MUSICLIBRARY_H
#define MUSICLIBRARY_H

#include <QObject>
#include <QVector>

#include "Common.h"
#include "Duration.h"
#include "SBIDAlbum.h"
#include "SBIDSong.h"

class MusicLibrary : public QObject
{
    Q_OBJECT

    //	Represents entry as it already exists in the database
    class MLperformance
    {
    public:
        MLperformance() { _init(); }

        int songID;
        int performerID;
        int albumID;
        int albumPosition;
        QString path;
        bool pathExists;
    private:
        void _init() { pathExists=0; }
    };
    typedef std::shared_ptr<MLperformance> MLperformancePtr;

    //	A single class that does it all
    class MLentity
    {
    public:
        MLentity() { _init(); }

        //	File attributes
        QString filePath;
        QString parentDirectoryName;
        QString parentDirectoryPath;
        QString extension;

        //	Primary meta data attributes (need to exist)
        int albumPosition;
        QString albumTitle;
        QString songPerformerName;
        QString songTitle;

        //	Secondary meta data attributes (optional)
        QString albumPerformerName;
        Duration duration;
        QString genre;
        QString notes;
        int year;

        //	Songbase ids
        int songID;
        int songPerformerID;
        int albumID;
        int albumPerformerID;

        //	Helper attributes
        bool createArtificialAlbumFlag;
        QString errorMsg;

        inline bool errorFlag() const { return errorMsg.length()>0?1:0; }
        inline bool isValid() const
        {
            return
                (
                    errorFlag()==0  &&
                    (albumPosition>=0 || createArtificialAlbumFlag==1) &&
                    albumTitle.length()>0 &&
                    songPerformerName.length()>0 &&
                    songTitle.length()>0
                )?1:0;
        }

    private:
        void _init() { songID=-1; songPerformerID=-1; albumID=-1; albumPosition=-1; albumPerformerID=-1; createArtificialAlbumFlag=0; }
    };
    typedef std::shared_ptr<MLentity> MLentityPtr;

    class MLalbumPath
    {
    public:
        MLalbumPath() { _init(); }

        int maxPosition;
        QVector<QString> uniqueAlbumTitles;

    private:
        void _init() { maxPosition=0; }

    };
    typedef std::shared_ptr<MLalbumPath> MLalbumPathPtr;

public:
    explicit MusicLibrary(QObject *parent = 0);
    void rescanMusicLibrary();

signals:

public slots:

private:
};

#endif // MUSICLIBRARY_H
