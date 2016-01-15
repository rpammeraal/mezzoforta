#include "SBMediaPlayer.h"

#include "QFileInfo"

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
    //	Unescape filename
    QString fn=QString(fileName).replace("\\","");
    //fn="c:/temp/mies.mp3";
    fn="/tmp/aap.mp3";
    QUrl o=QUrl::fromLocalFile(fn);
    qDebug() << SB_DEBUG_INFO << o.toLocalFile();
    QMediaPlayer::setMedia(o);
    this->setVolume(100);
    QMediaPlayer::play();

    return (this->error()==QMediaPlayer::NoError)?1:0;
}

///	Private methods
void
SBMediaPlayer::init()
{
    _playerID=-1;
}
