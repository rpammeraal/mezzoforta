//#include <ogg/ogg.h>
//#include <vorbis/codec.h>

#include <portaudio.h>

#include <QAudioDeviceInfo>
#include "QFileInfo"

#include "SBAudioDecoderFactory.h"
#include "SBMediaPlayer.h"
#include "SBMessageBox.h"

#include "Common.h"

PaStream* stream;
FILE* wavfile;
int numChannels;
int sampleRate;
PaSampleFormat sampleFormat;
int bytesPerSample, bitsPerSample;


int paStreamCallback
(
    const void *input,
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
)
{
    Q_UNUSED(input);
    Q_UNUSED(timeInfo);
    Q_UNUSED(statusFlags);
    Q_UNUSED(userData);

    size_t numRead = fread(output, bytesPerSample * numChannels, frameCount, wavfile);
    output = (qint8*)output + numRead * numChannels * bytesPerSample;
    frameCount -= numRead;

    if(frameCount > 0)
    {
        memset(output, 0, frameCount * numChannels * bytesPerSample);
        qDebug() << SB_DEBUG_INFO << "Done";
        return paComplete;
    }

    return paContinue;
}

bool portAudioOpen()
{
    int err=0;
    err=Pa_Initialize();
    if(err != paNoError)
    {
        qDebug() << SB_DEBUG_INFO
                 << "Pa_Terminate:"
                 << Pa_GetErrorText(err)
        ;
        return 0;
    }
    qDebug() << SB_DEBUG_INFO;

    PaStreamParameters outputParameters;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    if(	outputParameters.device==paNoDevice)
    {
        qDebug() << SB_DEBUG_INFO
                 << "No device available"
        ;
        return 0;
    }
    qDebug() << SB_DEBUG_INFO;

    outputParameters.channelCount = numChannels;
    outputParameters.sampleFormat = sampleFormat;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency;

//    err = Pa_OpenStream(
//        &stream,
//        NULL, // no input
//        &outputParameters,
//        sampleRate,
//        paFramesPerBufferUnspecified, // framesPerBuffer
//        0, // flags
//        &paStreamCallback,
//        NULL //void *userData
//        );
    err = Pa_OpenDefaultStream(
        &stream,
        NULL, // no input
        numChannels,
        sampleFormat,
        sampleRate,
        paFramesPerBufferUnspecified, // framesPerBuffer
        //0, // flags
        &paStreamCallback,
        NULL //void *userData
        );

    if(err != paNoError)
    {
        qDebug() << SB_DEBUG_INFO
                 << "Pa_OpenDefaultStream:"
                 << Pa_GetErrorText(err)
        ;

        if(stream)
        {
            Pa_CloseStream(stream);
        }
        return false;
    }
    qDebug() << SB_DEBUG_INFO;

    err=Pa_StartStream(stream);
    if( err != paNoError )
    {
        qDebug() << SB_DEBUG_INFO
                 << "Pa_StartStream:"
                 << Pa_GetErrorText(err)
        ;
        return 0;
    }
    return true;
}

std::string freadStr(FILE* f, size_t len) {
    std::string s(len, '\0');
    CHECK(fread(&s[0], 1, len, f) == len);
    return s;
}

template<typename T>
T freadNum(FILE* f) {
    T value;
    CHECK(fread(&value, sizeof(value), 1, f) == 1);
    return value; // no endian-swap for now... WAV is LE anyway...
}

int readFmtChunk(qint32 chunkLen) {
    CHECK(chunkLen >= 16);
    qint16 fmttag = freadNum<qint16>(wavfile); // 1: PCM (int). 3: IEEE float
    CHECK(fmttag == 1 || fmttag == 3);
    numChannels = freadNum<qint16>(wavfile);
    CHECK(numChannels > 0);
    printf("%i channels\n", numChannels);
    sampleRate = freadNum<qint32>(wavfile);
    printf("%i Hz\n", sampleRate);
    qint32 byteRate = freadNum<qint32>(wavfile);
    qint16 blockAlign = freadNum<qint16>(wavfile);
    bitsPerSample = freadNum<qint16>(wavfile);
    bytesPerSample = bitsPerSample / 8;
    CHECK(byteRate == sampleRate * numChannels * bytesPerSample);
    CHECK(blockAlign == numChannels * bytesPerSample);
    if(fmttag == 1 /*PCM*/) {
        switch(bitsPerSample) {
            case 8: sampleFormat = paInt8; break;
            case 16: sampleFormat = paInt16; break;
            case 32: sampleFormat = paInt32; break;
            default: CHECK(false);
        }
        printf("PCM %ibit int\n", bitsPerSample);
    } else {
        CHECK(fmttag == 3 /* IEEE float */);
        CHECK(bitsPerSample == 32);
        sampleFormat = paFloat32;
        printf("32bit float\n");
    }
    if(chunkLen > 16) {
        qint16 extendedSize = freadNum<qint16>(wavfile);
        CHECK(chunkLen == 18 + extendedSize);
        fseek(wavfile, extendedSize, SEEK_CUR);
    }
    return 0;
}

