#ifndef AUDIODECODERWAVE_H
#define AUDIODECODERWAVE_H

#include "AudioDecoder.h"

class AudioDecoderWave : public AudioDecoder
{
    Q_OBJECT

protected:
    friend class AudioDecoderFactory;

    AudioDecoderWave(const QString& fileName);
    virtual ~AudioDecoderWave();

    static bool supportFileExtension(const QString& extension);

private:
    typedef struct WaveHeader
    {
        char    ckID[4];             /* chunk id 'RIFF'            */
        qint32  ckSize;              /* chunk size                 */
        char    wave_ckID[4];        /* wave chunk id 'WAVE'       */
        char    fmt_ckID[4];         /* format chunk id 'fmt '     */
        qint32  fmt_ckSize;          /* format chunk size          */
        qint16  formatTag;           /* format tag currently pcm   */
        qint16  nChannels;           /* number of channels         */
        qint32  nSamplesPerSec;      /* sample rate in hz          */
        qint32  nAvgBytesPerSec;     /* average bytes per second   */
        qint16  nBlockAlign;         /* number of bytes per sample */
        qint16  nBitsPerSample;      /* number of bits in a sample */
        char    data_ckID[4];        /* data chunk id 'data'       */
        qint32  data_ckSize;         /* length of data chunk       */
    } WaveHeader;

};

#endif // AUDIODECODERWAVE_H
