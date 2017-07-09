#include <QDebug>

#include "SBIDBase.h"

#include "Common.h"
#include "Context.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDPlaylist.h"
#include "SBIDManagerTemplate.h"
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
    _mergedWithID=c._mergedWithID;


    _changedFlag=c._changedFlag;
    _id=c._id;
    _newFlag=c._newFlag;
    _sb_item_type=c._sb_item_type;
    _sb_mbid=c._sb_mbid;
    _sb_model_position=c._sb_model_position;
    _url=c._url;
    _wiki=c._wiki;
}

SBIDBase::~SBIDBase()
{
}

SBIDPtr
SBIDBase::createPtr(SBIDBase::sb_type itemType,int itemID,bool noDependentsFlag,bool showProgressDialogFlag)
{
    SBIDPtr ptr;
    switch(itemType)
    {
    case SBIDBase::sb_type_album:
        ptr=SBIDAlbum::retrieveAlbum(itemID,noDependentsFlag);
        break;

    case SBIDBase::sb_type_performer:
        ptr=SBIDPerformer::retrievePerformer(itemID,noDependentsFlag,showProgressDialogFlag);
        break;

    case SBIDBase::sb_type_song:
        ptr=SBIDSong::retrieveSong(itemID,noDependentsFlag);
        break;

    case SBIDBase::sb_type_playlist:
        ptr=SBIDPlaylist::retrievePlaylist(itemID,noDependentsFlag,showProgressDialogFlag);
        break;

    case SBIDBase::sb_type_album_performance:
        ptr=SBIDAlbumPerformance::retrieveAlbumPerformance(itemID,noDependentsFlag);
        break;

    case SBIDBase::sb_type_online_performance:
        ptr=SBIDOnlinePerformance::retrieveOnlinePerformance(itemID,noDependentsFlag);
        break;

    case SBIDBase::sb_type_chart:
        ptr=SBIDChart::retrieveChart(itemID,noDependentsFlag);
        break;

    case SBIDBase::sb_type_chart_performance:
        ptr=SBIDChartPerformance::retrieveChartPerformance(itemID,noDependentsFlag);
        break;

    case SBIDBase::sb_type_song_performance:
    case SBIDBase::sb_type_invalid:
        break;
    }
    return ptr;
}

SBIDPtr
SBIDBase::createPtr(const QByteArray& encodedData)
{
    QString s=QString(encodedData);
    if(s.length()==0)
    {
        qDebug() << SB_DEBUG_ERROR << "NO MIME DATA!";
        return SBIDPtr();
    }
    return SBIDBase::createPtr(s,1);
}

SBIDPtr
SBIDBase::createPtr(const QString &key,bool noDependentsFlag,bool showProgressDialogFlag)
{
    qDebug() << SB_DEBUG_INFO;
    SBIDPtr ptr=SBIDPtr();
    if(key.length())
    {
        const QStringList list=key.split(":");
        const SBIDBase::sb_type itemType=static_cast<SBIDBase::sb_type>(list[0].toInt());
        const int itemID=list[1].toInt();
        ptr=SBIDBase::createPtr(itemType,itemID,noDependentsFlag,showProgressDialogFlag);
    }
    return ptr;
}

///	Public methods

QByteArray
SBIDBase::encode() const
{
    QByteArray encodedData;
    encodedData.append(key());

    return encodedData;
}


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
    if(
        i.itemType()==this->itemType() &&
        i.key()==this->key())
    {
        return 1;
    }
    return 0;
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

///	Aux
SBIDBase::sb_type
SBIDBase::convert(Common::sb_field f)
{
    sb_type t;
    switch(f)
    {
    case Common::sb_field_invalid:
        t=SBIDBase::sb_type_invalid;
        break;

    case Common::sb_field_song_id:
        t=SBIDBase::sb_type_song;
        break;

    case Common::sb_field_performer_id:
        t=SBIDBase::sb_type_performer;
        break;

    case Common::sb_field_album_id:
        t=SBIDBase::sb_type_album;
        break;

    case Common::sb_field_chart_id:
        t=SBIDBase::sb_type_chart;
        break;

    case Common::sb_field_album_performance_id:
        t=SBIDBase::sb_type_album_performance;
        break;

    case Common::sb_field_online_performance_id:
        t=SBIDBase::sb_type_online_performance;
        break;

    case Common::sb_field_playlist_id:
        t=SBIDBase::sb_type_playlist;
        break;


    case Common::sb_field_key:
    case Common::sb_field_album_position:
        qDebug() << SB_DEBUG_ERROR << "Not able to translate Common::sb_field_album_position to SBIDBase::sb_type!";
        t=SBIDBase::sb_type_invalid;
        break;
    }
    return t;
}

///	Protected
void
SBIDBase::clearChangedFlag()
{
    _changedFlag=0;
}

///	PRIVATE
void
SBIDBase::_init()
{
    QString e;

    //	Private
    _changedFlag=0;
    _id=Common::nextID();
    _newFlag=0;
    _sb_item_type=sb_type_invalid;
    _sb_mbid=e;
    _sb_model_position=-1;
    _url=e;
    _wiki=e;

    //	Protected
    _errorMsg=e;
    _deletedFlag=0;
    _mergedWithID=-1;
}
