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

    this->_sb_song_id=c._sb_song_id;
    this->_sb_song_performer_id=c._sb_song_performer_id;
    this->_sb_album_id=c._sb_album_id;
    this->_sb_album_performer_id=c._sb_album_performer_id;
    this->_sb_performer_id=c._sb_performer_id;
    this->_sb_album_position=c._sb_album_position;
    this->_sb_chart_id=c._sb_chart_id;
    this->_sb_playlist_id=c._sb_playlist_id;
    this->_sb_playlist_position=c._sb_playlist_position;
    this->_sb_model_position=c._sb_model_position;
    this->_sb_play_position=c._sb_play_position;

    this->_originalPerformerFlag=c._originalPerformerFlag;
    this->_albumTitle=c._albumTitle;
    this->_count1=c._count1;
    this->_count2=c._count2;
    this->_duration=c._duration;
    this->_genre=c._genre;
    this->_lyrics=c._lyrics;
    this->_notes=c._notes;
    this->_songPerformerName=c._songPerformerName;
    this->_performerName=c._performerName;
    this->_albumPerformerName=c._albumPerformerName;
    this->_playlistName=c._playlistName;
    this->_songTitle=c._songTitle;
    this->_url=c._url;
    this->_wiki=c._wiki;
    this->_year=c._year;
    this->_path=c._path;

    this->_sb_tmp_item_id=c._sb_tmp_item_id;
    this->_sb_tmp_song_id=c._sb_tmp_song_id;
    this->_sb_tmp_album_id=c._sb_tmp_album_id;
    this->_sb_tmp_performer_id=c._sb_tmp_performer_id;
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
    _sb_album_id              =sl[3].toInt();
    _sb_album_performer_id    =sl[4].toInt();
    _sb_performer_id          =sl[5].toInt();
    _sb_album_position        =sl[6].toInt();
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
SBIDBase::createPtr(SBIDBase::sb_type itemType, int ID)
{
    SBIDPlaylistMgr* plmgr=Context::instance()->getPlaylistMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();

    SBIDPtr ptr;
    switch(itemType)
    {
    case SBIDBase::sb_type_album:
        ptr=std::make_shared<SBIDAlbum>(SBIDAlbum(ID));
        break;

    case SBIDBase::sb_type_performer:
    {
        ptr=pemgr->retrieve(ID);
        break;
    }

    case SBIDBase::sb_type_song:
        ptr=std::make_shared<SBIDSong>(SBIDSong(ID));
        break;

    case SBIDBase::sb_type_playlist:
        ptr=plmgr->retrieve(ID);
        break;

    case SBIDBase::sb_type_invalid:
    case SBIDBase::sb_type_chart:
        break;
    }
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

    ptr->_sb_song_performer_id      =sl[2].toInt();
    ptr->_sb_album_id               =sl[3].toInt();
    ptr->_sb_album_performer_id     =sl[4].toInt();
    ptr->_sb_performer_id           =sl[5].toInt();
    ptr->_sb_album_position         =sl[6].toInt();
    ptr->_sb_chart_id               =sl[7].toInt();
    ptr->_sb_playlist_id            =sl[8].toInt();
    ptr->_sb_mbid                   =sl[9].replace("___SB_UNDERSCORE_123___","_");
    ptr->_originalPerformerFlag    =sl[10].toInt();
    ptr->_songPerformerName        =sl[11];
    ptr->_albumPerformerName       =sl[12];
    ptr->_performerName            =sl[13];
    ptr->_albumTitle               =sl[14];
    ptr->_songTitle                =sl[15];
    ptr->_year                     =sl[16].toInt();
    ptr->_path                     =sl[17];
    ptr->_lyrics                   =sl[18];
    ptr->_notes                    =sl[19];
    ptr->_genre                    =sl[20];
    ptr->_url                      =sl[21];
    ptr->_wiki                     =sl[22];
    ptr->_playlistName             =sl[23];
    ptr->_count1                   =sl[24].toInt();
    ptr->_count2                   =sl[25].toInt();
         ptr->_duration.setDuration(sl[26].toInt());
    ptr->_sb_play_position         =sl[27].toInt();
    ptr->_sb_playlist_position     =sl[28].toInt();
    ptr->_sb_model_position        =sl[29].toInt();

    return ptr;
}

///	Public methods

