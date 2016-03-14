#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <QByteArray>
#include <QDebug>

#include "Common.h"
#include "SBAudioDecoderOggVorbis.h"

SBAudioDecoderOggVorbis::SBAudioDecoderOggVorbis()
{
}

SBAudioDecoderOggVorbis::~SBAudioDecoderOggVorbis()
{
}


bool
SBAudioDecoderOggVorbis::supportFileExtension(const QString& extension)
{
    return
            (
                extension.compare("ogg",Qt::CaseInsensitive)==0
            ) ? 1: 0;
}

QIODevice*
SBAudioDecoderOggVorbis::stream(const QString& fileName)
{
    qDebug() << SB_DEBUG_INFO << fileName;
    QByteArray fnba=fileName.toLatin1();
    FILE *f=fopen(fnba.data(),"r");
    if(!f)
    {
        errStr=QString("Error opening file '%1': %2").arg(fileName).arg(QString(strerror(errno)));
        qDebug() << SB_DEBUG_ERROR << errStr;
        return NULL;
    }

    if (ov_test(f, &vf, NULL, 0) != 0)
    {
        //	CWIP: get error string from vorbis lib
        errStr=QString("Error opening file '%1': <format error>").arg(fileName);
        qDebug() << SB_DEBUG_ERROR << errStr;
        return NULL;
    }

    return NULL;
}
