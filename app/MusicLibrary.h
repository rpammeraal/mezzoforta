#ifndef MUSICLIBRARY_H
#define MUSICLIBRARY_H

#include <QObject>
#include <QVector>

#include "Common.h"
#include "Duration.h"

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
        int songPerformerID;
        int albumPerformerID;
        int albumID;
        int songID;

    private:
        void _init() { songPerformerID=-1; albumPerformerID=-1;  albumPosition=-1; albumID=-1; songID=-1; }
    };

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

    class MLalbum
    {
    public:
        MLalbum() { _init() ; }

        int albumID;
        int albumPerformerID;
        QString path;
        QString title;

        QVector<QString> paths;
    private:
        void _init() { albumID=-1; albumPerformerID=-1; }
    };

    class MLsong
    {
    public:
        MLsong() { _init(); }
        int songID;
        int performerID;
        QString songTitle;
        QString songPerformerName;

        QString key() const { return QString("%1:%2").arg(Common::simplified(songTitle)).arg(performerID); }
        QVector<QString> paths;
    private:
        void _init() { songID=-1; performerID=-1; }
    };

    typedef std::shared_ptr<MLperformance> MLperformancePtr;
    typedef std::shared_ptr<MLentry> MLentryPtr;
    typedef std::shared_ptr<MLperformer> MLperformerPtr;
    typedef std::shared_ptr<MLalbum> MLalbumPtr;
    typedef std::shared_ptr<MLsong> MLsongPtr;

public:
    explicit MusicLibrary(QObject *parent = 0);
    void rescanMusicLibrary();

signals:

public slots:

private:
    void _rescanMusicLibrary(const QString& schema);
};

#endif // MUSICLIBRARY_H
