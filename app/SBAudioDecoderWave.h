#ifndef SBAUDIODECODERWAVE_H
#define SBAUDIODECODERWAVE_H

#include "SBAudioDecoder.h"

class SBAudioDecoderWave : public SBAudioDecoder
{
public:
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

    SBAudioDecoderWave();
    virtual ~SBAudioDecoderWave();

    static bool supportFileExtension(const QString& extension);
    virtual StreamContent stream(const QString& fileName);
};

#endif // SBAUDIODECODERWAVE_H
