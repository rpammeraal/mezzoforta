#include <QDebug>

#include "SBIDBase.h"

#include "Common.h"
#include "Context.h"
#include "SBIDAlbum.h"
#include "SBIDChart.h"
#include "SBIDChartPerformance.h"
#include "SBIDPerformer.h"
#include "SBIDPlaylist.h"
#include "CacheTemplate.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDSong.h"
#include "SBMessageBox.h"


SBIDBase::SBIDBase()
{
    _init();
}

SBIDBase::SBIDBase(const SBIDBase &c)
{
    _init();


    _errorMsg=c._errorMsg;
    _deletedFlag=c._deletedFlag;
    _changedFlag=c._changedFlag;
    _id=-2;	//	do NOT copy -- identifies copy
    _sb_item_type=c._sb_item_type;
    _sb_mbid=c._sb_mbid;
    _sb_model_position=c._sb_model_position;
    _url=c._url;
    _wiki=c._wiki;
    _owningCache=NULL;	//	do NOT copy -- identifies copy
}

SBIDBase::~SBIDBase()
{
}

SBIDPtr
SBIDBase::createPtr(Common::sb_type itemType,int itemID,bool noDependentsFlag)
{
    SBIDPtr ptr;
    switch(itemType)
    {
    case Common::sb_type_album:
        ptr=SBIDAlbum::retrieveAlbum(itemID,noDependentsFlag);
        break;

    case Common::sb_type_performer:
        ptr=SBIDPerformer::retrievePerformer(itemID,noDependentsFlag);
        break;

    case Common::sb_type_song:
        ptr=SBIDSong::retrieveSong(itemID,noDependentsFlag);
        break;

    case Common::sb_type_playlist:
        ptr=SBIDPlaylist::retrievePlaylist(itemID,noDependentsFlag);
        break;

    case Common::sb_type_album_performance:
        ptr=SBIDAlbumPerformance::retrieveAlbumPerformance(itemID,noDependentsFlag);
        break;

    case Common::sb_type_online_performance:
        ptr=SBIDOnlinePerformance::retrieveOnlinePerformance(itemID,noDependentsFlag);
        break;

    case Common::sb_type_chart:
        ptr=SBIDChart::retrieveChart(itemID,noDependentsFlag);
        break;

    case Common::sb_type_chart_performance:
        ptr=SBIDChartPerformance::retrieveChartPerformance(itemID,noDependentsFlag);
        break;

    case Common::sb_type_playlist_detail:
        ptr=SBIDPlaylistDetail::retrievePlaylistDetail(itemID,noDependentsFlag);
		break;

    case Common::sb_type_song_performance:
        ptr=SBIDSongPerformance::retrieveSongPerformance(itemID,noDependentsFlag);
        break;

    case Common::sb_type_invalid:
        break;
    }
    if(!ptr)
    {
        qDebug() << SB_DEBUG_NPTR;
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << ptr->itemType() << ptr->itemID() << ptr->genericDescription();
    }
    return ptr;
}

SBIDPtr
SBIDBase::createPtr(const SBKey &key,bool noDependentsFlag)
{
    return SBIDBase::createPtr(key.itemType(),key.itemID(),noDependentsFlag);
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
    return this->key() + ":" + this->genericDescription();
}

SBKey
SBIDBase::key() const
{
    return SBKey(itemType(),itemID());
}

///	Aux
Common::sb_type
SBIDBase::convert(Common::sb_field f)
{
    Common::sb_type t;
    switch(f)
    {
    case Common::sb_field_invalid:
        t=Common::sb_type_invalid;
        break;

    case Common::sb_field_song_id:
        t=Common::sb_type_song;
        break;

    case Common::sb_field_performer_id:
        t=Common::sb_type_performer;
        break;

    case Common::sb_field_album_id:
        t=Common::sb_type_album;
        break;

    case Common::sb_field_chart_id:
        t=Common::sb_type_chart;
        break;

    case Common::sb_field_album_performance_id:
        t=Common::sb_type_album_performance;
        break;

    case Common::sb_field_online_performance_id:
        t=Common::sb_type_online_performance;
        break;

    case Common::sb_field_playlist_id:
        t=Common::sb_type_playlist;
        break;


    case Common::sb_field_key:
    case Common::sb_field_album_position:
        qDebug() << SB_DEBUG_ERROR << "Not able to translate Common::sb_field_album_position to Common::sb_type!";
        t=Common::sb_type_invalid;
        break;
    }
    return t;
}

QString
SBIDBase::iconResourceLocationClass(const SBKey &key)
{
    return iconResourceLocationClass(key.itemType());
}

QString
SBIDBase::iconResourceLocationClass(Common::sb_type itemType)
{
    switch(itemType)
    {
    case Common::sb_type_performer:
        return QString(":/images/NoBandPhoto.png");

    case Common::sb_type_album:
        return ":/images/NoAlbumCover.png";

    case Common::sb_type_chart:
        return ":/images/ChartIcon.png";

    case Common::sb_type_playlist:
        return ":/images/PlaylistIcon.png";

    case Common::sb_type_album_performance:
    case Common::sb_type_chart_performance:
    case Common::sb_type_playlist_detail:
    case Common::sb_type_song:
    case Common::sb_type_song_performance:
        return QString(":/images/SongIcon.png");

    case Common::sb_type_online_performance:
        break;

    case Common::sb_type_invalid:
        break;
    }

    return QString("n/a");
}

///	Protected
void
SBIDBase::clearChangedFlag()
{
    _changedFlag=0;
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

///	PRIVATE
void
SBIDBase::_init()
{
    QString e;

    //	Private
    _changedFlag=0;
    _id=Common::nextID();
    _sb_item_type=Common::sb_type_invalid;
    _sb_mbid=e;
    _sb_model_position=-1;
    _url=e;
    _wiki=e;

    //	Protected
    _errorMsg=e;
    _deletedFlag=0;
}
