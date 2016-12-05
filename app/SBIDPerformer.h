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
    void addRelatedPerformer(int performerID);
    void deleteRelatedPerformer(int performerID);
    inline QString notes() const { return _notes; }
    int numAlbums() const;
    int numSongs() const;
    inline int performerID() const { return _sb_performer_id; }
    inline QString performerName() const { return _performerName; }
    QVector<SBIDPerformerPtr> relatedPerformers();
    SBTableModel* songs() const;

    static void updateSoundexFields();

    //	Operators
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    static bool match(const QString& editedPerformerName,int skipID);
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBIDPerformerPtr retrievePerformer(int performerID,bool noDependentsFlag=0,bool showProgressDialogFlag=0);
    static SBIDPerformerPtr retrieveVariousArtists();

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDPerformer();

    static SBIDPerformerPtr createInDB();
    static QString createKey(int performerID,int unused=-1);
    static SBSqlQueryModel* find(const Common::sb_parameters& tobeFound,SBIDPerformerPtr existingPerformerPtr);
    static SBIDPerformerPtr instantiate(const QSqlRecord& r);
    void mergeTo(SBIDPerformerPtr& to);
    static void openKey(const QString& key, int& performerID);
    void postInstantiate(SBIDPerformerPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;
    static SBIDPerformerPtr userMatch(const Common::sb_parameters& parameters, SBIDPerformerPtr existingPerformerPtr);

    //	Helper methods
    QString addRelatedPerformerSQL(int performerID) const;
    QString deleteRelatedPerformerSQL(int performerID) const;

private:
    QVector<SBIDAlbumPtr>             _albums;
    QString                           _notes;
    QVector<SBIDAlbumPerformancePtr>  _performances;
    QString                           _performerName;
    int                               _sb_performer_id;
    QVector<int>                      _relatedPerformerID;

    //	Not instantiated
    int                               _num_albums;
    int                               _num_songs;

    //	Methods
    void _init();
    void _loadAlbums();
    void _loadPerformances(bool showProgressDialogFlag=0);
    QVector<int> _loadRelatedPerformers() const;

    //	Internal setters
    void _setNotes(const QString& notes) { _notes=notes; setChangedFlag(); }
    void _setPerformerName(const QString& performerName) { _performerName=performerName; setChangedFlag(); }
};

inline uint qHash(const SBIDPerformer& p,uint seed=0)
{
    return p.performerID()>=0?qHash(p.performerID(),seed):qHash(p.performerName(),seed);
}

#endif // SBIDPERFORMER_H