int doit()
{
    qDebug() << SB_DEBUG_INFO;
    wavfile = fopen("/tmp/aap.wav","r");

    if(wavfile==NULL)
    {
        qDebug() << SB_DEBUG_INFO << "wavFIle=NULL";
        return 0;
    }

    qDebug() << SB_DEBUG_INFO;

    CHECK(freadStr(wavfile, 4) == "RIFF");
    qint32 wavechunksize = freadNum<qint32>(wavfile);
    CHECK(freadStr(wavfile, 4) == "WAVE");
    while(true)
    {
        std::string chunkName = freadStr(wavfile, 4);
        qint32 chunkLen = freadNum<qint32>(wavfile);
        if(chunkName == "fmt ")
        {
            readFmtChunk(chunkLen);
        }
        else if(chunkName == "data")
        {
            CHECK(sampleRate != 0);
            CHECK(numChannels > 0);
            CHECK(bytesPerSample > 0);
            printf("len: %.0f secs\n", double(chunkLen) / sampleRate / numChannels / bytesPerSample);
            break; // start playing now
        }
        else
        {
            // skip chunk
            CHECK(fseek(wavfile, chunkLen, SEEK_CUR) == 0);
        }
    }
    qDebug() << SB_DEBUG_INFO;

    printf("start playing...\n");
    int i=portAudioOpen();
    if(i!=1)
    {
        qDebug() << SB_DEBUG_INFO << "Something went wrong with portAudioOpen";
    }


    qDebug() << SB_DEBUG_INFO;

    return 1;

    // wait until stream has finished playing
    while(Pa_IsStreamActive(stream) > 0)
    {
        QThread::usleep(1000);
    }

    qDebug() << SB_DEBUG_INFO;

    printf("finished\n");
    fclose(wavfile);
    Pa_CloseStream(stream);
    Pa_Terminate();
    return 1;
}


SBMediaPlayer::SBMediaPlayer()
{
    init();
    qDebug() << SB_DEBUG_INFO << this;
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
        doit();
    }
//    if(0)
//    {
//        QString fn=QString(fileName).replace("\\","");
//        fn="/tmp/aap.wav";
//        QUrl o=QUrl::fromLocalFile(fn);
//        SBAudioDecoderFactory adf;
//        QIODevice* iod=adf.stream(fn);

//        if(iod)
//        {
//            QMediaPlayer::setMedia(o,iod);
//        }
//        else
//        {
//            SBMessageBox::createSBMessageBox(QString("File `%1' cannot be opened").arg(fileName),
//                                             adf.error(),
//                                             QMessageBox::Critical,
//                                             QMessageBox::Close,
//                                             QMessageBox::Close,
//                                             QMessageBox::Close
//                                             );
//        }
//    }
//    if(1)
//    {
//        //	Attempt to use SBAudioDecoder
//        QString fn=QString(fileName).replace("\\","");
//        fn="/tmp/aap.wav";
//        QUrl o=QUrl::fromLocalFile(fn);
//        SBAudioDecoder ad;
//
//        ad.setSourceFilename(fn);
//        QMediaPlayer::setMedia(o,ad.stream());
//
//    }
//    if(0)
//    {
//        //	Proof of concept code
//
//        //	Unescape filename
//        QString fn=QString(fileName).replace("\\","");
//        //fn="c:/temp/mies.mp3";
//        fn="/tmp/aap.mp3";
//        QUrl o=QUrl::fromLocalFile(fn);
//        qDebug() << SB_DEBUG_INFO << o.toLocalFile();
//        QMediaPlayer::setMedia(o);
//        this->setVolume(100);
//        QMediaPlayer::play();
//
//        QAudioDeviceInfo di(QAudioDeviceInfo::defaultOutputDevice());
//        QStringList l=di.supportedCodecs();
//        qDebug() << SB_DEBUG_INFO << l;
//
//
//        return (this->error()==QMediaPlayer::NoError)?1:0;
//    }
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

    return 1;
}

///	Private methods
void
SBMediaPlayer::init()
{
    _playerID=-1;
    //fileLength=0;
}

