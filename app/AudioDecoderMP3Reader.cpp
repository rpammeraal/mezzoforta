#include <qsystemdetection.h>
#include <QDebug>


#ifdef Q_OS_UNIX
#include <id3tag.h>
#endif

#include <mad.h>

#include "Common.h"
#include "AudioDecoderMP3Reader.h"
#include "AudioDecoderMP3.h"

AudioDecoderMP3Reader::AudioDecoderMP3Reader(AudioDecoderMP3* admp3):AudioDecoderReader(admp3)
{
}

AudioDecoderMP3Reader::~AudioDecoderMP3Reader()
{
}

void
AudioDecoderMP3Reader::backFill()
{
    qDebug() << SB_DEBUG_INFO << "start" << _fileName;

    //	Get size and buffer to file
    quint64 bufferLength = _ad->_file->size();
    void* buffer = _ad->_file->map(0, bufferLength);

    //	Open up with mad
    mad_stream madStream;
    mad_stream_init(&madStream);
    mad_stream_options(&madStream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(&madStream,(unsigned char*)buffer,bufferLength);

    //	Decode header, parse data
    mad_header madHeader;
    mad_header_init(&madHeader);
    mad_stream_init(&madStream);
    mad_stream_options(&madStream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(&madStream,(unsigned char*)buffer,bufferLength);
    mad_frame madFrame;//=new mad_frame;
    mad_frame_init(&madFrame);
    mad_synth madSynth;
    mad_synth_init(&madSynth);

    //	Read MP3 stream and stow in memory
    qint16* samplePtr=(qint16 *)(_ad->_stream);
    unsigned int frameIndex=0;

    while(frameIndex<dynamic_cast<AudioDecoderMP3 *>(_ad)->_frameCount)
    {
        if(mad_frame_decode(&madFrame,&madStream))
        {
            if(MAD_RECOVERABLE(madStream.error))
            {
                if(madStream.error==MAD_ERROR_LOSTSYNC)
                {
                    // Ignore LOSTSYNC due to ID3 tags
                    int tagsize =
#ifdef Q_OS_WIN
                            0;
#endif
#ifdef Q_OS_UNIX
                                   id3_tag_query(madStream.this_frame, madStream.bufend - madStream.this_frame);
#endif
                    if(tagsize > 0)
                    {
                        mad_stream_skip(&madStream, tagsize);
                    }
                    continue;
                }
                continue;
            }
            else if(madStream.error==MAD_ERROR_BUFLEN)
            {
                qDebug() << "MAD: buflen ERR";
                break;
            }
            else
            {
                qDebug() << "MAD: Unrecoverable frame level ERR (" << mad_stream_errorstr(&madStream) << ").";
                break;
            }
        }
        ++frameIndex;
        mad_synth_frame(&madSynth,&madFrame);

        for (int i=0; i<madSynth.pcm.length; i++)
        {
            //	Process left channel.
            *(samplePtr++) = madScale(madSynth.pcm.samples[0][i]);

            //	Process right channel. If the decoded stream is mono then
            //	right output channel is same as left one.
            if (_ad->numChannels()==2)
            {
                *(samplePtr++) = madScale(madSynth.pcm.samples[1][i]);
            }
            else
            {
                *(samplePtr++) = madScale(madSynth.pcm.samples[0][i]);
            }

            quint64 bytesReadSoFar=(char *)samplePtr-(char *)(_ad->_stream);
            if(bytesReadSoFar%1000==0)
            {
                _ad->_maxScrollableIndex=bytesReadSoFar;
            }
        }
    }
    mad_stream_finish(&madStream);
    qDebug() << SB_DEBUG_INFO << "end";
    emit QThread::currentThread()->exit();
}
