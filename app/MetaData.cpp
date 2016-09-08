#include <taglib/tpropertymap.h>
#include "Common.h"
#include "MetaData.h"


MetaData::MetaData(const QString& path)
{
    QByteArray qfn=path.toUtf8();
    _f=TagLib::FileRef(qfn.constData(),true,TagLib::AudioProperties::Accurate);
}

MetaData::~MetaData()
{

}

SBIDSong
MetaData::parse()
{
    SBIDSong s;
    s.albumTitle=Common::sanitize(TStringToQString(_f.tag()->album()));
    s.genre=Common::sanitize(TStringToQString(_f.tag()->genre()));
    s.notes=Common::sanitize(TStringToQString(_f.tag()->comment()));
    s.songPerformerName=Common::sanitize(TStringToQString(_f.tag()->artist()));
    s.songTitle=Common::sanitize(TStringToQString(_f.tag()->title()));
    s.sb_position=_f.tag()->track();
    s.year=_f.tag()->year();
    s.duration=Duration(0,0,_f.audioProperties()->length());

    return s;
}
