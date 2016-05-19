#include <qsystemdetection.h>

#ifdef Q_OS_UNIX
#include <id3tag.h>
#endif

#include "AudioDecoderMP3.h"
#include "AudioDecoderMP3Reader.h"
#include "Common.h"

#include <QFile>

AudioDecoderMP3::AudioDecoderMP3(const QString& fileName)
{
    init();

    qDebug() << SB_DEBUG_INFO << fileName;
    _file=new QFile(fileName);
    if(!_file->open(QIODevice::ReadOnly))
    {
        _error=QString("Error opening file: '%1' [%2]").arg(fileName).arg(_file->error());
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    //	Get size and buffer to file
    quint64 bufferLength = _file->size();
    void* buffer = _file->map(0, bufferLength);

    //	Open up with mad
    mad_stream madStream;
    mad_stream_init(&madStream);
    mad_stream_options(&madStream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(&madStream,(unsigned char*)buffer,bufferLength);

    //	Decode header, parse data
    mad_header madHeader;
    mad_header_init(&madHeader);
    _fileLength=mad_timer_zero;
    _frameCount=0;
    unsigned int sampleRate=0;
    int numChannels=0;
    while((madStream.bufend-madStream.this_frame)>0)
    {
        if(mad_header_decode(&madHeader,&madStream)==-1)
        {
            if(!MAD_RECOVERABLE (madStream.error))
            {
                break;
            }
            if(madStream.error==MAD_ERROR_LOSTSYNC)
            {
                // ignore LOSTSYNC due to ID3 tags
                int tagsize=
#ifdef Q_OS_WIN
                            0;
#endif
#ifdef Q_OS_UNIX
                            id3_tag_query(madStream.this_frame,madStream.bufend-madStream.this_frame);
#endif
                if (tagsize>0)
                {
                    //qDebug() << "SSMP3::SSMP3() : skipping ID3 tag size " << tagsize;
                    mad_stream_skip (&madStream, tagsize);
                    continue;
                }
            }

            // qDebug() << "MAD: ERR decoding header "
            //          << _frameCount << ": "
            //          << mad_stream_errorstr(&madStream)
            //          << " (len=" << mad_timer_count(_fileLength,MAD_UNITS_MILLISECONDS)
            //          << ")";
            continue;
        }

        // Grab data from madHeader

        // This warns us only when the reported sample rate changes. (and when
        // it is first set)
        if(sampleRate==0 && madHeader.samplerate>0)
        {
            sampleRate=madHeader.samplerate;
        }
        else if(sampleRate!=madHeader.samplerate)
        {
            qDebug() << "SSMP3: file has differing samplerate in some headers:"
                     << fileName
                     << sampleRate << "vs" << madHeader.samplerate;
        }

        numChannels=MAD_NCHANNELS(&madHeader);
        mad_timer_add(&_fileLength,madHeader.duration);

        // Add frame to list of frames
        //	Not sure if this is needed
        //MadSeekFrameType* p=new MadSeekFrameType;
        //p->m_pStreamPos=(unsigned char *)madStream.this_frame;
        //p->pos = length();
        //m_qSeekList.append(p);
        _frameCount++;
    }
    mad_header_finish (&madHeader);

    //	Not a valid MP3 file then
    if(_frameCount==0)
    {
        _error=QString("Unable to parse MP3 file `%1'").arg(fileName);
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    //	Reposition MAD to start reading from start
    mad_stream_finish(&madStream);

    //	Set stream parameters
    //	1.	_bitsPerSample
    _bitsPerSample=16;	//	CWIP: to be derived from meta data at some point

    //	2.	_length
    quint64 totalSamplesRounded=numChannels*sampleRate*(_fileLength.seconds+1);
    int bitsPerSample=16;
    _length=totalSamplesRounded*(bitsPerSample/8);

    //	3.	_numChannels
    _numChannels=numChannels;

    //	4.	_sampleRate
    _sampleRate=sampleRate;

    //	5.	_sampleFormat
    _sampleFormat=paInt16;	//	2 bytes per sample per channel. CWIP: to be derived from meta data

    //	Set file up for reading
    mad_stream_finish(&madStream);

    //	Allocate buffer to store stream in
    _stream=(char *)malloc(_length);

    //	Put reader to work.
    _adr=new AudioDecoderMP3Reader(this);
    _adr->moveToThread(&_workerThread);
    connect(&_workerThread, &QThread::finished, _adr, &QObject::deleteLater);
    connect(this, &AudioDecoderMP3::startBackfill, _adr, &AudioDecoderReader::backFill);
    _workerThread.start();
    emit startBackfill();
}

AudioDecoderMP3::~AudioDecoderMP3()
{
}

bool
AudioDecoderMP3::supportFileExtension(const QString& extension)
{
    return
        (
            extension.compare("mp3",Qt::CaseInsensitive)==0
        ) ? 1: 0;
}

void
AudioDecoderMP3::init()
{
    _frameCount=0;
}
