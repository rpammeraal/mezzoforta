#ifndef CONTEXT_H
#define CONTEXT_H

#include <QDebug>

#include "Common.h"

class Controller;
class DataAccessLayer;
class MainWindow;
class SBModelSonglist;
class SonglistScreenHandler;


class Context
{
public:
    static Context* instance()
    {
        //	In future, we'll maintain all Context instances and
        //	return an integer to refer to a specific context
        static Context _instance;
        return &_instance;
    }

    inline Controller* getController() const { return c; }
    inline DataAccessLayer* getDataAccessLayer() const { return dal; }
    inline MainWindow* getMainWindow() const { return mw; }
    inline SBModelSonglist* getSBModelSonglist() const { return sl; }
    inline SonglistScreenHandler* getSonglistScreenHandler() const { if(!ssh) { qDebug() << SB_DEBUG_NPTR; } return ssh; }

    void setController(Controller* nc);
    void setDataAccessLayer(DataAccessLayer* ndal);
    void setMainWindow(MainWindow* nmw);
    void setSBModelSonglist(SBModelSonglist *nsl);
    void setSonglistScreenHandler(SonglistScreenHandler* nssh);

private:
    Controller* c;
    DataAccessLayer* dal;
    MainWindow* mw;
    SBModelSonglist* sl;
    SonglistScreenHandler* ssh;

    Context();
    ~Context();

    Context(Context const&);
    void operator=(Context const&);

    void init();
};

#endif // CONTEXT_H
