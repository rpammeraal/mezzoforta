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
        //fn="/tmp/aap.wav";
        //fn="C:/temp/aap.wav";
        //fn="/tmp/noot.ogg";
        //fn="C:/temp/noot.ogg";
        //fn="/tmp/mies.mp3";
        fn="C:/temp/mies.mp3";
        SBAudioDecoderFactory adf;
        StreamContent sc=adf.stream(fn);

        if(sc.hasErrorFlag())
        {
            setErrorMsg(sc.errorMsg());
            qDebug() << SB_DEBUG_INFO << sc.errorMsg();
            return 0;
        }
        portAudioOpen(sc);
        play();
    }
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

    if(this->position() % 1000 <= 10)
    {
        qDebug() << SB_DEBUG_INFO << "toRead=" << toRead
                 << ":frameCount=" <<frameCount
                 << ":_index=" << _index
                 << ":position=" << this->position()
                 << this->position() % 1000
        ;
    }

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
        memset(output, 0, frameCount * _sc.numChannels() * (_sc.bitsPerSample()/8));
        qDebug() << SB_DEBUG_INFO << "Done";
        resultCode=paComplete;
        setState(QMediaPlayer::StoppedState);
    }
    return resultCode;
}

qint64
SBMediaPlayer::position() const
{
    //	CWIP: probably incorrect
    return index2PositionInMS(_index);
}

///	Public slots
void
SBMediaPlayer::play()
{
    qDebug() << SB_DEBUG_INFO;
    _paError=Pa_StartStream(_stream);
    qDebug() << SB_DEBUG_INFO;
    if( _paError != paNoError )
    {
        setErrorMsg(Pa_GetErrorText(_paError));
        qDebug() << SB_DEBUG_INFO
                 << "Pa_StartStream:"
                 << Pa_GetErrorText(_paError)
        ;
        return;
    }
    qDebug() << SB_DEBUG_INFO;
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
    qDebug() << SB_DEBUG_INFO << "position=" << position;
    _index=( position * _sc.bitsPerSample() * _sc.numChannels() * _sc.sampleRate()/8)/1000;
}

void
SBMediaPlayer::stop()
{
    Pa_StopStream(_stream);
    setState(QMediaPlayer::StoppedState);
}

///	Private methods
qint64
SBMediaPlayer::index2PositionInMS(qint64 index) const
{
    return index * 1000 / ( _sc.bitsPerSample() * _sc.numChannels() * _sc.sampleRate()/8);
}

void
SBMediaPlayer::init()
{
    _playerID=-1;
    _index=0;
    _portAudioInitFlag=0;
    _paError=paNoError;
    _stream=NULL;
    _state=QMediaPlayer::StoppedState;
    _hasErrorFlag=0;

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
    qDebug() << SB_DEBUG_INFO;
    if(_paError!=paNoError)
    {
        return 0;
    }
    qDebug() << SB_DEBUG_INFO;

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
    emit durationChanged(index2PositionInMS(_sc.length()));
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
