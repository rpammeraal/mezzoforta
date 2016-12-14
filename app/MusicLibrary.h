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















    //	Represents what has been found on the file system
    class MLentry
    {
    public:
        MLentry() { _init(); }

        //	Found at following path
        QString path;

        //	Retrieved from meta data from file at path
        QString albumTitle;
        int albumPosition;
        Duration duration;
        QString genre;
        QString notes;
        QString songPerformerName;
        QString songTitle;
        int year;

        //	Calculated attributes
        QString dirName;

        //	keys assigned
        int albumPerformerID;	//	only used to determine albumID

        //	Ptrs assigned
        SBIDAlbumPtr albumPtr;
        SBIDPerformerPtr performerPtr;
        SBIDSongPtr songPtr;

        bool validFlag() const { return albumTitle.length()>0 && (albumPosition==-2 || albumPosition>0) && songPerformerName.length()>0 && songTitle.length()>0 ? 1: 0; }
    private:
        void _init() { albumPerformerID=-1;  albumPosition=-1; albumPtr=SBIDAlbumPtr(); performerPtr=SBIDPerformerPtr(); songPtr=SBIDSongPtr(); }
    };
    typedef std::shared_ptr<MLentry> MLentryPtr;







    class MLperformer
    {
    public:
        MLperformer() { _init(); }

        QString name;
        int performerID;
        QVector<QString> paths;

    private:
        void _init() { performerID=-1; }
    };
    typedef std::shared_ptr<MLperformer> MLperformerPtr;

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

    class MLalbum
    {
    public:
        MLalbum() { _init() ; }

        int albumID;
        int albumPerformerID;
        QString path;
        QString title;
        int year;
        QString genre;

        int offset; //	if there any existing performances on the album, need to add offset to albumPosition
        bool artificiallyCreatedFlag;
        int maxPosition;

        QVector<QString> paths;

        SBIDAlbumPtr albumPtr;
    private:
        void _init() { albumID=-1; albumPerformerID=-1; offset=0; year=1900; artificiallyCreatedFlag=1; maxPosition=-1; albumPtr=SBIDAlbumPtr(); }
    };
    typedef std::shared_ptr<MLalbum> MLalbumPtr;

    class MLsongPerformance
    {
    public:
        MLsongPerformance() { _init(); }
        int songID;
        int performerID;
        QString songTitle;
        QString songPerformerName;
        int year;
        QString notes;

        QString key() const { return QString("%1:%2").arg(Common::simplified(songTitle)).arg(performerID); }
        QVector<QString> paths;
    private:
        void _init() { songID=-1; performerID=-1; }
    };
    typedef std::shared_ptr<MLsongPerformance> MLsongPerformancePtr;

public:
    explicit MusicLibrary(QObject *parent = 0);
    void rescanMusicLibrary();
    void rescanMusicLibrary_v2();

signals:

public slots:

private:
};

#endif // MUSICLIBRARY_H
