#include <QDebug>

#include "Cache.h"
#include "CacheManager.h"
#include "Context.h"
#include "SBIDBase.h"
#include "SBIDAlbum.h"
#include "SBIDChart.h"
#include "SBIDChartPerformance.h"
#include "SBIDPerformer.h"
#include "SBIDPlaylist.h"
#include "SBIDPlaylistDetail.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDSong.h"


SBIDBase::SBIDBase(const SBIDBase &c):SBKey(c)
{
    _copy(c);
}

SBIDBase::SBIDBase(ItemType itemType, int itemID):SBKey(itemType,itemID)
{
    _init();
}

SBIDBase::~SBIDBase()
{
}

///	Public methods

///	Public virtual methods (Methods that only apply to subclasseses)
void
SBIDBase::showDebug(const QString& title) const
{
    qDebug() << SB_DEBUG_INFO << title;
    qDebug() << SB_DEBUG_INFO << "key" << key();
}

bool
SBIDBase::operator ==(const SBIDBase& i) const
{
    return i.key()==this->key();
}

bool
SBIDBase::operator !=(const SBIDBase& i) const
{
    return !(this->operator==(i));
}

SBIDBase::operator QString() const
{
    return this->key().toString() + ":" + this->genericDescription();
}

///	Aux
SBKey::ItemType
SBIDBase::convert(Common::sb_field f)
{
    ItemType t;
    switch(f)
    {
    case Common::sb_field_invalid:
        t=Invalid;
        break;

    case Common::sb_field_song_id:
        t=Song;
        break;

    case Common::sb_field_performer_id:
        t=Performer;
        break;

    case Common::sb_field_album_id:
        t=Album;
        break;

    case Common::sb_field_chart_id:
        t=Chart;
        break;

    case Common::sb_field_album_performance_id:
        t=AlbumPerformance;
        break;

    case Common::sb_field_online_performance_id:
        t=OnlinePerformance;
        break;

    case Common::sb_field_playlist_id:
        t=Playlist;
        break;


    case Common::sb_field_key:
    case Common::sb_field_album_position:
        qDebug() << SB_DEBUG_ERROR << "Not able to translate Common::sb_field_album_position to Common::sb_type!";
        t=Invalid;
        break;
    }
    return t;
}

QString
SBIDBase::iconResourceLocationClass(SBKey key)
{
    return iconResourceLocationClass(key.itemType());
}

QString
SBIDBase::iconResourceLocationClass(ItemType itemType)
{
    switch(itemType)
    {
    case Performer:
        return QString(":/images/NoBandPhoto.png");

    case Album:
        return ":/images/NoAlbumCover.png";

    case Chart:
        return ":/images/ChartIcon.png";

    case Playlist:
        return ":/images/PlaylistIcon.png";

    case AlbumPerformance:
    case ChartPerformance:
    case PlaylistDetail:
    case Song:
    case SongPerformance:
        return QString(":/images/SongIcon.png");

    case OnlinePerformance:
        break;

    case Invalid:
        break;
    }

    return QString("n/a");
}

void
SBIDBase::setReloadFlag()
{
    _reloadFlag=1;
    qDebug() << SB_DEBUG_INFO << this->ID() << this->key();
}


///	Protected
void
SBIDBase::clearChangedFlag()
{
    _changedFlag=0;
}

void
SBIDBase::clearReloadFlag()
{
    _reloadFlag=0;
}

void
SBIDBase::rollback()
{
    clearChangedFlag();
}

void
SBIDBase::setChangedFlag()
{
    _changedFlag=1;
    SB_RETURN_VOID_IF_NULL(_owningCache);
    _owningCache->addChangedKey(this->key());
}

void
SBIDBase::setDeletedFlag()
{
    _deletedFlag=1;
    setChangedFlag();
}

void
SBIDBase::_copy(const SBIDBase &c)
{
    _errorMsg=c._errorMsg;
    _deletedFlag=c._deletedFlag;
    _changedFlag=c._changedFlag;
    _id=-2;	//	do NOT copy -- identifies copy
    _sb_mbid=c._sb_mbid;
    _sb_model_position=c._sb_model_position;
    _reloadFlag=c._reloadFlag;
    _url=c._url;
    _wiki=c._wiki;
    _owningCache=NULL;	//	do NOT copy -- identifies copy
}

///	PRIVATE
SBIDBase::SBIDBase()
{
    _init();
}

void
SBIDBase::_init()
{
    QString e;

    //	Private
    _changedFlag=0;
    _deletedFlag=0;
    _id=Common::nextID();
    _sb_mbid=e;
    _sb_model_position=-1;
    _reloadFlag=0;
    _url=e;
    _wiki=e;

    //	Protected
    _errorMsg=e;
}
