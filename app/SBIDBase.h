#ifndef SBIDBASE_H
#define SBIDBASE_H

#include <memory>

#include <QString>
#include <QByteArray>

#include <QDataStream>
#include <QStandardItem>

#include "Common.h"
#include "SBKey.h"

class SBSqlQueryModel;

class SBIDBase;             typedef std::shared_ptr<SBIDBase>              SBIDPtr;


class Cache;

class SBIDAlbum;            typedef std::shared_ptr<SBIDAlbum>             SBIDAlbumPtr;
class SBIDAlbumPerformance; typedef std::shared_ptr<SBIDAlbumPerformance>  SBIDAlbumPerformancePtr;
class SBIDChart;            typedef std::shared_ptr<SBIDChart>             SBIDChartPtr;
class SBIDChartPerformance; typedef std::shared_ptr<SBIDChartPerformance>  SBIDChartPerformancePtr;
class SBIDOnlinePerformance;typedef std::shared_ptr<SBIDOnlinePerformance> SBIDOnlinePerformancePtr;
class SBIDPerformer;        typedef std::shared_ptr<SBIDPerformer>         SBIDPerformerPtr;
class SBIDSong;             typedef std::shared_ptr<SBIDSong>              SBIDSongPtr;
class SBIDSongPerformance;  typedef std::shared_ptr<SBIDSongPerformance>   SBIDSongPerformancePtr;

class SBIDBase : public SBKey
{

public:

    virtual ~SBIDBase();

    //	Public methods
    inline bool changedFlag() const { return _changedFlag; }
    inline bool deletedFlag() const { return _deletedFlag; }

    //	Public virtual methods (Methods that only apply to subclasseses)
    virtual int commonPerformerID() const=0;
    virtual QString commonPerformerName() const=0;
    virtual QString genericDescription() const=0;
    virtual QString iconResourceLocation() const=0;
    virtual ItemType itemType() const=0;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const=0;
    virtual void sendToPlayQueue(bool enqueueFlag=0)=0;
    virtual QString text() const=0;
    virtual QString type() const=0;

    //	Common Getters
    inline QString url() const { return _url; }
    inline QString wiki() const { return _wiki; }

    //	Methods specific to SBIDBase
    QString errorMessage() const { return _errorMsg; }
    QString ID() const { return QString("ID=%1").arg(_id); }
    QString MBID() const { return _sb_mbid; }
    int modelPosition() const { return _sb_model_position; }
    inline bool reloadFlag() const { return _reloadFlag; }

    //	Setters
    void setErrorMessage(const QString& errorMsg) { _errorMsg=errorMsg; }
    void setModelPosition(int modelPosition) { _sb_model_position=modelPosition; }
    void setMBID(const QString& mbid) { qDebug() << SB_DEBUG_INFO; _sb_mbid=mbid; setChangedFlag(); }
    void setURL(const QString& url) { _url=url; setChangedFlag(); }	//	CWIPneed to save this in DB instantly

    void showDebug(const QString& title) const;

    //	Operators
    virtual bool operator==(const SBIDBase& i) const;	//	compares on key()
    virtual bool operator!=(const SBIDBase& i) const;	//	inverse of operator==()
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0)=0;

    //	Aux methods
    static ItemType convert(Common::sb_field f);
    static QString iconResourceLocationClass(SBKey key);
    static QString iconResourceLocationClass(ItemType itemType);
    virtual void setReloadFlag();

protected:
    SBIDBase(const SBIDBase& c);
    SBIDBase(ItemType itemType,int itemID);

    friend class SBIDAlbum;
    friend class SBIDAlbumPerformance;
    friend class SBIDChart;
    friend class SBIDChartPerformance;
    friend class SBIDOnlinePerformance;
    friend class SBIDPerformer;
    friend class SBIDPlaylist;
    friend class SBIDSong;
    friend class SBIDSongPerformance;
    friend class SBModel;
    template <class T, class parentT> friend class CacheTemplate;

    //	Tertiary identifiers (navigation et al)
    QString     _errorMsg;

    //	Used by CacheManager and SBID*:: classes
    virtual void clearChangedFlag();
    virtual void clearReloadFlag();
    virtual void rollback();
    void setChangedFlag();
    virtual void setDeletedFlag();

    void _copy(const SBIDBase& c);

private:
    bool            _changedFlag;
    bool            _deletedFlag;
    int             _id;
    QString         _sb_mbid;
    int             _sb_model_position;
    bool            _reloadFlag; //	set when any of its dependencies has been changed or removed
    QString         _url;	//	any item may have an url
    QString         _wiki;	//	any item may have an wiki page
    Cache*          _owningCache;

    SBIDBase();
    void _init();
};

//Q_DECLARE_METATYPE(SBIDBase);

//inline uint qHash(const SBIDBase& k, uint seed)
//{
//    return qHash(k.getKey(),seed);
//}

#endif // SBIDBASE_H
