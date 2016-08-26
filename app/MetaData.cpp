#include "Common.h"
#include "MetaData.h"


MetaData::MetaData(const QString& path)
{
    QByteArray qfn=path.toUtf8();
    _f=TagLib::FileRef(qfn.constData());
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
    s.performerName=Common::sanitize(TStringToQString(_f.tag()->artist()));
    s.songTitle=Common::sanitize(TStringToQString(_f.tag()->title()));
    s.year=_f.tag()->year();

    return s;
}
