#ifndef SBIDCHARTPERFORMANCE_H
#define SBIDCHARTPERFORMANCE_H

#include "SBIDBase.h"

class SBIDChartPerformance : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDChartPerformance(const SBIDChartPerformance& p);

    //	Inherited methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString defaultIconResourceLocation() const;
    virtual QString genericDescription() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual SBIDPtr retrieveItem(const SBKey& itemKey) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDChartPerformance specific methods
    inline int chartPerformanceID() const { return itemID(); }
    inline int chartPosition() const { return _chartPosition; }

    //	Setters

    //	Pointers
    SBIDSongPerformancePtr songPerformancePtr() const;
    SBIDSongPtr songPtr() const;
    SBIDChartPtr chartPtr() const;

    //	Redirectors
    int songPerformerID() const;
    int songID() const;
    inline int chartID() const { return _chartID; } ;
    QString songPerformerName() const;
    QString songTitle() const;

    //	Operators
    virtual operator QString();

    //	Methods required by CacheTemplate
    static SBKey createKey(int songPerformanceID);
    virtual void refreshDependents(bool forcedFlag=0);

    //	Static methods
    static SBSqlQueryModel* chartPerformancesBySongPerformance(int songPerformanceID);
    static SBIDChartPerformancePtr retrieveChartPerformance(int chartPerformanceID);
    static SBIDChartPerformancePtr retrieveChartPerformance(const SBKey& key);

    //	Helper methods for CacheTemplate
    //static ItemType classType() { return ChartPerformance; }

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDChartPerformance();
    SBIDChartPerformance(int chartPerformanceID);

    //	Operators
    SBIDChartPerformance& operator=(const SBIDChartPerformance& t);

    //	Methods used by SBIDManager
    static SBIDChartPerformancePtr createInDB(Common::sb_parameters& p);
    static SBIDChartPerformancePtr instantiate(const QSqlRecord& r);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    QStringList updateSQL(const Common::db_change db_change) const;

    //	Protected setters
    friend class SBIDSongPerformance;
    inline void setSongPerformanceID(int songPerformanceID) { _songPerformanceID=songPerformanceID; setChangedFlag(); }

private:
    //	Attributes
    int     _chartID;
    int     _songPerformanceID;
    int     _chartPosition;
    QString _notes;

    void _copy(const SBIDChartPerformance& c);
    void _init();
};

#endif // SBIDCHARTPERFORMANCE_H
