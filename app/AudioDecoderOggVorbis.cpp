#include <errno.h>
#include <fcntl.h>

#include "AudioDecoderOggVorbis.h"
#include "AudioDecoderOggVorbisReader.h"
#include "Common.h"
#include "SBMessageBox.h"

///	Protected methods
AudioDecoderOggVorbis::AudioDecoderOggVorbis(const QString& fileName)
{
    _fileName=fileName;	//	CWIP

    //	Init. What else.
    _init();
	
	int resultCode = 0;
#ifdef Q_OS_UNIX
    //	Open file
    _file=new QFile(fileName);
    SB_DEBUG_IF_NULL(_file);

    if(!_file->open(QIODevice::ReadOnly))
    {
        _error=QString("Error opening file: '%1' [%2]").arg(fileName).arg(_file->errorString());
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    int fd=_file->handle();

    _fp=fdopen(fd,"r");
    if(_fp==NULL)
    {
        _error=QString("Cannot open file pointer: %1").arg(strerror(errno)?strerror(errno):"Unknown");
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }
#endif
#ifdef Q_OS_WIN
	
	QString windowsPath = this->convertToWindowsPath(fileName);
	QByteArray ba = windowsPath.toLocal8Bit();
	const char* c_str = ba.data();

    _fp = fopen(c_str, "rb");
    if (_fp == NULL)
	{
		_error = QString("Cannot open file pointer: %1").arg(strerror(errno) ? strerror(errno) : "Unknown");
		qDebug() << SB_DEBUG_ERROR << _error;
		return;
	}

    SB_DEBUG_IF_NULL(_fp);
	_file = new QFile(fileName);
	_file->open(QIODevice::ReadOnly);

#endif

    SB_DEBUG_IF_NULL(_fp);

#ifdef Q_OS_WIN
    //resultCode=ov_open_callbacks(_fp,&_ovf,NULL,0,OV_CALLBACKS_NOCLOSE);
    resultCode=ov_open_callbacks(_fp,&_ovf,NULL,0,OV_CALLBACKS_DEFAULT);
#endif
#ifdef Q_OS_UNIX
    resultCode=ov_open(_fp,&_ovf,NULL,0);
#endif
    if(resultCode!=0)
    {
        _error=QString("Could not open as an OGG file '%1'").arg(fileName);
        SBMessageBox::standardWarningBox(_error);
        qDebug() << SB_DEBUG_ERROR << _error << resultCode;
        return;
    }
    _ovInitialized=1;

    vorbis_info* vi=ov_info(&_ovf,-1);
    if(vi==NULL)
    {
        _error=QString("Could not get metadata from '%1'").arg(fileName);
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    if(vi->channels!=2)
    {
        _error=QString("Only 2 ogg/vorbis channels supported '%1'").arg(fileName);
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    quint64 numFrames=ov_pcm_total(&_ovf,-1);
    if(!numFrames)
    {
        _error=QString("Zero frames in '%1'").arg(fileName);
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    //	Set stream parameters
    //	1.	_bitsPerSample
    _bitsPerSample=16;	//	CWIP: to be derived from meta data at some point

    //	2.	_length
    _length=numFrames*vi->channels*(_bitsPerSample/8);

    //	3.	_numChannels
    _numChannels=vi->channels;

    //	4.	_sampleRate
    _sampleRate=vi->rate;

    //	5.	_sampleFormat
    _sampleFormat=paInt16;	//	2 bytes per sample per channel. CWIP: to be derived from meta data

    //	Set file up for reading
    _file->reset();
    _file->seek(0);
    ov_time_seek(&_ovf,0);

    //	Allocate buffer to store stream in
    _stream=(char *)malloc(this->lengthInBytes());

    //	Put reader to work.

    AudioDecoderReader* adr;	//	No need to keep track of this, as the thread, running this instance, owns this instance.
    adr=new AudioDecoderOggVorbisReader(this);
    adr->moveToThread(&_workerThread);
    adr->_fileName=this->_fileName;
    connect(&_workerThread, &QThread::finished, adr, &QObject::deleteLater);
    connect(this, &AudioDecoderOggVorbis::startBackfill, adr, &AudioDecoderReader::backFill);
    _workerThread.start();
    emit startBackfill();
}

AudioDecoderOggVorbis::~AudioDecoderOggVorbis()
{
    //	Tell reader to stop. This slows down repeatedly clicking the next song/prev song buttons
    _workerThread.exit();
    _workerThread.wait();
    if(_ovInitialized)
    {
        ov_clear(&_ovf);
        _ovInitialized=0;
    }
}

bool
AudioDecoderOggVorbis::supportFileExtension(const QString& extension)
{
    return
        (
            extension.compare("ogg",Qt::CaseInsensitive)==0
        ) ? 1: 0;
}

void
AudioDecoderOggVorbis::_init()
{
#ifdef Q_BIG_ENDIAN
    _endianity=0;
#else
    _endianity=1;
#endif
    _ovInitialized=0;
}
