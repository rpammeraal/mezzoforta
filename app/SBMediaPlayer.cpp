#include <string.h>
#include <portaudio.h>

#include <QAudioDeviceInfo>
#include <QCoreApplication>
#include "QFileInfo"
#include <QThread>

#include "AudioDecoder.h"
#include "AudioDecoderFactory.h"
#include "SBMediaPlayer.h"

#include "Common.h"

int staticPaCallBack
(
    const void *input,
    void *output,
    unsigned long sampleCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
)
{
    return ((SBMediaPlayer*)userData)->paCallback(input,output,sampleCount,timeInfo,statusFlags);
}

SBMediaPlayer::SBMediaPlayer()
{
    init();
}

SBMediaPlayer::~SBMediaPlayer()
{
    closeStream();
}

void
SBMediaPlayer::assignID(int playerID)
{
    _playerID=playerID;
}

bool
SBMediaPlayer::setMedia(const QString &fileName)
{
    closeStream();
    QString fn=QString(fileName).replace("\\","");

    if(_stream)
    {
        closeStream();
    }
    AudioDecoderFactory adf;
    _ad=adf.openFile(fn);

    if(!_ad)
    {
        qDebug() << SB_DEBUG_ERROR << fileName;
        setErrorMsg(adf.error());
        return 0;
    }
    if(_ad->error().length())
    {
        setErrorMsg(_ad->error());
        return 0;
    }
    portAudioOpen(_ad);

    return 1;
}

QMediaPlayer::State
SBMediaPlayer::state() const
{
    return _state;
}

int
SBMediaPlayer::paCallback
(
    const void *input,
    void *output,
    unsigned long sampleCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags
)
{
    Q_UNUSED(input);
    Q_UNUSED(timeInfo);
    Q_UNUSED(statusFlags);
    int resultCode;

    if(!_threadPrioritySetFlag)
    {
        QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
        _threadPrioritySetFlag=1;
    }

    //const PaStreamInfo* si=Pa_GetStreamInfo(_stream);
    quint64 samplesRead=_ad->getSamples(output,sampleCount);

    if(samplesRead)
    {
        resultCode=paContinue;
        quint64 newPositionInSec=this->position()/1000;

        //	Eliminate a barrage of signals by only updating once a second.
        if(newPositionInSec!=_oldPositionInSec)
        {
            _oldPositionInSec=newPositionInSec;
            emit positionChanged(this->position());
        }
    }
    else
    {
        memset(output, 0, sampleCount * _ad->numChannels() * (_ad->bitsPerSample()/8));
        resultCode=paComplete;
        setState(QMediaPlayer::StoppedState);
    }

    /*
     * CODE TO READ FROM MEMORY MAPPED STREAM -- DON'T DELETE
     *
    quint64 toRead=(_sc.bitsPerSample()/8) * _sc.numChannels() * sampleCount;


    if(_index+toRead>_sc.length())
    {
        toRead=_sc.length()-_index;
    }
    if(toRead)
    {
        memcpy(output,(char *)_sc.data()+_index,toRead);
        _index+=toRead;
        resultCode=paContinue;
        emit positionChanged(this->position());
    }
    else
    {
        memset(output, 0, sampleCount * _sc.numChannels() * (_sc.bitsPerSample()/8));
        qDebug() << SB_DEBUG_INFO << "Done";
        resultCode=paComplete;
        setState(QMediaPlayer::StoppedState);
    }
     *
     * END
     */
    QCoreApplication::processEvents();
    return resultCode;
}

quint64
SBMediaPlayer::position() const
{
    return (_ad==NULL)?0:_ad->index2MS(_ad->getIndex());
}

///	Public slots
void
SBMediaPlayer::play()
{
    _paError=Pa_StartStream(_stream);
    if( _paError != paNoError )
    {
        setErrorMsg(Pa_GetErrorText(_paError));
        qDebug() << SB_DEBUG_ERROR << Pa_GetErrorText(_paError);
        return;
    }
    setState(QMediaPlayer::PlayingState);
}

void
SBMediaPlayer::pause()
{
    Pa_StopStream(_stream);
    setState(QMediaPlayer::PausedState);
}

void
SBMediaPlayer::setPosition(qint64 position)
{
    if(_ad)
    {
        _ad->setPosition(position);
    }
    return;
}

