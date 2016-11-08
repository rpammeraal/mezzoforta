#ifndef SBIDPERFORMER_H
#define SBIDPERFORMER_H

#include <QHash>
#include <QSqlRecord>

#include "SBIDBase.h"

class SBSqlQueryModel;
class QLineEdit;

class SBIDPerformer;
typedef std::shared_ptr<SBIDPerformer> SBIDPerformerPtr;

class SBIDPerformer : public SBIDBase
{

public:
        //	Ctors, dtors
    SBIDPerformer(const SBIDPerformer& c);
    ~SBIDPerformer();

    //	Public methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods unique to SBIDPerformer
    void addRelatedPerformer(int performerID);
    void deleteRelatedPerformer(int performerID);
    SBSqlQueryModel* getAlbums() const;
    SBSqlQueryModel* getAllSongs() const;
    SBSqlQueryModel* getAllOnlineSongs() const;
    inline QString notes() const { return _notes; }
    inline int numAlbums() const { return _num_albums; }
    inline int numSongs() const { return _num_songs; }
    inline int performerID() const { return _sb_performer_id; }
    inline QString performerName() const { return _performerName; }
    QVector<SBIDPerformerPtr> relatedPerformers();
    //void setCount1(int count1) { _count1=count1; }
    //void setCount2(int count2) { _count1=count2; }
    void setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void setPerformerName(const QString& performerName) { _performerName=performerName; setChangedFlag(); }
    static bool selectSavePerformer(const QString& editedPerformerName,const SBIDPerformerPtr& existingPerformerPtr,SBIDPerformerPtr& selectedPerformerPtr,QLineEdit* field=NULL, bool saveNewPerformer=1);

    static void updateSoundexFields();

    //	Operators
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    QString key() const;

    //	Static methods
    static SBIDPerformerPtr retrievePerformer(int performerID,bool noDependentsFlag=0);

protected:
    template <class T> friend class SBIDManagerTemplate;

    SBIDPerformer();

    //	Methods used by SBIDManager (these should all become pure virtual if not static)
    static SBIDPerformerPtr createInDB();
    static QString createKey(int performerID,int unused=-1);
    static SBSqlQueryModel* find(const QString& tobeFound,int excludeItemID,QString secondaryParameter);
    static SBIDPerformerPtr instantiate(const QSqlRecord& r,bool noDependentsFlag=0);
    void mergeTo(SBIDPerformerPtr& to);
    static void openKey(const QString& key, int& performerID);
    void postInstantiate(SBIDPerformerPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;

    //	Helper methods
    QString addRelatedPerformerSQL(int performerID) const;
    QString deleteRelatedPerformerSQL(int performerID) const;

private:
    QString      _notes;
    QString      _performerName;
    int          _sb_performer_id;
    QVector<int> _relatedPerformerID;

    //	Not instantiated
    int          _num_albums;
    int          _num_songs;

    //	Methods
    void _init();
    QVector<int> _loadRelatedPerformers() const;
};

inline uint qHash(const SBIDPerformer& p,uint seed=0)
{
    return p.performerID()>=0?qHash(p.performerID(),seed):qHash(p.performerName(),seed);
}

#endif // SBIDPERFORMER_H
