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

public:
    PlayManager(QObject *parent = 0);
    inline int currentPlayID() const { return _currentPlayID; }
    inline int radioModeFlag() const { return _radioModeFlag; }

signals:
    void playlistChanged(const SBIDPlaylist& playlist);
    void setRowVisible(int playIndex);

public slots:
    //	Player related
    void playerPrevious();
    bool playerPlay();
    bool playerNext(bool previousFlag=0);
    void playerStop();

    //	Playlist et al related
    void changeSchema();
    void clearPlaylist();
    bool playItemNow(SBIDBase& toPlay,const bool enqueueFlag=0);
    bool playItemNow(unsigned int playlistIndex);
    void shufflePlaylist();
    void startRadio();

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

private:
    int  _currentPlayID;  //	0-based, -1: no song active
    bool _radioModeFlag;

    void _init();
    void _loadRadio();
    void _resetCurrentPlayID();
    SBIDSong _songAt(int index) const;
    void _setCurrentPlayID(int currentPlayID);
};

#endif // PLAYMANAGER_H