void
SBMediaPlayer::stop()
{
    Pa_StopStream(_stream);
    setState(QMediaPlayer::StoppedState);
}

///	Private methods
void
SBMediaPlayer::closeStream()
{
    if(_stream)
    {
        Pa_CloseStream(_stream); _stream=NULL;
    }
    if(_ad)
    {
        delete _ad; _ad=NULL;
    }
}

void
SBMediaPlayer::init()
{
    _ad=NULL;
    _playerID=-1;
    _portAudioInitFlag=0;
    _paError=paNoError;
    _stream=NULL;
    _state=QMediaPlayer::StoppedState;
    _threadPrioritySetFlag=0;
    _hasErrorFlag=0;
    _oldPositionInSec=0;

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
            qDebug() << SB_DEBUG_ERROR
                     << Pa_GetErrorText(_paError)
            ;
            return;
        }
    }
    _portAudioInitFlag=1;
}

bool
SBMediaPlayer::portAudioOpen(AudioDecoder* ad)
{
    if(_paError!=paNoError)
    {
        qDebug() << SB_DEBUG_ERROR
                 << Pa_GetErrorText(_paError)
        ;
        return 0;
    }

    ////////////////////
    ///
    /// tmp code to query audio devices
    ///
    ////////////////////
//    int numDevices=Pa_GetDeviceCount();
//    for(int i=0;i<numDevices;i++)
//    {
//        const PaDeviceInfo* devInfo=Pa_GetDeviceInfo(i);
//        qDebug() << SB_DEBUG_INFO
//                 << devInfo->name
//                 << devInfo->maxOutputChannels
//                 << devInfo->hostApi
//                 << devInfo->defaultHighOutputLatency
//                 << devInfo->defaultLowOutputLatency
//        ;
//    }
    ////////////////////

    _ad=ad;
    PaStreamParameters outputParameters;

    outputParameters.channelCount=_ad->numChannels();
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.hostApiSpecificStreamInfo=NULL;
    outputParameters.sampleFormat=_ad->sampleFormat();
    outputParameters.suggestedLatency=128;

    if(	outputParameters.device==paNoDevice)
    {
        qDebug() << SB_DEBUG_ERROR
                 << "No device available"
        ;
        _paError=paNoDevice;
        return 0;
    }

//    for(int myChannels=0;myChannels<99;myChannels++)
//    {
//        outputParameters.channelCount=myChannels;
//        _paError=Pa_IsFormatSupported(NULL,&outputParameters,_sc.sampleRate());
//        if(_paError != paNoError)
//        {
//            qDebug() << SB_DEBUG_INFO
//                     << "Pa_IsFormatSupported:"
//                     << myChannels
//                     << Pa_GetErrorText(_paError)
//            ;
//        }
//        else
//        {
//            qDebug() << SB_DEBUG_INFO << "###############################SUPPORTED:myChannels=" << myChannels;
//        }
//    }

//    _paError = Pa_OpenStream(
//        &_stream,
//        NULL,
//        &outputParameters,
//        _ad->sampleRate(),
//        //	The following should calculate out to 512 samples with 2 channels and 16 bit per sample
//        (_blockSize * 8)/(_ad->numChannels() * _ad->bitsPerSample()),
//        paPrimeOutputBuffersUsingStreamCallback,
//        &staticPaCallBack,
//        (void *)this
//    );

    _paError = Pa_OpenDefaultStream(
        &_stream,
        0, // no input
        _ad->numChannels(),// _sc.numChannels(),
        _ad->sampleFormat(),// _sc.sampleFormat(),
        _ad->sampleRate(),// _sc.sampleRate(),
        paFramesPerBufferUnspecified, // framesPerBuffer
        //0, // flags
        &staticPaCallBack,
        (void *)this //void *userData
        );

    if(_paError != paNoError)
    {
        qDebug() << SB_DEBUG_ERROR
                 << "Pa_OpenStream:"
                 << Pa_GetErrorText(_paError)
        ;

        if(_stream)
        {
            closeStream();
        }
        return false;
    }
    emit durationChanged(_ad->lengthInMs());
    return 1;
}

void
SBMediaPlayer::setErrorMsg(const QString &errMsg)
{
    _errMsg=errMsg;
    _hasErrorFlag=1;
}

void
SBMediaPlayer::setState(QMediaPlayer::State state)
{
    _state=state;
    emit stateChanged(_state);
}
