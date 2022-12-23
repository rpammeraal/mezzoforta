#include <portaudio.h>

#include <QAudioDeviceInfo>
#include <QCoreApplication>
#include "QFileInfo"
#include "QHBoxLayout"
#include "QLabel"
#include "QPushButton"
#include <QThread>

#include "AudioDecoder.h"
#include "AudioDecoderFactory.h"
#include "Context.h"
#include "PlayerController.h"
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
    _init();
}

SBMediaPlayer::~SBMediaPlayer()
{
    _closeStream();
}

//	Public
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

    quint64 samplesRead=_ad->getSamples(output,sampleCount);

    if(!_fiveSecondsLeftToPlay && (_ad->lengthInMs() - this->position())<5000)
    {
        qDebug() << SB_DEBUG_INFO << "5 second left";
        _fiveSecondsLeftToPlay=1;
        _playerProgressSlider->setEnabled(0);
    }
    else if(!_startNextSongSignalGiven && (_ad->lengthInMs() - this->position())<1000)
    {
        qDebug() << SB_DEBUG_INFO << "1 second left";
        //	throw signal to alert other player that we're almost done.
        emit prepareToStartNextSong(this->_playerID);
        //	only throw signal once.
        _startNextSongSignalGiven=1;
    }

    if(samplesRead)
    {
        resultCode=paContinue;
        quint64 newPositionInSec=this->position()/1000;

        //	Eliminate a barrage of signals by only updating once a second.
        if(newPositionInSec!=_oldPositionInSec)
        {
            _oldPositionInSec=newPositionInSec;
            this->_updatePosition();
            this->setPlayerVisible(1);
            qDebug() << SB_DEBUG_INFO << _playerID << newPositionInSec;
        }
    }
    else
    {
        memset(output, 0, sampleCount * _ad->numChannels() * (_ad->bitsPerSample()/8));
        resultCode=paComplete;

        _isActivePlayer=0;
        _mediaLoaded=0;
        _isReadyTogo=0;
        //	Setting the player to finished cannot be done with QT signal/slots. It is way to slow.
        PlayerController* pc=Context::instance()->playerController();
        pc->setPlayerFinished(_playerID);
        _state=QMediaPlayer::State::StoppedState;
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
        resultCode=paComplete;
        _setState(QMediaPlayer::StoppedState);
    }
     *
     * END
     */
    QCoreApplication::processEvents();
    return resultCode;
}

QString
SBMediaPlayer::path() const
{
    return _constructPath(_opPtr);
}

quint64
SBMediaPlayer::position() const
{
    return (_ad==NULL)?0:_ad->index2MS(_ad->getIndex());
}

void
SBMediaPlayer::releaseMedia()
{
    this->_closeStream();
}

bool
SBMediaPlayer::setMedia(SBIDOnlinePerformancePtr opPtr)
{
    qDebug() << SB_DEBUG_INFO << _playerID << "start";
    SB_RETURN_IF_NULL(opPtr,0);

    _opPtr=SBIDOnlinePerformancePtr();
    _state=QMediaPlayer::State::StoppedState;
    _durationTime=SBDuration();
    qDebug() << SB_DEBUG_INFO << _playerID << opPtr->path();

    if(_stream)
    {
        _closeStream();
    }
    _portAudioTerminate();
    _portAudioInit();

    AudioDecoderFactory adf;
    _ad=adf.openFile(_constructPath(opPtr));

    if(!_ad)
    {
        _setErrorMsg(adf.error());
        qDebug() << SB_DEBUG_ERROR << _getErrorMsg();
        return 0;
    }
    if(_ad->error().length())
    {
        _setErrorMsg(_ad->error());
        qDebug() << SB_DEBUG_ERROR << _getErrorMsg();
        return 0;
    }
    this->_setDuration(_ad->lengthInMs());
    _opPtr=opPtr;
    _portAudioOpen();
    _mediaLoaded=1;
    qDebug() << SB_DEBUG_INFO << _playerID << "_mediaLoaded=" << _mediaLoaded;

    return 1;
}

