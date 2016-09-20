#include "SBIDPlaylist.h"

#include "Context.h"
#include "DataEntityPlaylist.h"
#include "SBModelQueuedSongs.h"

SBIDPlaylist::SBIDPlaylist():SBIDBase()
{
    _init();
}

SBIDPlaylist::SBIDPlaylist(const SBIDPlaylist &c):SBIDBase(c)
{
}

SBIDPlaylist::SBIDPlaylist(const SBIDBase &c):SBIDBase(c)
{
    _sb_item_type=SBIDBase::sb_type_playlist;
}

SBIDPlaylist::SBIDPlaylist(int itemID):SBIDBase()
{
    _init();
    _sb_playlist_id=itemID;
}

SBIDPlaylist::~SBIDPlaylist()
{
}

///	Public methods
SBSqlQueryModel*
SBIDPlaylist::findMatches(const QString& name) const
{
    Q_UNUSED(name);
    qDebug() << SB_DEBUG_ERROR << "NOT IMPLEMENTED!";
    return NULL;
}

int
SBIDPlaylist::getDetail(bool createIfNotExistFlag)
{
    Q_UNUSED(createIfNotExistFlag);
    qDebug() << SB_DEBUG_ERROR << "NOT IMPLEMENTED!";
    return 0;
}

QString
SBIDPlaylist::genericDescription() const
{
    return "Playlist - " + this->text();
}

QString
SBIDPlaylist::hash() const
{
    return QString("%1:%2").arg(itemType()).arg(this->playlistID());
}

QString
SBIDPlaylist::iconResourceLocation() const
{
    return ":/images/PlaylistIcon.png";
}

int
SBIDPlaylist::itemID() const
{
    return _sb_playlist_id;
}

SBIDBase::sb_type
SBIDPlaylist::itemType() const
{
    return SBIDBase::sb_type_playlist;
}

bool
SBIDPlaylist::save()
{
    qDebug() << SB_DEBUG_ERROR << "NOT IMPLEMENTED!";
    return 0;
}

void
SBIDPlaylist::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDBase> list;
    DataEntityPlaylist dep;
    list=dep.retrievePlaylistItems(*this);

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

void
SBIDPlaylist::setText(const QString &text)
{
    _playlistName=text;
}

QString
SBIDPlaylist::text() const
{
    return this->_playlistName;
}

QString
SBIDPlaylist::type() const
{
    return "playlist";
}

///	Operators
bool
SBIDPlaylist::operator ==(const SBIDBase& i) const
{
    if(
        i._sb_playlist_id==this->_sb_playlist_id
    )
    {
        return 1;
    }
    return 0;
}

SBIDPlaylist::operator QString() const
{
    QString playlistName=this->_playlistName.length() ? this->_playlistName : "<N/A>";
    return QString("SBIDPlaylist:%1,%2:n=%3")
            .arg(this->_sb_playlist_id)
            .arg(this->_sb_tmp_item_id)
            .arg(playlistName)
    ;
}

///	Private methods

void
SBIDPlaylist::_init()
{
    _sb_item_type=SBIDBase::sb_type_playlist;
    _sb_playlist_id=-1;
}