QByteArray
SBIDBase::encode() const
{
    //	Put all attributes in a stringlist
    QStringList sl;
    sl.append(QString("%1").arg(itemType()));                 //	0
    sl.append(QString("%1").arg(_sb_song_id));
    sl.append(QString("%1").arg(_sb_song_performer_id));
    sl.append(QString("%1").arg(_sb_album_id));
    sl.append(QString("%1").arg(_sb_album_performer_id));
    sl.append(QString("%1").arg(_sb_performer_id));            //	5
    sl.append(QString("%1").arg(_sb_album_position));
    sl.append(QString("%1").arg(_sb_chart_id));
    sl.append(QString("%1").arg(_sb_playlist_id));
    sl.append(_sb_mbid);
    sl.append(QString("%1").arg(_originalPerformerFlag));    //	10
    sl.append(SB_REPLACE_UNDERSCORE(_songPerformerName));
    sl.append(SB_REPLACE_UNDERSCORE(_albumPerformerName));
    sl.append(SB_REPLACE_UNDERSCORE(_performerName));
    sl.append(SB_REPLACE_UNDERSCORE(_albumTitle));
    sl.append(SB_REPLACE_UNDERSCORE(_songTitle));              //	15
    sl.append(QString("%1").arg(_year));
    sl.append(SB_REPLACE_UNDERSCORE(_path));
    sl.append(SB_REPLACE_UNDERSCORE(_lyrics));
    sl.append(SB_REPLACE_UNDERSCORE(_notes));
    sl.append(SB_REPLACE_UNDERSCORE(_genre));                  //	20
    sl.append(SB_REPLACE_UNDERSCORE(_url));
    sl.append(SB_REPLACE_UNDERSCORE(_wiki));
    sl.append(SB_REPLACE_UNDERSCORE(_playlistName));
    sl.append(QString("%1").arg(_count1));
    sl.append(QString("%1").arg(_count2));                     //	25
    sl.append(QString("%1").arg(_duration.MS()));
    sl.append(QString("%1").arg(_sb_play_position));
    sl.append(QString("%1").arg(_sb_playlist_position));
    sl.append(QString("%1").arg(_sb_model_position));          //	30


    QString combined=sl.join('_');

    QByteArray encodedData;
    encodedData.append(combined);

    return encodedData;
}


///	Public virtual methods (Methods that only apply to subclasseses)

//bool
//SBIDBase::compare(const SBIDBase &t) const
//{
//    qDebug() << SB_DEBUG_ERROR << "SHOULD NOT BE CALLED!";
//    Q_UNUSED(t);
//    return 0;
//}
int
SBIDBase::commonPerformerID() const
{
    SBMessageBox::standardWarningBox(QString("Method %1() called [%2:%3]").arg(__FUNCTION__).arg(__FILE__).arg(__LINE__));
    return -1;
}

QString
SBIDBase::commonPerformerName() const
{
    return "Ambigious Name at SBID Base";
}

SBSqlQueryModel*
SBIDBase::findMatches(const QString& name) const
{
    Q_UNUSED(name);
    qDebug() << SB_DEBUG_ERROR << "SHOULD NOT BE CALLED!";
    return NULL;
}

int
SBIDBase::getDetailOLD(bool createIfNotExistFlag)
{
    Q_UNUSED(createIfNotExistFlag);
    qDebug() << SB_DEBUG_ERROR << "SHOULD NOT BE CALLED!";
    return 0;
}

QString
SBIDBase::genericDescription() const
{
    return "Generic";
}

QString
SBIDBase::hash() const
{
    return QString("%1:%2:%3:%4:%5:%6:%7:%8:%9")
        .arg(this->itemType())
        .arg(this->songID())
        .arg(this->songPerformerID())
        .arg(this->albumID())
        .arg(this->albumPerformerID())
        .arg(this->performerID())
        .arg(this->playlistID())
        .arg(this->albumPosition())
        .arg(this->playlistPosition())
    ;
}

QString
SBIDBase::iconResourceLocation() const
{
    switch(this->itemType())
    {
    case SBIDBase::sb_type_album:
        return SBIDSong::iconResourceLocation();

    case SBIDBase::sb_type_performer:
        return SBIDPerformer::iconResourceLocation();

    case SBIDBase::sb_type_song:
        return SBIDSong::iconResourceLocation();

    case SBIDBase::sb_type_playlist:
        return SBIDPlaylist::iconResourceLocation();

    case SBIDBase::sb_type_invalid:
    case SBIDBase::sb_type_chart:
        break;
    }
    return QString();
}

int
SBIDBase::itemID() const
{
    switch(this->itemType())
    {
    case SBIDBase::sb_type_album:
        return static_cast<const SBIDAlbum>(*this).itemID();

    case SBIDBase::sb_type_performer:
        //return static_cast<const SBIDPerformer>(*this).itemID();
        break;

    case SBIDBase::sb_type_song:
        return static_cast<const SBIDSong>(*this).itemID();

    case SBIDBase::sb_type_playlist:
        //return static_cast<const SBIDPlaylist>(*this).itemID();
        break;

    case SBIDBase::sb_type_invalid:
    case SBIDBase::sb_type_chart:
        break;
    }
    return -1;
}

