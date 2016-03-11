#include <ogg/ogg.h>
#include <vorbis/codec.h>

#include <QAudioDeviceInfo>
#include "QFileInfo"

#include "SBAudioDecoderFactory.h"
#include "SBMediaPlayer.h"
#include "SBMessageBox.h"


#include "Common.h"

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
        QString fn=QString(fileName).replace("\\","");
        fn="/tmp/aap.wav";
        QUrl o=QUrl::fromLocalFile(fn);
        SBAudioDecoderFactory adf;
        QIODevice* iod=adf.stream(fn);

        if(iod)
        {
            QMediaPlayer::setMedia(o,iod);
        }
        else
        {
            SBMessageBox::createSBMessageBox(QString("File `%1' cannot be opened").arg(fileName),
                                             adf.error(),
                                             QMessageBox::Critical,
                                             QMessageBox::Close,
                                             QMessageBox::Close,
                                             QMessageBox::Close
                                             );
        }
    }
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
    if(0)
    {
        //	Code to use OGG. May be moved to (a subclass of) SBAudioDecoder
        QString fn="/tmp/noot.ogg";
        QByteArray qBAFilename = fn.toUtf8();
        FILE *vorbisfile =  fopen(qBAFilename.data(), "r");

        if (!vorbisfile)
        {
            qDebug() << SB_DEBUG_ERROR << "Cannot open: " << fn;
            return 0;
        }

        if(ov_open(vorbisfile, &ovf, NULL, 0) < 0)
        {
            qDebug() << SB_DEBUG_ERROR << "Input is not OGG.";
            fileLength = 0;
            return 0;
        }

        vorbis_info* vi=ov_info(&ovf, -1);
        if(!vi)
        {
            qDebug() << SB_DEBUG_NPTR << "vorbis_info";
            return 0;
        }
        numChannels=vi->channels;
        sampleRate=vi->rate;

        if(numChannels>2)
        {
            qDebug() << SB_DEBUG_ERROR << "Can't support more than 2 channels";
            ov_clear(&ovf);
            init();
            return 0;
        }

        fileLength=ov_pcm_total(&ovf,-1)*2;
        if(!fileLength)
        {
            qDebug() << SB_DEBUG_ERROR << "Unknown file length";
            ov_clear(&ovf);
            init();
            return 0;
        }
        qDebug() << SB_DEBUG_INFO
                 << ":fileLength=" << fileLength
                 << ":numChannels=" << numChannels
                 << ":sampleRate=" << sampleRate
        ;
    }

    return 1;
}

///	Private methods
void
SBMediaPlayer::init()
{
    _playerID=-1;
    fileLength=0;
}

