#ifndef SBIDCHART_H
#define SBIDCHART_H

#include "SBIDBase.h"
#include "SBTableModel.h"
#include "SBIDSongPerformance.h"

class SBIDChart : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDChart(const SBIDChart& c);
    ~SBIDChart();

    //	Public methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual Common::sb_type itemType() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods specific to SBIDChart
    inline int chartID() const { return _chartID; }
    inline QString chartName() const { return _chartName; }
    inline QString chartNotes() const { return _notes; }
    inline QDate chartReleaseDate() const { return _releaseDate; }
    QMap<int,SBIDChartPerformancePtr> items() const;
    int numItems() const;
    SBTableModel* tableModelItems() const;
    //bool removeChartItem(int chartPosition);
    //bool moveItem(const SBIDPtr& fromPtr, int toRow);
    //void reorderItem(const SBIDPtr fromPtr, const SBIDPtr toID) const;	//	CWIP:pmgr rewrite

    //	Operators
    virtual operator QString() const;

    //	Methods required by CacheTemplate
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBKey createKey(int chartID);
    static SBIDChartPtr retrieveChart(int chartID,bool noDependentsFlag=1);
    static SBIDChartPtr retrieveChart(SBKey key,bool noDependentsFlag=1);

    //	Helper methods for CacheTemplate
    static Common::sb_type classType() { return Common::sb_type_chart; }

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDChart();

    //	Operators
    SBIDChart& operator=(const SBIDChart& t);

    //	Methods used by SBIDManager (these should all become pure virtual if not static)
    //bool addDependent(SBIDPtr tobeAddedPtr);
    static SBIDChartPtr createInDB(Common::sb_parameters& p);
    static SBIDChartPtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& albumID);
    void postInstantiate(SBIDChartPtr& ptr);
    //bool moveDependent(int fromPosition, int toPosition);
    //bool removeDependent(int position);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    virtual void setPrimaryKey(int PK) { _chartID=PK;  }
    QStringList updateSQL(const Common::db_change db_change) const;

private:
    int                               _chartID;
    QString                           _chartName;
    QString                           _notes;
    QDate                             _releaseDate;

    //	Not instantiated
    int                               _numItems ;	//	may only be used until _items has been loaded

    QMap<int,SBIDChartPerformancePtr> _items;

    //	Methods:0

    void _copy(const SBIDChart& c);
    void _init();
    void _loadPerformances();
    static QMap<SBIDChartPerformancePtr,SBIDChartPtr> _loadPerformancesFromDB(const SBIDChart& id);
};

#endif // SBIDCHART_H
