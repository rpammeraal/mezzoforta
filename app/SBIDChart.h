#ifndef SBIDCHART_H
#define SBIDCHART_H

#include "SBIDBase.h"

class SBTableModel;

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
    virtual QString defaultIconResourceLocation() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual SBIDPtr retrieveItem(const SBKey& itemKey) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //  Methods used for web interface
    virtual SBSqlQueryModel* allItems(const QChar& startsWith, qsizetype offset, qsizetype size) const;
    virtual QString getIconLocation(const SBKey::ItemType& fallbackType) const;
    virtual QString HTMLDetailItem(QString htmlTemplate) const;
    virtual QString HTMLListItem(const QSqlRecord& r) const;

    //	Methods specific to SBIDChart
    inline int chartID() const { return itemID(); }
    inline QString chartName() const { return _chartName; }
    inline QString chartNotes() const { return _notes; }
    inline QDate chartEndingDate() const { return _chartEndingDate; }
    bool import(const QString& fileName,bool truncateFlag);
    QMap<int,SBIDChartPerformancePtr> items() const;
    int numItems() const;
    SBTableModel* tableModelItems() const;
    //bool removeChartItem(int chartPosition);
    //bool moveItem(const SBIDPtr& fromPtr, int toRow);
    //void reorderItem(const SBIDPtr fromPtr, const SBIDPtr toID) const;	//	CWIP:pmgr rewrite

    //	Setters
    inline void setChartName(const QString& chartName) { _chartName=chartName; setChangedFlag(); }
    inline void setChartEndingDate(const QDate& chartEndingDate) { _chartEndingDate=chartEndingDate; setChangedFlag(); }

    //	Operators
    virtual operator QString() const;

    //	Methods required by CacheTemplate
    virtual void refreshDependents(bool forcedFlag=0);

    //	Static methods
    static SBKey createKey(int chartID);
    static SBIDChartPtr retrieveChart(int chartID);
    static SBIDChartPtr retrieveChart(SBKey key);

    //	Helper methods for CacheTemplate
    //static ItemType classType() { return Chart; }

protected:
    template <class T, class parentT> friend class CacheTemplate;
    template <class T> friend class WebTemplate;
    friend class Preloader;

    SBIDChart();
    SBIDChart(int chartID);

    //	Operators
    SBIDChart& operator=(const SBIDChart& t);

    //	Methods used by SBIDManager (these should all become pure virtual if not static)
    //bool addDependent(SBIDPtr tobeAddedPtr);
    static SBIDChartPtr createInDB(Common::sb_parameters& p);
    static SBIDChartPtr instantiate(const QSqlRecord& r);
    //bool moveDependent(int fromPosition, int toPosition);
    //bool removeDependent(int position);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    virtual void setDeletedFlag();
    QStringList updateSQL(const Common::db_change db_change) const;

private:
    QString                           _chartName;
    QString                           _notes;
    QDate                             _chartEndingDate;

    //	Not instantiated
    int                               _numItems ;	//	may only be used until _items has been loaded

    QMap<int,SBIDChartPerformancePtr> _items;

    //	Methods:0

    void _copy(const SBIDChart& c);
    void _init();
    void _loadPerformances();
    void _truncate();
    static QMap<SBIDChartPerformancePtr,SBIDChartPtr> _loadPerformancesFromDB(const SBIDChart& id);
};

#endif // SBIDCHART_H
