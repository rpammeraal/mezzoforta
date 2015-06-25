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
Context::setLeftColumnChooser(LeftColumnChooser *nlcc)
{
    lcc=nlcc;
}

void
Context::setMainWindow(MainWindow* nmw)
{
    mw=nmw;
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
    lcc=NULL;
    mw=NULL;
    ssh=NULL;
}
