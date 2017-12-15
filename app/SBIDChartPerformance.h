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
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual Common::sb_type itemType() const;
    virtual QString genericDescription() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDChartPerformance specific methods
    inline int chartPerformanceID() const { return _chartPerformanceID; }
    inline int chartPosition() const { return _chartPosition; }

    //	Setters

    //	Pointers
    SBIDSongPerformancePtr songPerformancePtr() const;

    //	Redirectors
    int songPerformerID() const;
    int songID() const;
    QString songPerformerName() const;
    QString songTitle() const;

    //	Operators
    virtual operator QString();

    //	Methods required by CacheTemplate
    static SBKey createKey(int songPerformanceID);
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBSqlQueryModel* chartPerformancesBySongPerformance(int songPerformanceID);
    static SBIDChartPerformancePtr retrieveChartPerformance(int chartPerformanceID, bool noDependentsFlag=1);

    //	Helper methods for CacheTemplate
    static Common::sb_type classType() { return Common::sb_type_chart_performance; }

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDChartPerformance();

    //	Operators
    SBIDChartPerformance& operator=(const SBIDChartPerformance& t);

    //	Methods used by SBIDManager
    static SBIDChartPerformancePtr createInDB(Common::sb_parameters& p);
    static SBIDChartPerformancePtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& chartPerformanceID);
    void postInstantiate(SBIDChartPerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    virtual void setPrimaryKey(int PK) { _chartPerformanceID=PK;  }
    QStringList updateSQL(const Common::db_change db_change) const;

    //	Protected setters
    friend class SBIDSongPerformance;
    inline void setSongPerformanceID(int songPerformanceID) { _songPerformanceID=songPerformanceID; setChangedFlag(); }

private:
    //	Attributes
    int     _chartPerformanceID;
    int     _chartID;
    int     _songPerformanceID;
    int     _chartPosition;
    QString _notes;

    void _copy(const SBIDChartPerformance& c);
    void _init();
};

#endif // SBIDCHARTPERFORMANCE_H
