#include <QDebug>

#include "SBIDBase.h"

#include "Common.h"
#include "Context.h"
#include "SBIDAlbum.h"
#include "SBIDPerformer.h"
#include "SBIDPlaylist.h"
#include "SBIDManagerTemplate.h"
#include "SBIDSong.h"
#include "SBMessageBox.h"


SBIDBase::SBIDBase()
{
    _init();
}

SBIDBase::SBIDBase(const SBIDBase &c)
{
    this->_sb_item_type=c._sb_item_type;
    this->_sb_mbid=c._sb_mbid;
    this->_sb_model_position=c._sb_model_position;
    this->_url=c._url;
    this->_wiki=c._wiki;
}

/*
SBIDBase::SBIDBase(QByteArray encodedData)
{
    _init();
    QString s=QString(encodedData);
    if(!s.length())
    {
        qDebug() << SB_DEBUG_ERROR << "NO MIME DATA!";
        return;
    }
    QStringList sl=s.split('_');


    _sb_item_type            =static_cast<sb_type>(sl[0].toInt());
    _sb_song_id               =sl[1].toInt();
    _sb_song_performer_id     =sl[2].toInt();
    //_sb_album_id              =sl[3].toInt();
    //_sb_album_performer_id    =sl[4].toInt();
    _sb_performer_id          =sl[5].toInt();
    //_sb_album_position        =sl[6].toInt();
    _sb_chart_id              =sl[7].toInt();
    _sb_playlist_id           =sl[8].toInt();
    _sb_mbid                  =sl[9].replace("___SB_UNDERSCORE_123___","_");
    _originalPerformerFlag  =sl[10].toInt();
    _songPerformerName        =sl[11];
    _albumPerformerName       =sl[12];
    _performerName            =sl[13];
    _albumTitle               =sl[14];
    _songTitle                =sl[15];
    _year                     =sl[16].toInt();
    _path                     =sl[17];
    _lyrics                   =sl[18];
    _notes                    =sl[19];
    _genre                    =sl[20];
    _url                      =sl[21];
    _wiki                     =sl[22];
    _playlistName             =sl[23];
    _count1                   =sl[24].toInt();
    _count2                   =sl[25].toInt();
    _duration.setDuration(sl[26].toInt());
    _sb_play_position         =sl[27].toInt();
    _sb_playlist_position     =sl[28].toInt();
    _sb_model_position        =sl[29].toInt();
    return;
}
*/

SBIDBase::~SBIDBase()
{
}

SBIDPtr
SBIDBase::createPtr(SBIDBase::sb_type itemType, int itemID)
{
    SBIDPtr ptr;
    switch(itemType)
    {
    case SBIDBase::sb_type_album:
        ptr=SBIDAlbum::retrieveAlbum(itemID);
        break;

    case SBIDBase::sb_type_performer:
        ptr=SBIDPerformer::retrievePerformer(itemID);
        break;

    case SBIDBase::sb_type_song:
        qDebug() << SB_DEBUG_INFO;
        ptr=SBIDSong::retrieveSong(itemID);
        break;

    case SBIDBase::sb_type_playlist:
        ptr=SBIDPlaylist::retrievePlaylist(itemID);
        break;

    case SBIDBase::sb_type_performance:
    case SBIDBase::sb_type_invalid:
    case SBIDBase::sb_type_chart:
        break;
    }
    qDebug() << SB_DEBUG_INFO;
    return ptr;
}

SBIDPtr
SBIDBase::createPtr(const QByteArray& encodedData)
{
    SBIDPtr ptr;
    QString s=QString(encodedData);
    if(!s.length())
    {
        qDebug() << SB_DEBUG_ERROR << "NO MIME DATA!";
        return SBIDPtr();
    }
    QStringList sl=s.split('_');

    //	This gotta be the weirdest formatting.
    ptr=SBIDBase::createPtr(
                static_cast<sb_type>(sl[0].toInt()),
                                     sl[1].toInt()
    );

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
    return QString("SBIDBase::operator QString():We don't know!");
}

///	PRIVATE
void
SBIDBase::_init()
{
    QString e;

    //	Private
    _changedFlag=0;
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