///	Public slots
void
SBMediaPlayer::playOnCue()
{
    PlayerController* pc=Context::instance()->playerController();
    int pID=pc->getCurrentPlayerID();
    int i=0;	//	This can be removed later on.
    qDebug() << SB_DEBUG_INFO << _playerID << "start:playerPlaying=" << pID;
    while(pID!=-1)
    {
        if(i++ % 50000==0)
        {
            qDebug() << SB_DEBUG_INFO << _playerID << "waiting:PlayerController:playerPlayerID=" << pID;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        pID=pc->getPlayerPlayingID();
    }
    qDebug() << SB_DEBUG_INFO << _playerID << "_mediaLoaded" << _mediaLoaded;
    qDebug() << SB_DEBUG_INFO << _playerID << "calling startPlay()";
    this->startPlay();
}

int
SBMediaPlayer::startPlay()
{
    if(!_mediaLoaded)
    {
        qDebug() << SB_DEBUG_INFO << _playerID << "_mediaLoaded=" << _mediaLoaded;
        qDebug() << SB_DEBUG_WARNING << _playerID << "no media loaded";
        emit needMoreSongs();
        return 0;
    }
    if(_isReadyTogo)
    {
        qDebug() << SB_DEBUG_ERROR << _playerID << "Player already going";
        return 0;
    }
    qDebug() << SB_DEBUG_INFO << _playerID << "start";
    _isActivePlayer=1;
    _fiveSecondsLeftToPlay=0;
    _playerProgressSlider->setEnabled(1);
    _startNextSongSignalGiven=0;
    qDebug() << SB_DEBUG_INFO << _playerID << "path=" << path();
    _paError=Pa_StartStream(_stream);
    if( _paError != paNoError )
    {
        _setErrorMsg(Pa_GetErrorText(_paError));
        qDebug() << SB_DEBUG_ERROR << Pa_GetErrorText(_paError);
        return 0;
    }
    qDebug() << SB_DEBUG_INFO << _playerID;
    _setState(QMediaPlayer::PlayingState);
    _isReadyTogo=1;
    qDebug() << SB_DEBUG_INFO << _playerID << "Setting playerPlaying";
    emit weArePlayingTime2LoadNextSong(_playerID,_opPtr);
    return 1;
}

void
SBMediaPlayer::play()
{
    if(_checkReadyTogoStatus() && _progressSliderMovable())
    {
        qDebug() << SB_DEBUG_INFO << _playerID;
        Pa_StartStream(_stream);
        _setState(QMediaPlayer::PlayingState);
    }
}

void
SBMediaPlayer::pause()
{
    if(_checkReadyTogoStatus() && _progressSliderMovable())
    {
        qDebug() << SB_DEBUG_INFO << _playerID;
        Pa_StopStream(_stream);
        _setState(QMediaPlayer::PausedState);
    }
}

void
SBMediaPlayer::setPosition(qint64 position)
{
    //	setPosition is controlled by sliding da slider.
    if(_ad && _progressSliderMovable())
    {
        _ad->setPosition(position);
    }
}

void
SBMediaPlayer::stop()
{
    if(_checkReadyTogoStatus() && _progressSliderMovable())
    {
        qDebug() << SB_DEBUG_INFO << _playerID;
        Pa_StopStream(_stream);
        _setState(QMediaPlayer::StoppedState);
        _playerProgressSlider->setValue(0);
    }
}

///	Protected methods
void
SBMediaPlayer::resetPlayer()
{
    _resetPlayer();
}

void
SBMediaPlayer::refreshPlayingNowData() const
{
    QString playState;
    qDebug() << SB_DEBUG_INFO;
    switch(_state)
    {
    case QMediaPlayer::State::StoppedState:
        _playerPlayButton->setText(">");
        playState="Stopped";
        break;

    case QMediaPlayer::State::PlayingState:
        _playerPlayButton->setText("||");
        playState="Now Playing";
        break;

    case QMediaPlayer::State::PausedState:
        _playerPlayButton->setText(">");
        playState="Paused";
        break;
    }
    qDebug() << SB_DEBUG_INFO;

    playState="<BODY BGCOLOR=\"#f0f0f0\"><CENTER>"+playState;
    {
        QString apNotes;
        SBIDAlbumPerformancePtr apPtr;

        if(_opPtr)
        {
            playState=playState+": ";
            apPtr=SBIDAlbumPerformancePtr();

            if(apPtr)
            {
                apNotes=apPtr->notes();
            }
            if(apNotes.length()!=0)
            {
                apNotes=QString(" [%1]").arg(apNotes);
            }

            playState+=QString(
                "<A HREF=\"%1_%2\">%3 %10</A> by "
                "<A HREF=\"%4_%5\">%6</A> from the "
                "<A HREF=\"%7_%8\">'%9'</A> album")
                .arg(SBKey::Song)
                .arg(_opPtr->songID())
                .arg(_opPtr->songTitle())

                .arg(SBKey::Performer)
                .arg(_opPtr->songPerformerID())
                .arg(_opPtr->songPerformerName())

                .arg(SBKey::Album)
                .arg(_opPtr->albumID())
                .arg(_opPtr->albumTitle())

                .arg(apNotes)
            ;
        }
    }
    qDebug() << SB_DEBUG_INFO;
    playState+="</CENTER></BODY>";
    qDebug() << SB_DEBUG_INFO << playState;
    _setPlayerDataLabel(playState);
    qDebug() << SB_DEBUG_INFO;
}

///	Private methods
int
SBMediaPlayer::_checkReadyTogoStatus() const
{
    if(_isReadyTogo)
    {
        return 1;
    }
    qDebug() << SB_DEBUG_WARNING << _playerID  << "not ready to go";
    return 0;
}

void
SBMediaPlayer::_closeStream()
{
    if(_stream)
    {
        Pa_CloseStream(_stream);
    }
#ifndef Q_OS_LINUX
    //  The following code crashes on Ubuntu 20.04. This may be the cause of
    //  upgraded libraries.
    if(_ad)
    {
        delete _ad; _ad=NULL;
    }
#endif
    _ad=NULL;
    _stream=NULL;
    _mediaLoaded=0;
    _isReadyTogo=0;
    _opPtr=nullptr;
}

QString
SBMediaPlayer::_constructPath(SBIDOnlinePerformancePtr opPtr) const
{
    QString path;
    if(opPtr!=SBIDOnlinePerformancePtr())
    {
        PropertiesPtr p=Context::instance()->properties();
        path=QString("%1/%2")
                    .arg(p->musicLibraryDirectorySchema())
                    .arg(opPtr->path());
        path=path.replace("\\","");
    }
    return path;

}

QString
SBMediaPlayer::_getErrorMsg() const
{
    return _errMsg;
}

void
SBMediaPlayer::_init()
{
    _ad=NULL;
    _portAudioInitFlag=0;
    _paError=paNoError;
    _stream=NULL;
    _state=QMediaPlayer::StoppedState;
    _threadPrioritySetFlag=0;
    _hasErrorFlag=0;
    _oldPositionInSec=0;
    _isActivePlayer=0;
    _playerID=0;
    _fiveSecondsLeftToPlay=0;
    _startNextSongSignalGiven=0;
    _opPtr=SBIDOnlinePerformancePtr();
    _mediaLoaded=0;
    _isReadyTogo=0;

    _portAudioInit();
}

void
SBMediaPlayer::_portAudioInit()
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
SBMediaPlayer::_portAudioOpen()
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
            _resetPlayer();
        }
        return false;
    }
    return 1;
}

