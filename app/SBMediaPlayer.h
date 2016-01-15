#ifndef SBMEDIAPLAYER_H
#define SBMEDIAPLAYER_H

#include <QMediaPlayer>

class SBMediaPlayer : public QMediaPlayer
{

    Q_OBJECT

public:
    SBMediaPlayer();

    void assignID(int playerID);
    virtual bool setMedia(const QString& fileName);

private:
    int _playerID;

    void init();
};

#endif // SBMEDIAPLAYER_H
