#include <QDebug>

#include "Common.h"
#include "AudioDecoderFlac.h"
#include "AudioDecoderFlacReader.h"


#ifdef Q_OS_UNIX

//	Callback functions -- locally declared and defined.
FLAC__StreamDecoderReadStatus
FLAC_read_cb(const FLAC__StreamDecoder*, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
    return ((AudioDecoderFlac *) client_data)->flacRead(buffer, bytes);
}

FLAC__StreamDecoderSeekStatus
FLAC_seek_cb(const FLAC__StreamDecoder*, FLAC__uint64 absolute_byte_offset, void *client_data)
{
    return ((AudioDecoderFlac *) client_data)->flacSeek(absolute_byte_offset);
}

FLAC__StreamDecoderTellStatus
FLAC_tell_cb(const FLAC__StreamDecoder*, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
    return ((AudioDecoderFlac *) client_data)->flacTell(absolute_byte_offset);
}

FLAC__StreamDecoderLengthStatus
FLAC_length_cb(const FLAC__StreamDecoder*, FLAC__uint64 *stream_length, void *client_data)
{
    return ((AudioDecoderFlac *) client_data)->flacLength(stream_length);
}

FLAC__bool
FLAC_eof_cb(const FLAC__StreamDecoder*, void *client_data)
{
    return ((AudioDecoderFlac *) client_data)->flacEOF();
}

FLAC__StreamDecoderWriteStatus
FLAC_write_cb(const FLAC__StreamDecoder*, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
    return ((AudioDecoderFlac *) client_data)->flacWrite(frame, buffer);
}

void FLAC_metadata_cb(const FLAC__StreamDecoder*, const FLAC__StreamMetadata *metadata, void *client_data)
{
    ((AudioDecoderFlac *) client_data)->flacMetadata(metadata);
}

void FLAC_error_cb(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
    ((AudioDecoderFlac *) client_data)->flacError(status);
}

