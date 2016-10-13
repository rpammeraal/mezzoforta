#ifndef SBIDPERFORMER_H
#define SBIDPERFORMER_H

#include "QHash"

#include "SBIDBase.h"

class SBSqlQueryModel;
class QLineEdit;

class SBIDPerformer : public SBIDBase
{

public:
        //	Ctors, dtors
    SBIDPerformer();
    SBIDPerformer(const SBIDBase& c);
    SBIDPerformer(const SBIDPerformer& c);
    SBIDPerformer(int itemID);
    SBIDPerformer(const QString& performerName);
    ~SBIDPerformer();

    //	Public methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual SBSqlQueryModel* findMatches(const QString& name) const;
    virtual QString genericDescription() const;
    virtual int getDetail(bool createIfNotExistFlag=0);
    virtual QString hash() const;
    static QString iconResourceLocation();
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual bool save();
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual void setText(const QString &text);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods unique to SBIDPerformer
    QString addRelatedPerformerSQL(int performerID) const;
    QString deleteRelatedPerformerSQL(int performerID) const;
    SBSqlQueryModel* getAlbums() const;
    SBSqlQueryModel* getAllSongs() const;
    SBSqlQueryModel* getAllOnlineSongs() const;
    SBSqlQueryModel* getRelatedPerformers() const;
    void setCount1(int count1) { _count1=count1; }
    void setCount2(int count2) { _count1=count2; }
    void setNotes(const QString& notes) { _notes=notes; }
    void setPerformerID(int performerID) { _sb_performer_id=performerID; }
    void setPerformerName(const QString& performerName) { _performerName=performerName; }
    static bool selectSavePerformer(const QString& editedPerformerName,const SBIDPerformer& existingPerformer,SBIDPerformer& selectedPerformer,QLineEdit* field=NULL, bool saveNewPerformer=1);
    void setURL(const QString& url) { _url=url; }
    static bool updateExistingPerformer(const SBIDBase& orgPerformerID, SBIDPerformer& newPerformerID, const QStringList& extraSQL=QStringList(),bool commitFlag=1);	//	CWIP: merge with save
    void updateURLdb(const QString& homePage); //	CWIP: implement mechanism where these can be set through the regular set-methods
    void updateMBIDdb(const QString& mbid);    //	CWIP: and get either updated in the database by save() or on destruction.
                                               //	CWIP: would be helpful to have a cache of SBID* ready
    static void updateSoundexFields();

    //	Operators
    virtual bool operator==(const SBIDBase& i) const;
    virtual operator QString() const;

//public slots:
    /*
public:
    */

private:
    void _init();
};

inline uint qHash(const SBIDPerformer& p,uint seed=0)
{
    return p.performerID()>=0?qHash(p.performerID(),seed):qHash(p.performerName(),seed);
}

#endif // SBIDPERFORMER_H
