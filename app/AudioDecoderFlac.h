#ifndef AudioDecoderFLAC_H
#define AudioDecoderFLAC_H

#include <qsystemdetection.h>

#ifdef Q_OS_UNIX
#include "FLAC/all.h"

#include <QFile>

#include "AudioDecoder.h"


class AudioDecoderFlac : public AudioDecoder
{
public:
    AudioDecoderFlac();
    virtual ~AudioDecoderFlac();

    static bool supportFileExtension(const QString& extension);
    virtual StreamContent stream(const QString& fileName);

    //	Flac specific methods
    FLAC__StreamDecoderReadStatus flacRead(FLAC__byte buffer[], size_t *bytes);
    FLAC__StreamDecoderSeekStatus flacSeek(FLAC__uint64 offset);
    FLAC__StreamDecoderTellStatus flacTell(FLAC__uint64 *offset);
    FLAC__StreamDecoderLengthStatus flacLength(FLAC__uint64 *length);
    FLAC__bool flacEOF();
    FLAC__StreamDecoderWriteStatus flacWrite(const FLAC__Frame *frame, const FLAC__int32 *const buffer[]);
    void flacMetadata(const FLAC__StreamMetadata *metadata);
    void flacError(FLAC__StreamDecoderErrorStatus status);

private:
    QFile* f;
    qint64 numSamples;
    int numChannels;
    int sampleRate;
    qint64 bitsPerSample;
    qint64 minBlockSize;
    qint64 maxBlockSize;
    qint64 minFrameSize;
    qint64 maxFrameSize;
    unsigned int flacBufferLength;
    FLAC__int16* flacBuffer;
    FLAC__int16* leftOverBuffer;

    void init();
    void exit();

    inline int getShift() const
    {
        return 16 - bitsPerSample;
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
