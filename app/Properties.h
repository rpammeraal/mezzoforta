#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QString>

class Properties
{
public:
    QString localHostName() const;
    QString musicLibraryDirectory(bool interactiveFlag=1);
    QString musicLibraryDirectorySchema();
    void setMusicLibraryDirectory();

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

private:

    Properties();
    int _getHostID() const;
};

#endif // PROPERTIES_H
