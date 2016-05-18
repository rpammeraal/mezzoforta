#ifndef AudioDecoderFLAC_H
#define AudioDecoderFLAC_H

#include <qsystemdetection.h>

#ifdef Q_OS_UNIX
#include "FLAC/all.h"

#include <QFile>

#include "AudioDecoder.h"


class AudioDecoderFlac : public AudioDecoder
{
    Q_OBJECT

public:
    //	Flac specific methods
    FLAC__StreamDecoderReadStatus flacRead(FLAC__byte buffer[], size_t *bytes);
    FLAC__StreamDecoderSeekStatus flacSeek(FLAC__uint64 offset);
    FLAC__StreamDecoderTellStatus flacTell(FLAC__uint64 *offset);
    FLAC__StreamDecoderLengthStatus flacLength(FLAC__uint64 *length);
    FLAC__bool flacEOF();
    FLAC__StreamDecoderWriteStatus flacWrite(const FLAC__Frame *frame, const FLAC__int32 *const buffer[]);
    void flacMetadata(const FLAC__StreamMetadata *metadata);
    void flacError(FLAC__StreamDecoderErrorStatus status);

protected:
    friend class AudioDecoderFactory;

    AudioDecoderFlac(const QString& fileName);
    virtual ~AudioDecoderFlac();

    static bool supportFileExtension(const QString& extension);


private:
    friend class AudioDecoderFlacReader;

    qint64               _numSamples;
    qint64               _minBlockSize;
    qint64               _maxBlockSize;
    qint64               _minFrameSize;
    qint64               _maxFrameSize;
    unsigned int         _flacBufferLength;
    FLAC__int16*         _flacBuffer;
    FLAC__int16*         _leftOverBuffer;
    FLAC__StreamDecoder* _flacDecoder;

    void init();
    virtual void exit();

    inline int getShift() const
    {
        return 16 - _bitsPerSample;
    }

    inline FLAC__int16 shift(FLAC__int32 sample) const
    {
        int shift(getShift());
        if (shift==0)
        {
            return sample;
        }
        else if(shift<0)
        {
            return sample >> abs(shift);
        }
        return sample << shift;
    };
};

#endif

#endif // AudioDecoderFLAC_H
