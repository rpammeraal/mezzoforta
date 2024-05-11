#ifndef PLAYMANAGER_H
#define PLAYMANAGER_H

#include <QObject>
#include <QDebug>

#include "SBKey.h"
#include "SBIDBase.h"

///
/// \brief The PlayManager class
///
/// Meant to untangle PlayerController vs SBTabQueuedSong vs SBModelQueuedSong.
///
/// PlayManager: interface to playing music to the rest of the app
/// PlayerController: manages which deck is active, load, stop, start, pause song
/// SBTabQueuedSong: UI for queued songs
/// SBModelQueuedSong: model for SBTabQueuedSong
///
class PlayManager : public QObject
{
    Q_OBJECT

    enum PlayMode
    {
        Default=0,
        Previous,
        SetReady,
    };

public:
    PlayManager(QObject *parent = 0);
    int currentPlayID() const;
    inline int radioModeFlag() const { return _radioModeFlag; }
    bool songPlayingFlag() const;
    int numSongs() const;

signals:
    void playlistChanged(int playlistID);
    void setRowVisible(int playIndex);
    void listReordered();

public slots:
    //	Player related
    void playerPrevious();
    bool playerPlay();
    bool playerNext(PlayManager::PlayMode playMode=PlayManager::PlayMode::Default);
    void playerStop();
    void handleNeedMoreSongs();

    //	Playlist et al related
    void changeCurrentDatabaseSchema();
    void clearPlaylist();
    bool playItemNow(SBKey key,const bool enqueueFlag=0);
    void shufflePlaylist();
    void startRadio();

protected:
    friend class Context;
    friend class PlayerController;
    void doInit();	//	Init done by Context::

    friend class SBTabQueuedSongs;
    bool playItem(unsigned int playlistIndex);
    SBIDOnlinePerformancePtr getNextPlayItem() const;
    void handlePlayingSong(SBIDOnlinePerformancePtr opPtr);

private:
    bool _radioModeFlag;

    qsizetype checkLast100Performers(const QMap<int,SBIDPerformerPtr>& last100Performers,const QString& performerName, qsizetype index) const;
    void _init();
    void _loadRadio();
    void _resetCurrentPlayID();
    SBIDOnlinePerformancePtr _performanceAt(int index) const;
    void _setCurrentPlayID(int currentPlayID);
};

#endif // PLAYMANAGER_H
