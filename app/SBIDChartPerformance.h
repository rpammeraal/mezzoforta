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
    virtual SBIDBase::sb_type itemType() const;
    virtual QString genericDescription() const;
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

    //	Methods required by SBIDManagerTemplate
    static QString createKey(int songPerformanceID);
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBIDChartPerformancePtr retrieveChartPerformance(int chartPerformanceID, bool noDependentsFlag=0);

    //	Helper methods for SBIDManagerTemplate
    //static SBSqlQueryModel* songPerformancesOnChart(int songID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDChartPerformance();

    //	Methods used by SBIDManager
    static SBIDChartPerformancePtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& chartPerformanceID);
    void postInstantiate(SBIDChartPerformancePtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    //QStringList updateSQL() const;

private:
    //	Attributes
    int     _chartPerformanceID;
    int     _chartID;
    int     _songPerformanceID;
    int     _chartPosition;
    QString _notes;

    void _init();
};

#endif // SBIDCHARTPERFORMANCE_H