void
SBMediaPlayer::_portAudioTerminate()
{
    if(_portAudioInitFlag)
    {
        Pa_Terminate();
    }
    _portAudioInitFlag=0;
}

int
SBMediaPlayer::_progressSliderMovable() const
{
    return _playerProgressSlider->isEnabled();
}

void
SBMediaPlayer::_resetPlayer()
{
    _closeStream();
    _portAudioTerminate();
    _paError=paNoError;
    _state=QMediaPlayer::StoppedState;
    _hasErrorFlag=0;
    _oldPositionInSec=0;
    _isActivePlayer=0;
    _fiveSecondsLeftToPlay=0;
    _startNextSongSignalGiven=0;
}

void
SBMediaPlayer::_setDuration(int durationMs)
{
    const int durationSec=durationMs/1000;
    this->_playerProgressSlider->setValue(0);
    this->_playerProgressSlider->setMaximum(durationSec);
    _durationTime=SBDuration(durationMs);
}

void
SBMediaPlayer::_setErrorMsg(const QString &errMsg)
{
    _errMsg=errMsg;
    _hasErrorFlag=1;
}

void
SBMediaPlayer::_setState(QMediaPlayer::State state)
{
    qDebug() << SB_DEBUG_INFO << _playerID << state;
    _state=state;
    refreshPlayingNowData();
}

void
SBMediaPlayer::_updatePosition()
{
    quint64 durationMS=this->position();
    QString tStr;
    const int durationSec=durationMS/1000;

    SBDuration currentTime(durationMS);
    QString format = "mm:ss";
    if(_durationTime.hour()>=1)
    {
        format = "hh:mm:ss";
    }
    tStr = currentTime.toString(SBDuration::sb_hhmmss_format) + " / " +
            _durationTime.toString(SBDuration::sb_hhmmss_format);

    this->_playerProgressSlider->setValue(durationSec);
    this->_playerDurationLabel->setText(tStr);

}

void
SBMediaPlayer::_setPlayerDataLabel(QString text) const
{
    SB_RETURN_VOID_IF_NULL(_playerDataLabel);
    _playerDataLabel->setText(text);
}
