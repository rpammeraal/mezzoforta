#ifndef SBIDPERFORMER_H
#define SBIDPERFORMER_H

#include <QHash>
#include <QSqlRecord>

#include "SBIDBase.h"

class SBSqlQueryModel;
class QLineEdit;

class SBIDPerformer;
typedef std::shared_ptr<SBIDPerformer> SBIDPerformerPtr;

#include "SBIDAlbumPerformance.h"

class SBTableModel;

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
    SBTableModel* albums() const;
    QVector<SBIDAlbumPtr> albumList() const;
    QVector<SBIDAlbumPerformancePtr> albumPerformances() const;
    SBTableModel* charts() const;
    inline QString notes() const { return _notes; }
    int numAlbums() const;
    int numSongs() const;
    inline int performerID() const { return _performerID; }
    inline QString performerName() const { return _performerName; }
    QVector<SBIDPerformerPtr> relatedPerformers();
    SBTableModel* songs() const;

    //	Setters
    void addRelatedPerformer(const QString& performerKey);
    void deleteRelatedPerformer(const QString& performerKey);
    void setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void setPerformerName(const QString& performerName) { _performerName=performerName; setChangedFlag() ;}

    //	Static methods
    static void updateSoundexFields();

    //	Operators
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    static QString createKey(int performerID,int unused=-1);
    virtual QString key() const;
    static bool match(const QString& editedPerformerName,int skipID);
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBIDPerformerPtr retrievePerformer(int performerID,bool noDependentsFlag=1);
    static SBIDPerformerPtr retrieveVariousPerformers();

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDPerformer();

    //	Operators
    SBIDPerformer& operator=(const SBIDPerformer& t);

    static SBIDPerformerPtr createInDB();
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDPerformerPtr existingPerformerPtr);
    static SBIDPerformerPtr instantiate(const QSqlRecord& r);
    void mergeTo(SBIDPerformerPtr& to);
    static void openKey(const QString& key, int& performerID);
    void postInstantiate(SBIDPerformerPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;
    static SBIDPerformerPtr userMatch(const Common::sb_parameters& parameters, SBIDPerformerPtr existingPerformerPtr);

    //	Helper methods
    QString addRelatedPerformerSQL(const QString& key) const;
    QString deleteRelatedPerformerSQL(const QString& key) const;

private:
    int                               _performerID;
    QString                           _performerName;
    QString                           _notes;

    //	Attributes derived from core attributes
    QVector<SBIDAlbumPtr>             _albumList;
    QVector<SBIDAlbumPerformancePtr>  _albumPerformances;
    QVector<QString>                  _relatedPerformerKey;

    //	Methods
    void _copy(const SBIDPerformer& c);
    void _init();
    void _loadAlbums();
    void _loadAlbumPerformances();
    QVector<QString> _loadRelatedPerformers() const;

    QVector<SBIDAlbumPerformancePtr> _loadAlbumPerformancesFromDB() const;
    QVector<SBIDAlbumPtr> _loadAlbumsFromDB() const;
};

inline uint qHash(const SBIDPerformer& p,uint seed=0)
{
    return p.performerID()>=0?qHash(p.performerID(),seed):qHash(p.performerName(),seed);
}

#endif // SBIDPERFORMER_H
