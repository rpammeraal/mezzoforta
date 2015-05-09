#include <QDebug>

#include "Context.h"

///	PUBLIC
void
Context::setController(Controller* nc)
{
    c=nc;
}

void
Context::setDataAccessLayer(DataAccessLayer* ndal)
{
    dal=ndal;
}

void
Context::setMainWindow(MainWindow* nmw)
{
    mw=nmw;
}

void
Context::setSBModelSonglist(SBModelSonglist *nsl)
{
    sl=nsl;
}

void
Context::setSonglistScreenHandler(SonglistScreenHandler* nssh)
{
    ssh=nssh;
}

///	PRIVATE
Context::Context()
{
    init();
}

Context::~Context()
{

}

void
Context::init()
{
    c=NULL;
    dal=NULL;
    mw=NULL;
    sl=NULL;
    ssh=NULL;
}
