#ifndef CONTEXT_H
#define CONTEXT_H

#include <QDebug>

#include "Common.h"

class BackgroundThread;
class Controller;
class DataAccessLayer;
class ExternalData;
class LeftColumnChooser;
class MainWindow;
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

    inline BackgroundThread* getBackgroundThread() const { return bgt; }
    inline Controller* getController() const { return c; }
    inline DataAccessLayer* getDataAccessLayer() const { return dal; }
    inline LeftColumnChooser* getLeftColumnChooser() const { return lcc; }
    inline MainWindow* getMainWindow() const { return mw; }
    inline SonglistScreenHandler* getSonglistScreenHandler() const { if(!ssh) { qDebug() << SB_DEBUG_NPTR; } return ssh; }

    void setBackgroundThread(BackgroundThread* nbgt);
    void setController(Controller* nc);
    void setDataAccessLayer(DataAccessLayer* ndal);
    void setLeftColumnChooser(LeftColumnChooser* nlcc);
    void setMainWindow(MainWindow* nmw);
    void setSonglistScreenHandler(SonglistScreenHandler* nssh);

private:
    BackgroundThread* bgt;
    Controller* c;
    DataAccessLayer* dal;
    LeftColumnChooser* lcc;
    MainWindow* mw;
    SonglistScreenHandler* ssh;

    Context();
    ~Context();

    Context(Context const&);
    void operator=(Context const&);

    void init();
};

#endif // CONTEXT_H