SBIDBase::sb_type
SBIDBase::itemType() const
{
    return _sb_item_type;
}

bool
SBIDBase::saveOLD()
{
    qDebug() << SB_DEBUG_ERROR << "SHOULD NOT BE CALLED!";
    return 0;
}

//void
//SBIDBase::resetSequence() const
//{
//    _sequence.clear();
//}

void
SBIDBase::sendToPlayQueue(bool enqueueFlag)
{
    SBMessageBox::standardWarningBox(QString("Method %1() called [%2:%3]").arg(__FUNCTION__).arg(__FILE__).arg(__LINE__));
    Q_UNUSED(enqueueFlag);
    qDebug() << SB_DEBUG_ERROR << "SHOULD NOT BE CALLED!";
}

void
SBIDBase::setText(const QString &text)
{
    qDebug() << SB_DEBUG_ERROR << "SHOULD NOT BE CALLED!";
    SBMessageBox::standardWarningBox(QString("Method %1() called [%2:%3]").arg(__FUNCTION__).arg(__FILE__).arg(__LINE__));
    Q_UNUSED(text);
}

QString
SBIDBase::text() const
{
    qDebug() << SB_DEBUG_ERROR << "SHOULD NOT BE CALLED!";
    SBMessageBox::standardWarningBox(QString("Method %1() called [%2:%3]").arg(__FUNCTION__).arg(__FILE__).arg(__LINE__));
    return QString("n/a");
}

QString
SBIDBase::type() const
{
    qDebug() << SB_DEBUG_ERROR << "SHOULD NOT BE CALLED!";
    SBMessageBox::standardWarningBox(QString("Method %1() called [%2:%3]").arg(__FUNCTION__).arg(__FILE__).arg(__LINE__));
    return "n/a";
}

bool
SBIDBase::validFlag() const
{
    return (itemType()!=SBIDBase::sb_type_invalid && itemID()>=0?1:0);
}


void
SBIDBase::showDebug(const QString& title) const
{
    qDebug() << SB_DEBUG_INFO << title;
    qDebug() << SB_DEBUG_INFO << "sb_item_id" << itemID();
    qDebug() << SB_DEBUG_INFO << "itemType" << itemType();
    qDebug() << SB_DEBUG_INFO << "sb_mbid" << _sb_mbid;
    qDebug() << SB_DEBUG_INFO << "sb_song_id" << _sb_song_id;
    qDebug() << SB_DEBUG_INFO << "sb_song_performer_id" << _sb_song_performer_id;
    qDebug() << SB_DEBUG_INFO << "sb_album_id" << _sb_album_id;
    qDebug() << SB_DEBUG_INFO << "sb_album_performer_id" << _sb_album_performer_id;
    qDebug() << SB_DEBUG_INFO << "sb_position" << _sb_album_position;
    qDebug() << SB_DEBUG_INFO << "sb_chart_id" << _sb_chart_id;
    qDebug() << SB_DEBUG_INFO << "sb_playlist_id" << _sb_playlist_id;
    qDebug() << SB_DEBUG_INFO << "sb_playlist_position" << _sb_playlist_position;
    qDebug() << SB_DEBUG_INFO << "isOriginalPerformerFlag" << _originalPerformerFlag;
    qDebug() << SB_DEBUG_INFO << "albumTitle" << _albumTitle;
    qDebug() << SB_DEBUG_INFO << "count1" << _count1;
    qDebug() << SB_DEBUG_INFO << "count2" << _count2;
    qDebug() << SB_DEBUG_INFO << "duration" << _duration.toString();
    qDebug() << SB_DEBUG_INFO << "genre" << _genre;
    //qDebug() << SB_DEBUG_INFO << "lyrics" << lyrics;
    qDebug() << SB_DEBUG_INFO << "notes" << _notes;
    qDebug() << SB_DEBUG_INFO << "songPerformerName" << _songPerformerName;
    qDebug() << SB_DEBUG_INFO << "albumPerformerName" << _albumPerformerName;
    qDebug() << SB_DEBUG_INFO << "performerName" << _performerName;
    qDebug() << SB_DEBUG_INFO << "playlistName" << _playlistName;
    qDebug() << SB_DEBUG_INFO << "songTitle" << _songTitle;
    qDebug() << SB_DEBUG_INFO << "url" << _url;
    qDebug() << SB_DEBUG_INFO << "wiki" << _wiki;
    qDebug() << SB_DEBUG_INFO << "year" << _year;
    qDebug() << SB_DEBUG_INFO << "path" << _path;
    qDebug() << SB_DEBUG_INFO << "playPosition" << _sb_play_position;
}

