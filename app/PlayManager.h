#ifndef PLAYMANAGER_H
#define PLAYMANAGER_H

#include <QObject>
#include <QDebug>

#include "Common.h"
#include "SBIDSong.h"
#include "SBIDPlaylist.h"

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
    inline int currentPlayID() const { return _currentPlayID; }
    inline int radioModeFlag() const { return _radioModeFlag; }
    bool songPlayingFlag() const;

signals:
    void playlistChanged(int playlistID);
    void setRowVisible(int playIndex);

public slots:
    //	Player related
    void playerPrevious();
    bool playerPlay();
    bool playerNextAuto(bool endOfSongFlag);
    bool playerNext(PlayMode playMode=PlayMode::Default,bool endOfSongFlag=0);
    void playerStop();

    //	Playlist et al related
    void changeCurrentDatabaseSchema();
    void clearPlaylist();
    bool playItemNow(SBKey key,const bool enqueueFlag=0);
    void shufflePlaylist();
    void startRadio();

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

    friend class SBTabQueuedSongs;
    bool playItem(unsigned int playlistIndex,PlayMode playMode=PlayMode::Default);

private:
    int  _currentPlayID;    //	0-based, -1: no song active
    bool _radioModeFlag;

    void _init();
    void _loadRadio();
    void _resetCurrentPlayID();
    SBIDOnlinePerformancePtr _performanceAt(int index) const;
    void _setCurrentPlayID(int currentPlayID);
};

#endif // PLAYMANAGER_H
