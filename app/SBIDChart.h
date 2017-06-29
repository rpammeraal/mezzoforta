#ifndef SBIDCHART_H
#define SBIDCHART_H

#include "SBIDBase.h"
#include "SBTableModel.h"
#include "SBIDSongPerformance.h"

class SBIDChart;
typedef std::shared_ptr<SBIDChart> SBIDChartPtr;

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
    virtual sb_type itemType() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods specific to SBIDChart
    inline int chartID() const { return _chartID; }
    inline QString chartName() const { return _chartName; }
    inline QString chartNotes() const { return _notes; }
    inline QDate chartReleaseDate() const { return _releaseDate; }
    SBTableModel* items() const;
    int numItems() const;
    //bool removeChartItem(int chartPosition);
    //bool moveItem(const SBIDPtr& fromPtr, int toRow);
    //void reorderItem(const SBIDPtr fromPtr, const SBIDPtr toID) const;	//	CWIP:pmgr rewrite

    //	Operators
    virtual operator QString() const;
    SBIDChart& operator=(const SBIDChart& t);	//	CWIP: to be moved to protected

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static QString createKey(int chartID,int unused=-1);
    static SBIDChartPtr retrieveChart(int chartID,bool noDependentsFlag=0,bool showProgressDialogFlag=0);

protected:
    SBIDChart();
    SBIDChart(int itemID);

    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    //	Methods used by SBIDManager (these should all become pure virtual if not static)
    //bool addDependent(SBIDPtr tobeAddedPtr);
    //static SBIDChartPtr createInDB();
    static SBIDChartPtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& albumID);
    void postInstantiate(SBIDChartPtr& ptr);
    //bool moveDependent(int fromPosition, int toPosition);
    //bool removeDependent(int position);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    //QStringList updateSQL() const;

private:
    int               _chartID;
    QString           _chartName;
    QString           _notes;
    QDate             _releaseDate;

    //	Not instantiated
    int               _num_items;	//	may only be used until _items has been loaded

    QMap<int,SBIDSongPerformancePtr> _items;

    //	Methods:0

    void _init();
    void _loadPerformances();
    static QMap<int,SBIDSongPerformancePtr> _loadPerformancesFromDB(int chartID,bool showProgressDialogFlag=0);
};

#endif // SBIDCHART_H