int
SBIDBase::assignTmpItemID()
{
    /*
    this->_sb_tmp_item_id=-(_sequence.length()+1);
    _sequence.append(this->_sb_tmp_item_id);
    _sequenceMap[this->_sb_tmp_item_id]=(*this);
    return this->_sb_tmp_item_id;
    */
    return 0;
}

void
SBIDBase::listTmpIDItems()
{
    QMapIterator<int,SBIDBase> it(_sequenceMap);

    qDebug() << SB_DEBUG_INFO;
    while(it.hasNext())
    {
        it.next();
        qDebug() << SB_DEBUG_INFO << it.key() << it.value();
    }
}


bool
SBIDBase::operator ==(const SBIDBase& i) const
{
    if(
        i.itemType()==this->itemType() &&
        i.itemID()==this->itemID())
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
    QStringList out;
    QString songTitle=this->_songTitle.length() ? this->_songTitle : "<N/A>";
    QString songPerformerName=this->_songPerformerName.length() ? this->_songPerformerName : "<N/A>";
    QString albumTitle=this->_albumTitle.length() ? this->_albumTitle : "<N/A>";
    QString albumPerformerName=this->_albumPerformerName.length() ? this->_albumPerformerName : "<N/A>";
    QString playlistName=this->_playlistName.length() ? this->_playlistName : "<N/A>";
    switch(this->itemType())
    {
    case SBIDBase::sb_type_song:
        return QString("SBIDBase:%1 %2,%3:t=%4:p=%5 %6,%7:a=%8 %9,%10")
                .arg(this->type())
                .arg(this->_sb_song_id)
                .arg(this->_sb_tmp_item_id)
                .arg(songTitle)
                .arg(songPerformerName)
                .arg(this->_sb_song_performer_id)
                .arg(this->_sb_tmp_performer_id)
                .arg(albumTitle)
                .arg(this->_sb_album_id)
                .arg(this->_sb_tmp_album_id)
        ;

    case SBIDBase::sb_type_performer:
        return QString("SBIDBase:%1 %2,%3:n=%4")
                .arg(this->type())
                .arg(this->_sb_performer_id)
                .arg(this->_sb_tmp_item_id)
                .arg(performerName().length()==0?"n/a":performerName())
        ;

    case SBIDBase::sb_type_album:
        return QString("SBIDBase:%1 %2,%3:t=%4:p=%5 %6 %7")
                .arg(this->type())
                .arg(this->_sb_album_id)
                .arg(this->_sb_tmp_item_id)
                .arg(albumTitle)
                .arg(albumPerformerName)
                .arg(this->_sb_album_performer_id)
                .arg(this->_sb_tmp_performer_id)
        ;

    case SBIDBase::sb_type_chart:
        return QString("Not implemented");
        break;

    case SBIDBase::sb_type_playlist:
        return QString("SBIDBase:%1 %2,%3:n=%4")
                .arg(this->type())
                .arg(this->_sb_playlist_id)
                .arg(this->_sb_tmp_item_id)
                .arg(playlistName)
        ;
        break;

    case SBIDBase::sb_type_invalid:
        return "<INVALID ID>";

    default:
        break;
    }
    return "<not implemented in operator<< >";
}

///	PRIVATE
void
SBIDBase::_init()
{
    QString e;
    _sb_item_type=sb_type_invalid;
    _sb_mbid=e;

    _sb_song_id=-1;
    _sb_song_performer_id=-1;
    _sb_album_id=-1;
    _sb_album_performer_id=-1;
    _sb_performer_id=-1;
    _sb_album_position=-1;
    _sb_chart_id=-1;
    _sb_playlist_id=-1;
    _sb_playlist_position=-1;
    _sb_model_position=-1;
    _sb_play_position=-1;

    _originalPerformerFlag=0;
    _albumTitle=e;
    _count1=0;
    _count2=0;
    _duration=Duration();
    _genre=e;
    _lyrics=e;
    _notes=e;
    _songPerformerName=e;
    _albumPerformerName=e;
    _performerName=e;
    _playlistName=e;
    _songTitle=e;
    _url=e;
    _wiki=e;
    _year=0;
    _path=e;

    this->_sb_tmp_item_id=0;
    this->_sb_tmp_song_id=0;
    this->_sb_tmp_album_id=0;
    this->_sb_tmp_performer_id=0;

    _changedFlag=0;
    _newFlag=0;

    _deletedFlag=0;
    _mergedWithID=-1;
}
