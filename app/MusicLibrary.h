#ifndef MUSICLIBRARY_H
#define MUSICLIBRARY_H

#include <QObject>

class MusicLibrary : public QObject
{
    Q_OBJECT

public:
    explicit MusicLibrary(QObject *parent = 0);
    void rescanMusicLibrary();

signals:

public slots:

private:
    void _rescanMusicLibrary(const QString& schema);
};

#endif // MUSICLIBRARY_H
