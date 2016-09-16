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
    s.setAlbumTitle(Common::sanitize(TStringToQString(_f.tag()->album())));
    s.setGenre(Common::sanitize(TStringToQString(_f.tag()->genre())));
    s.setNotes(Common::sanitize(TStringToQString(_f.tag()->comment())));
    s.setSongPerformerName(Common::sanitize(TStringToQString(_f.tag()->artist())));
    s.setSongTitle(Common::sanitize(TStringToQString(_f.tag()->title())));
    s.setAlbumPosition(_f.tag()->track());
    s.setYear(_f.tag()->year());
    s.setDuration(Duration(0,0,_f.audioProperties()->length()));

    return s;
}
