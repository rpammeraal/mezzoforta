#include <taglib/tpropertymap.h>
#include "Common.h"
#include "MetaData.h"


MetaData::MetaData(const QString& path)
{
    QByteArray qfn=path.toUtf8();
    //_f=TagLib::FileRef(qfn.constData(),true,TagLib::AudioProperties::Accurate);
    //_parse();
}

MetaData::~MetaData()
{

}

void
MetaData::_parse()
{

//    if(_f.audioProperties()==NULL)
//    {
//        return;
//    }
//    _albumTitle=Common::sanitize(TStringToQString(_f.tag()->album()));
//    _genre=Common::sanitize(TStringToQString(_f.tag()->genre()));
//    _notes=Common::sanitize(TStringToQString(_f.tag()->comment()));
//    _songPerformerName=Common::sanitize(TStringToQString(_f.tag()->artist()));
//    _songTitle=Common::sanitize(TStringToQString(_f.tag()->title()));
//    _albumPosition=_f.tag()->track();
//    _year=_f.tag()->year();
//    _duration=SBDuration(0,0,_f.audioProperties()->length());
}
