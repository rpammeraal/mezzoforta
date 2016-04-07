#include <id3tag.h>

#include "SBAudioDecoderMP3.h"

#include <QFile>

SBAudioDecoderMP3::SBAudioDecoderMP3()
{
}

SBAudioDecoderMP3::~SBAudioDecoderMP3()
{
}

bool
SBAudioDecoderMP3::supportFileExtension(const QString& extension)
{
    return
        (
            extension.compare("mp3",Qt::CaseInsensitive)==0
        ) ? 1: 0;
}

StreamContent
SBAudioDecoderMP3::stream(const QString& fileName)
{
    PaSampleFormat sampleFormat=paInt16;
    StreamContent sc;
    qDebug() << SB_DEBUG_INFO << fileName;
    QFile f(fileName);
    if(!f.open(QIODevice::ReadOnly))
    {
        errStr=QString("Error opening file '%1' [%2]").arg(fileName).arg(f.error());
        sc.setErrorMsg(errStr);
        qDebug() << SB_DEBUG_ERROR << errStr;
        return sc;
    }

    //	Get size and buffer to file
    qint64 bufferLength = f.size();
    void* buffer = f.map(0, bufferLength);

    //	Open up with mad
    mad_stream* madStream=new mad_stream;
    mad_stream_init(madStream);
    mad_stream_options(madStream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(madStream,(unsigned char*)buffer,bufferLength);

    //	Decode header, parse data
    mad_header madHeader;
    mad_header_init(&madHeader);
    mad_timer_t fileLength=mad_timer_zero;
    unsigned int frameCount=0;
    unsigned int sampleRate=0;
    int numChannels=0;
    while((madStream->bufend-madStream->this_frame)>0)
    {
        if(mad_header_decode(&madHeader,madStream)==-1)
        {
            if(!MAD_RECOVERABLE (madStream->error))
            {
                break;
            }
            if(madStream->error==MAD_ERROR_LOSTSYNC)
            {
                // ignore LOSTSYNC due to ID3 tags
                int tagsize=id3_tag_query(madStream->this_frame,madStream->bufend-madStream->this_frame);
                if (tagsize>0)
                {
                    //qDebug() << "SSMP3::SSMP3() : skipping ID3 tag size " << tagsize;
                    mad_stream_skip (madStream, tagsize);
                    continue;
                }
            }

            // qDebug() << "MAD: ERR decoding header "
            //          << frameCount << ": "
            //          << mad_stream_errorstr(madStream)
            //          << " (len=" << mad_timer_count(fileLength,MAD_UNITS_MILLISECONDS)
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
        mad_timer_add(&fileLength,madHeader.duration);

        // Add frame to list of frames
        //	Not sure if this is needed
        //MadSeekFrameType* p=new MadSeekFrameType;
        //p->m_pStreamPos=(unsigned char *)madStream->this_frame;
        //p->pos = length();
        //m_qSeekList.append(p);
        frameCount++;
    }
    mad_header_finish (&madHeader);

    //	Not a valid MP3 file then
    if(frameCount==0)
    {
        errStr=QString("Unable to parse MP3 file `%1'").arg(fileName);
        sc.setErrorMsg(errStr);
        qDebug() << SB_DEBUG_ERROR << errStr;
        return sc;
    }

    //	Reposition MAD to start reading from start
    mad_stream_finish(madStream);
    mad_stream_init(madStream);
    mad_stream_options(madStream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(madStream,(unsigned char*)buffer,bufferLength);
    mad_frame* madFrame=new mad_frame;
    mad_frame_init(madFrame);
    mad_synth* madSynth=new mad_synth;
    mad_synth_init(madSynth);

    qDebug() << SB_DEBUG_INFO << "channels=" << numChannels;
    qDebug() << SB_DEBUG_INFO << "sampleRate=" << sampleRate;
    qDebug() << SB_DEBUG_INFO << "length:seconds=" << fileLength.seconds;
    qDebug() << SB_DEBUG_INFO << "frameCount=" << frameCount;

    //	Allocate memory
    int bitsPerSample=16;
    qint64 totalSamplesRounded=numChannels*sampleRate*(fileLength.seconds+1);

    qint64 streamLength=totalSamplesRounded*(bitsPerSample/8);
    void* stream=malloc(streamLength);
    qint16* samplePtr=(qint16 *)stream;

    qDebug() << SB_DEBUG_INFO << "totalSamplesRounded=" << totalSamplesRounded;
    qDebug() << SB_DEBUG_INFO << "streamLength=" << streamLength;

    //	Read MP3 stream and stow in memory
    unsigned int frameIndex=0;

    qDebug() << SB_DEBUG_INFO
             << "frameIndex=" << frameIndex
             << "frameCount=" << frameCount
    ;

    while(frameIndex<frameCount)
    {
        if(mad_frame_decode(madFrame,madStream))
        {
            if(MAD_RECOVERABLE(madStream->error))
            {
                if(madStream->error==MAD_ERROR_LOSTSYNC)
                {
                    // Ignore LOSTSYNC due to ID3 tags
                    int tagsize = id3_tag_query(madStream->this_frame, madStream->bufend - madStream->this_frame);
                    if(tagsize > 0)
                    {
                        mad_stream_skip(madStream, tagsize);
                    }
                    continue;
                }
                continue;
            }
            else if(madStream->error==MAD_ERROR_BUFLEN)
            {
                qDebug() << "MAD: buflen ERR";
                break;
            }
            else
            {
                qDebug() << "MAD: Unrecoverable frame level ERR (" << mad_stream_errorstr(madStream) << ").";
                break;
            }
        }
        ++frameIndex;
        mad_synth_frame(madSynth,madFrame);

        for (int i=0; i<madSynth->pcm.length; i++)
        {
            //	Process left channel.
            *(samplePtr++) = madScale(madSynth->pcm.samples[0][i]);

            //	Process right channel. If the decoded stream is mono then
            //	right output channel is same as left one.
            if (numChannels==2)
            {
                *(samplePtr++) = madScale(madSynth->pcm.samples[1][i]);
            }
            else
            {
                *(samplePtr++) = madScale(madSynth->pcm.samples[0][i]);
            }
        }
    }
    mad_stream_finish(madStream);
    delete madStream; madStream=NULL;
    delete madFrame; madFrame=NULL;
    delete madSynth; madSynth=NULL;

    unsigned int completed=(char *)samplePtr - (char *)stream;
    qDebug() << SB_DEBUG_INFO
             << ":frameCount=" << frameCount
             << ":frameIndex=" << frameIndex
             << ":completed=" << completed
             << ":streamLength=" << streamLength
    ;

    sc=StreamContent(stream,streamLength,numChannels,sampleRate,sampleFormat,bitsPerSample);
    return sc;
}