AudioDecoderFlac::AudioDecoderFlac(const QString& fileName)
{
    init();

    _file=new QFile(fileName);
    if(!_file->open(QIODevice::ReadOnly))
    {
        _error=QString("Error opening file: '%1' [%2]").arg(fileName).arg(_file->error());
        AudioDecoderFlac::exit();
        return;
    }

    _flacDecoder= FLAC__stream_decoder_new();

    FLAC__StreamDecoderInitStatus flacInitStatus
    (
        FLAC__stream_decoder_init_stream
        (
            _flacDecoder,
            FLAC_read_cb,
            FLAC_seek_cb,
            FLAC_tell_cb,
            FLAC_length_cb,
            FLAC_eof_cb,
            FLAC_write_cb,
            FLAC_metadata_cb,
            FLAC_error_cb,
            (void*) this
        )
    );

    if (flacInitStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    {
        _error="Decoder init failed.";
        qDebug() << SB_DEBUG_ERROR << _error;
        AudioDecoderFlac::exit();
        return;
    }

    if (!FLAC__stream_decoder_process_until_end_of_metadata(_flacDecoder))
    {
        _error=QString("Process to end of meta data failed -- decoder state=%1").arg(FLAC__stream_decoder_get_state(_flacDecoder));
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    if (_flacBuffer == NULL)
    {
        _flacBuffer = new FLAC__int16[_maxBlockSize * _numChannels];
    }

    if (_leftOverBuffer == NULL)
    {
        _leftOverBuffer = new FLAC__int16[_maxBlockSize * _numChannels];
    }


    //	Set stream parameters
    //	1.	_bitsPerSample
    //	Taken care of by call to flacMetadata()

    //	2.	_length
    _length=_numSamples*(_bitsPerSample/8);

    //	3.	_numChannels
    //	Taken care of by call to flacMetadata()

    //	4.	_sampleRate
    //	Taken care of by call to flacMetadata()

    //	5.	_sampleFormat
    qDebug() << SB_DEBUG_INFO << _bitsPerSample << _numChannels << _bitsPerSample * _numChannels;
    switch(_bitsPerSample * _numChannels)
    {
        case 24:
        _sampleFormat=paInt24;
        break;

        case 32:
        _sampleFormat=paInt16;
        break;

    default:
        _error=QString("Unknown sample rate: bps=%1, numChannels=%2").arg(_bitsPerSample).arg(_numChannels);
        return;
    }

    //	Allocate buffer to store stream in
    _stream=(char *)malloc(this->lengthInBytes());

    //	Put reader to work.
    AudioDecoderReader* adr;	//	No need to keep track of this, as the thread, running this instance, owns this instance.
    adr=new AudioDecoderFlacReader(this);
    adr->moveToThread(&_workerThread);
    connect(&_workerThread, &QThread::finished, adr, &QObject::deleteLater);
    connect(this, &AudioDecoderFlac::startBackfill, adr, &AudioDecoderReader::backFill);
    _workerThread.start();
    emit startBackfill();
}

AudioDecoderFlac::~AudioDecoderFlac()
{
    _workerThread.exit();
    _workerThread.wait();
    exit();
}

bool
AudioDecoderFlac::supportFileExtension(const QString& extension)
{
    return
        (
            extension.compare("flc",Qt::CaseInsensitive)==0 ||
            extension.compare("flac",Qt::CaseInsensitive)==0
        ) ? 1: 0;
}

//	Flac specific methods
FLAC__StreamDecoderReadStatus
AudioDecoderFlac::flacRead(FLAC__byte buffer[], size_t *bytes)
{
    SB_DEBUG_IF_NULL(_file);
    SB_DEBUG_IF_NULL(bytes);
    *bytes = _file->read((char*) buffer, *bytes);
    if (*bytes > 0)
    {
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }
    else if (*bytes == 0)
    {
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    else
    {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
}

FLAC__StreamDecoderSeekStatus
AudioDecoderFlac::flacSeek(FLAC__uint64 offset)
{
    SB_DEBUG_IF_NULL(_file);
    if (_file->seek(offset))
    {
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    }
    else
    {
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
    }
}

FLAC__StreamDecoderTellStatus
AudioDecoderFlac::flacTell(FLAC__uint64 *offset)
{
    SB_DEBUG_IF_NULL(_file);
    SB_DEBUG_IF_NULL(offset);
    if (_file->isSequential())
    {
        return FLAC__STREAM_DECODER_TELL_STATUS_UNSUPPORTED;
    }
    *offset = _file->pos();
    return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus
AudioDecoderFlac::flacLength(FLAC__uint64 *length)
{
    SB_DEBUG_IF_NULL(_file);
    SB_DEBUG_IF_NULL(length);
    if (_file->isSequential())
    {
        return FLAC__STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED;
    }
    *length = _file->size();
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__bool
AudioDecoderFlac::flacEOF()
{
    SB_DEBUG_IF_NULL(_file);
    if (_file->isSequential())
    {
        return false;
    }
    return _file->atEnd();
}

FLAC__StreamDecoderWriteStatus
AudioDecoderFlac::flacWrite(const FLAC__Frame *frame, const FLAC__int32 *const buffer[])
{
    SB_DEBUG_IF_NULL(_flacBuffer);
    SB_DEBUG_IF_NULL(frame);
    SB_DEBUG_IF_NULL(buffer);
    unsigned int i(0);
    _flacBufferLength = 0;
    if (frame->header.channels > 1)
    {
        //	Stereo (or greater)
        for(i = 0; i < frame->header.blocksize; ++i)
        {
            _flacBuffer[_flacBufferLength++] = shift(buffer[0][i]);
            _flacBuffer[_flacBufferLength++] = shift(buffer[1][i]);
        }
    }
    else
    {
        // 	Mono
        for(i = 0; i < frame->header.blocksize; ++i)
        {
            _flacBuffer[_flacBufferLength++] = shift(buffer[0][i]);
            _flacBuffer[_flacBufferLength++] = shift(buffer[0][i]);
        }
    }
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void
AudioDecoderFlac::flacMetadata(const FLAC__StreamMetadata *metaData)
{
    SB_DEBUG_IF_NULL(metaData);
    if(metaData->type==FLAC__METADATA_TYPE_STREAMINFO)
    {
        _numChannels = metaData->data.stream_info.channels;
        _numSamples = metaData->data.stream_info.total_samples * _numChannels;
        _sampleRate = metaData->data.stream_info.sample_rate;
        _bitsPerSample = metaData->data.stream_info.bits_per_sample;
        _minBlockSize = metaData->data.stream_info.min_blocksize;
        _maxBlockSize = metaData->data.stream_info.max_blocksize;
        _minFrameSize = metaData->data.stream_info.min_framesize;
        _maxFrameSize = metaData->data.stream_info.max_framesize;
    }
}

void
AudioDecoderFlac::flacError(FLAC__StreamDecoderErrorStatus status)
{
    QString error;
    switch (status)
    {
#ifndef Q_OS_LINUX
    case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA:
        error = "STREAM_DECODER_ERROR_STATUS_BAD_METADATA";
        break;
#endif
    case FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC:
        error = "STREAM_DECODER_ERROR_STATUS_LOST_SYNC";
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER:
        error = "STREAM_DECODER_ERROR_STATUS_BAD_HEADER";
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH:
        error = "STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH";
        break;
    case FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM:
        error = "STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM";
        break;
    }
    qWarning()
            << "SSFLAC got error" << error
            << "from libFLAC for file" << _file->fileName();
}

void
AudioDecoderFlac::init()
{
    _numSamples=0;
    _minBlockSize=0;
    _maxBlockSize=0;
    _minFrameSize=0;
    _maxFrameSize=0;
    _flacBuffer=NULL;
    _flacBufferLength=0;
    _flacDecoder=NULL;
    _leftOverBuffer=NULL;
}

void
AudioDecoderFlac::exit()
{
    if(_flacBuffer)
    {
        delete(_flacBuffer);
        _flacBuffer=NULL;
    }
    if(_leftOverBuffer)
    {
        delete(_leftOverBuffer);
        _leftOverBuffer=NULL;
    }
    if(_flacDecoder)
    {
        FLAC__stream_decoder_finish(_flacDecoder);
        FLAC__stream_decoder_delete(_flacDecoder);
        _flacDecoder=NULL;
    }
    init();
}

#endif
