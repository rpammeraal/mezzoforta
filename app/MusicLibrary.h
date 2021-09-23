#ifndef MUSICLIBRARY_H
#define MUSICLIBRARY_H

#include <memory>

#include <QObject>
#include <QVector>

#include "Common.h"
#include "SBDuration.h"

class QProgressDialog;
class DataAccessLayer;

class MusicLibrary : public QObject
{
    Q_OBJECT
public:
    //	Represents entry as it already exists in the database
    class MLperformance
    {
    public:
        MLperformance() { _init(); }

        int songID;
        int songPerformerID;
        int songPerformanceID;
        int albumID;
        int albumPerformerID;
        int albumPosition;
        int albumPerformanceID;
        int onlinePerformanceID;
        QString path;
        bool pathExists;
        QString key;
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
        QString filePath;                     //	relative path to file
        QString parentDirectoryName;          //	name of parent directory
        QString parentDirectoryPath;          //	relative path to parent directory
        QString absoluteParentDirectoryPath;  //	absolute path to parent directory
        QString extension;

        //	Primary meta data attributes (need to exist)
        int albumPosition;
        QString albumTitle;
        QString songPerformerName;
        QString songTitle;

        //	Secondary meta data attributes (optional)
        QString albumPerformerName;
        SBDuration duration;
        QString genre;
        QString notes;
        int year;

        //	Album edit
        int mergedToAlbumPosition;
        int orgAlbumPosition;
        bool removedFlag;
        bool newFlag;

        //	Chart edit
        int chartPosition;

        //	Songbase ids
        int songID;
        int songPerformerID;
        int songPerformanceID;
        int albumID;
        int albumPerformerID;
        int albumPerformanceID;

        //	Helper attributes
        QString errorMsg;
        int ID;
        QString key;	//	file path to online song
        bool isImported;
        QString importReason;

        inline bool compareID(const MLentity& i) const { return ((songID==i.songID)&&(songPerformerID==i.songPerformerID)&&(albumID==i.albumID)&&(albumPosition==i.albumPosition))?1:0; }
        inline bool errorFlag() const { return errorMsg.length()>0?1:0; }
    private:
        void _init() { songID=-1; songPerformerID=-1; songPerformanceID=-1; albumID=-1; albumPerformerID=-1; albumPerformanceID=-1; albumPosition=-1; mergedToAlbumPosition=-1; orgAlbumPosition=-1; removedFlag=0; newFlag=0; ID=-1; }
    };
    typedef std::shared_ptr<MLentity> MLentityPtr;

    class MLalbumPath
    {
    public:
        MLalbumPath() { _init(); }

        int              albumID;
        int              albumPerformerID;
        QString          albumPerformerName;
        QString          albumTitle;
        QString          genre;
        int              maxPosition;
        QString          path;
        QString          directoryName;
        QVector<QString> uniqueAlbumTitles;
        QVector<QString> uniqueSongPerformerNames;
        bool             variousPerformerFlag;
        int              year;

        bool multipleEntriesFlag() const { return (variousPerformerFlag || uniqueAlbumTitles.count()>1 || uniqueSongPerformerNames.count()>1)?1:0; }
    private:
        void _init() { albumID=-1; albumPerformerID=-1; maxPosition=0; variousPerformerFlag=0; year=-1; }

    };
    typedef std::shared_ptr<MLalbumPath> MLalbumPathPtr;

    //	Public methods
    explicit MusicLibrary(QObject *parent = 0);
    void rescanMusicLibrary(bool suppressDialogsFlag=false);
    bool validateEntityList(QVector<MLentityPtr>& list,QHash<QString,MLalbumPathPtr>& directory2albumPathMap, bool suppressDialogsFlag=false);

signals:

public slots:

private:
    int _numNewSongs;
    int _numNewPerformers;
    int _numNewAlbums;

    QStringList _greatestHitsAlbums() const;
    QMap<QString,QString> _alternativePerformerName2CorrectPerformerName;


    QString _retrieveCorrectPerformerName(DataAccessLayer* dal, const QString& altPerformerName);
    void _addAlternativePerformerName(DataAccessLayer* dal, const QString& altPerformerName,const QString& correctPerformerName);

};

#endif // MUSICLIBRARY_H
