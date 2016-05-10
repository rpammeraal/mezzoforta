//#include <errno.h>
//#include <fcntl.h>

//#include <ogg/ogg.h>
//#include <vorbis/codec.h>
//#include <vorbis/vorbisfile.h>

//#include <QByteArray>
//#include <QDebug>
//#include <QFile>

//#include "Common.h"
//#include "AudioDecoderOggVorbis.h"

//AudioDecoderOggVorbis::AudioDecoderOggVorbis()
//{
//}

//AudioDecoderOggVorbis::~AudioDecoderOggVorbis()
//{
//}


//bool
//AudioDecoderOggVorbis::supportFileExtension(const QString& extension)
//{
//    return
//        (
//            extension.compare("ogg",Qt::CaseInsensitive)==0
//        ) ? 1: 0;
//}

//StreamContent
//AudioDecoderOggVorbis::stream(const QString& fileName)
//{
//    qDebug() << SB_DEBUG_INFO << fileName << this;
//    PaSampleFormat sampleFormat=paInt16;	//	2 bytes per sample per channel.
//    int bitsPerSample=16;	//	sampleFormat and bitsPerSample should correspond with eachother
//    StreamContent sc;
//    QFile f(fileName);
//    if(!f.open(QIODevice::ReadOnly))
//    {
//        _error=QString("Error opening file '%1' [%2]").arg(fileName).arg(f.error());
//        sc.setErrorMsg(_error);
//        qDebug() << SB_DEBUG_ERROR << _error;
//        return sc;
//    }

//    OggVorbis_File ovf;
//    vorbis_info* info;
//    int resultCode=0;
//    int fd=f.handle();
//    FILE* fp=fdopen(fd,"r");

//#ifdef Q_OS_WIN
//    resultCode=ov_open_callbacks(fp,&ovf,NULL,0,OV_CALLBACKS_NOCLOSE);
//#endif
//#ifdef Q_OS_UNIX
//    resultCode=ov_open(fp,&ovf,NULL,0);
//#endif
//    if(resultCode!=0)
//    {
//        _error=QString("Could not open '%s' as a OGG file").arg(fileName);
//        sc.setErrorMsg(_error);
//        qDebug() << SB_DEBUG_ERROR << _error;
//        return sc;
//    }

//    info=ov_info(&ovf,-1);
//    if(info==NULL)
//    {
//        _error=QString("Could not get metadata from '%s'").arg(fileName);
//        sc.setErrorMsg(_error);
//        qDebug() << SB_DEBUG_NPTR << _error;
//        return sc;
//    }
//    qDebug() << SB_DEBUG_INFO << "channels=" << info->channels;
//    qDebug() << SB_DEBUG_INFO << "rate=" << info->rate;

//    char **ptr=ov_comment(&ovf,-1)->user_comments;
//    while(*ptr)
//    {
//        qDebug() << SB_DEBUG_INFO << *ptr;
//        ++ptr;
//    }
//    qDebug() << SB_DEBUG_INFO << "encoded by " <<  ov_comment(&ovf,-1)->vendor;

//    if(info->channels!=2)
//    {
//        _error=QString("Only 2 ogg/vorbis channels supported '%s'").arg(fileName);
//        sc.setErrorMsg(_error);
//        qDebug() << SB_DEBUG_NPTR << _error;
//        return sc;
//    }

//    qint64 numFrames=ov_pcm_total(&ovf,-1);
//    if(!numFrames)
//    {
//        _error=QString("Zero frames in '%s'").arg(fileName);
//        sc.setErrorMsg(_error);
//        qDebug() << SB_DEBUG_NPTR << _error;
//        return sc;
//    }

//    qint64 size=numFrames*info->channels*(bitsPerSample/8);
//    void* src=malloc(size);
//    if(!src)
//    {
//        _error=QString("Unable to allocate memory '%s'").arg(fileName);
//        sc.setErrorMsg(_error);
//        qDebug() << SB_DEBUG_NPTR << _error;
//        return sc;
//    }

//    const int bufferSize=8192;
//    int endianity;
//    int currentSection=0;
//#ifdef Q_BIG_ENDIAN
//    endianity=0;
//#else
//    endianity=1;
//#endif

//    qint64 i=0;
//    while(i<size)
//    {
//        qint64 bytesRead=ov_read(&ovf,((char *)src)+i,bufferSize,endianity,(bitsPerSample/8),1,&currentSection);

//        if(i==0)
//        {
//            qDebug() << SB_DEBUG_INFO << "Reading data:size=" << size;
//        }
//        if(bytesRead<=0)
//        {
//            _error=QString("Unable to read '%s'").arg(fileName);
//            sc.setErrorMsg(_error);
//            qDebug() << SB_DEBUG_NPTR << _error;
//            return sc;
//        }
//        i+=bytesRead;
//    }
//    sc=StreamContent(src,size,info->channels,info->rate,sampleFormat,bitsPerSample);
//    qDebug() << SB_DEBUG_INFO << "EOF";
//    ov_clear(&ovf);
//    return sc;
//}
