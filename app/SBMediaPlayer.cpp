#include <string.h>
#include <portaudio.h>

#include <QAudioDeviceInfo>
#include "QFileInfo"

#include "SBAudioDecoderFactory.h"
#include "SBMediaPlayer.h"
#include "SBMessageBox.h"
#include "StreamContent.h"

#include "Common.h"

int staticPaCallBack
(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
)
{
    return ((SBMediaPlayer*)userData)->paCallback(input,output,frameCount,timeInfo,statusFlags);
}

SBMediaPlayer::SBMediaPlayer()
{
    init();
}

SBMediaPlayer::~SBMediaPlayer()
{
    if(_stream)
    {
        Pa_CloseStream(_stream);
        _stream=NULL;
    }
}

void
SBMediaPlayer::assignID(int playerID)
{
    _playerID=playerID;
}

bool
SBMediaPlayer::setMedia(const QString &fileName)
{
    if(1)
    {
        QString fn=QString(fileName).replace("\\","");
        fn="/tmp/aap.wav";
        QUrl o=QUrl::fromLocalFile(fn);
        SBAudioDecoderFactory adf;
        StreamContent sc=adf.stream(fn);

        if(sc.hasErrorFlag())
        {
            _errorStr=sc.errorMsg();
            return 0;
        }
        portAudioOpen(adf.stream(fn));
        play();
    }
    return 1;
//    if(0)
//    {
//        //	Code to use OGG. May be moved to (a subclass of) SBAudioDecoder
//        QString fn="/tmp/noot.ogg";
//        QByteArray qBAFilename = fn.toUtf8();
//        FILE *vorbisfile =  fopen(qBAFilename.data(), "r");

//        if (!vorbisfile)
//        {
//            qDebug() << SB_DEBUG_ERROR << "Cannot open: " << fn;
//            return 0;
//        }

//        if(ov_open(vorbisfile, &ovf, NULL, 0) < 0)
//        {
//            qDebug() << SB_DEBUG_ERROR << "Input is not OGG.";
//            fileLength = 0;
//            return 0;
//        }

//        vorbis_info* vi=ov_info(&ovf, -1);
//        if(!vi)
//        {
//            qDebug() << SB_DEBUG_NPTR << "vorbis_info";
//            return 0;
//        }
//        numChannels=vi->channels;
//        sampleRate=vi->rate;

//        if(numChannels>2)
//        {
//            qDebug() << SB_DEBUG_ERROR << "Can't support more than 2 channels";
//            ov_clear(&ovf);
//            init();
//            return 0;
//        }

//        fileLength=ov_pcm_total(&ovf,-1)*2;
//        if(!fileLength)
//        {
//            qDebug() << SB_DEBUG_ERROR << "Unknown file length";
//            ov_clear(&ovf);
//            init();
//            return 0;
//        }
//        qDebug() << SB_DEBUG_INFO
//                 << ":fileLength=" << fileLength
//                 << ":numChannels=" << numChannels
//                 << ":sampleRate=" << sampleRate
//        ;
//    }
}

int
SBMediaPlayer::paCallback
(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags
)
{
    Q_UNUSED(input);
    Q_UNUSED(timeInfo);
    Q_UNUSED(statusFlags);
    int resultCode;

    qint64 toRead=(_sc.bitsPerSample()/8) * _sc.numChannels() * frameCount;

    qDebug() << SB_DEBUG_INFO
             << "toRead=" << toRead
             << ":frameCount=" <<frameCount
             << ":_index=" << _index
    ;

    if(_index+toRead>_sc.length())
    {
        toRead=_sc.length()-_index;
    }
    if(toRead)
    {
        memcpy(output,(char *)_sc.data()+_index,toRead);
        _index+=toRead;
        resultCode=paContinue;
    }
    else
    {
        memset(output, 0, frameCount * _sc.numChannels() * (_sc.bitsPerSample()/8));
        qDebug() << SB_DEBUG_INFO << "Done";
        resultCode=paComplete;

    }
    return resultCode;
}


///	Private methods
void
SBMediaPlayer::init()
{
    _playerID=-1;
    _index=0;
    _portAudioInitFlag=0;
    _paError=paNoError;
    _stream=NULL;

    portAudioInit();
}

void
SBMediaPlayer::portAudioInit()
{
    if(!_portAudioInitFlag)
    {
        _paError=Pa_Initialize();
        if(_paError != paNoError)
        {
            qDebug() << SB_DEBUG_INFO
                     << "Pa_Terminate:"
                     << Pa_GetErrorText(_paError)
            ;
            return;
        }
    }
    _portAudioInitFlag=1;
    qDebug() << SB_DEBUG_INFO;
}

bool
SBMediaPlayer::portAudioOpen(const StreamContent& sc)
{
    if(_paError!=paNoError)
    {
        return 0;
    }
    _sc=sc;
    _index=0;
    PaStreamParameters outputParameters;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if(	outputParameters.device==paNoDevice)
    {
        qDebug() << SB_DEBUG_INFO
                 << "No device available"
        ;
        _paError=paNoDevice;
        return 0;
    }
    qDebug() << SB_DEBUG_INFO;

    _paError = Pa_OpenDefaultStream(
        &_stream,
        0, // no input
        _sc.numChannels(),
        _sc.sampleFormat(),
        _sc.sampleRate(),
        paFramesPerBufferUnspecified, // framesPerBuffer
        //0, // flags
        &staticPaCallBack,
        (void *)this //void *userData
        );

    qDebug() << SB_DEBUG_INFO;
    if(_paError != paNoError)
    {
        qDebug() << SB_DEBUG_INFO
                 << "Pa_OpenDefaultStream:"
                 << Pa_GetErrorText(_paError)
        ;

        if(_stream)
        {
            Pa_CloseStream(_stream);
            _stream=NULL;
        }
        return false;
    }
    qDebug() << SB_DEBUG_INFO;
    return 1;
}

bool
SBMediaPlayer::play()
{
    _paError=Pa_StartStream(_stream);
    if( _paError != paNoError )
    {
        qDebug() << SB_DEBUG_INFO
                 << "Pa_StartStream:"
                 << Pa_GetErrorText(_paError)
        ;
        return 0;
    }
    return 1;
}
