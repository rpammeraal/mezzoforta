#ifndef SBIDBASE_H
#define SBIDBASE_H

#include <memory>

#include <QString>
#include <QByteArray>

#include <QDataStream>
#include <QStandardItem>

#include "Common.h"
#include "Duration.h"

class SBSqlQueryModel;

class SBIDBase;
typedef std::shared_ptr<SBIDBase> SBIDPtr;

class SBIDBase
{

public:
    enum sb_type
    {
        sb_type_invalid=0,
        sb_type_song=1,
        sb_type_performer=2,
        sb_type_album=3,
        sb_type_chart=4,
        sb_type_playlist=5,
        sb_type_performance=6
    };

    SBIDBase();
    SBIDBase(const SBIDBase& c);
    //SBIDBase(QByteArray encodedData);
    virtual ~SBIDBase();
    static SBIDPtr createPtr(SBIDBase::sb_type itemType,int ID,bool noDependentsFlag=0);
    static SBIDPtr createPtr(const QByteArray& encodedData);
    static SBIDPtr createPtr(const QString& key,bool noDependentsFlag=0);

    //	Public methods
    virtual QByteArray encode() const;

    //	Public virtual methods (Methods that only apply to subclasseses)
    virtual int commonPerformerID() const=0;
    virtual QString commonPerformerName() const=0;
    virtual QString genericDescription() const=0;
    virtual QString iconResourceLocation() const=0;
    virtual int itemID() const=0;
    virtual sb_type itemType() const=0;
    virtual void sendToPlayQueue(bool enqueueFlag=0)=0;
    virtual QString text() const=0;
    virtual QString type() const=0;

    //	Common Getters
    inline QString url() const { return _url; }
    inline QString wiki() const { return _wiki; }

    //	Setters
    void setURL(const QString& url) { _url=url; }	//	CWIPneed to save this in DB instantly

    //	Methods specific to SBIDBase
    QString errorMessage() const { return _errorMsg; }
    QString MBID() const { return _sb_mbid; }
    int modelPosition() const { return _sb_model_position; }
    void setErrorMessage(const QString& errorMsg) { _errorMsg=errorMsg; }
    void setModelPosition(int modelPosition) { _sb_model_position=modelPosition; }
    void setMBID(const QString& mbid) { _sb_mbid=mbid; setChangedFlag(); }

    void showDebug(const QString& title) const;

    //	Operators
    virtual bool operator==(const SBIDBase& i) const;	//	compares on itemType(),key()
    virtual bool operator!=(const SBIDBase& i) const;	//	inverse of operator==()
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const=0;
    virtual void refreshDependents(bool showProgressDialogFlag=1,bool forcedFlag=1)=0;

    //	Aux methods
    static SBIDBase::sb_type convert(Common::sb_field f);

protected:
    friend class SBIDAlbum;
    friend class SBIDPerformer;
    friend class SBIDPlaylist;
    friend class SBIDSong;
    friend class SBModel;
    template <class T, class parentT> friend class SBIDManagerTemplate;


    //	Tertiary identifiers (navigation et al)
    QString     _errorMsg;

    //	Used by SBIDManager
    bool        _deletedFlag;
    int         _mergedWithID;

    //	Used by SBIDManager*:: and SBID*:: classes
    inline bool changedFlag() const { return _changedFlag; }
    inline void clearChangedFlag() { _changedFlag=0; }
    inline bool deletedFlag() const { return _deletedFlag; }
    inline bool mergedFlag() const { qDebug() << SB_DEBUG_INFO << _mergedWithID; return _mergedWithID!=-1; }
    inline int mergeWithID() const { return _mergedWithID; }
    inline void setChangedFlag() { _changedFlag=1; }
    inline void setDeletedFlag() { _deletedFlag=1; }

    //	Used by SBIDManager*:: and SBID*:: classes
    virtual void isSaved();

private:
    bool        _changedFlag;
    bool        _newFlag;
    sb_type     _sb_item_type;
    QString     _sb_mbid;
    int         _sb_model_position;
    QString     _url;	//	any item may have an url
    QString     _wiki;	//	any item may have an wiki page

    void _init();
};

//Q_DECLARE_METATYPE(SBIDBase);

inline uint qHash(const SBIDBase& k, uint seed)
{
    return qHash(k.key(),seed);
}

#endif // SBIDBASE_H
