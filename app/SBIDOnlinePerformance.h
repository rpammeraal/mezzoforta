#ifndef SBIDONLINEPERFORMANCE_H
#define SBIDONLINEPERFORMANCE_H


class SBIDOnlinePerformance : public SBIDAlbumPerformance
{
public:
    SBIDOnlinePerformance(const SBIDOnlinePerformance& p);
    ~SBIDOnlinePerformance();

    //	Inherited methods
    virtual SBIDBase::sb_type itemType() const;
    virtual QString genericDescription() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString type() const;

    //	SBIDOnlinePerformance specific methods
    inline QString path() const { return _path; }

    //	Setters

    //	Pointers

    //	Operators
    virtual operator QString();

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static QString createKey(int onlinePerformanceID);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDOnlinePerformance();

    //	Methods used by SBIDManager

    static SBIDOnlinePerformancePtr createNew(int songID, int performerID, int albumID, int albumPosition, int year, const Duration& duration, const QString& notes, const QString& path);

private:
    int              _onlinePerformanceID;
    Duration         _duration;
    QString          _path;

    void _init();
};

#endif // SBIDONLINEPERFORMANCE_H
