#include <QDebug>

#include "Context.h"

///	PUBLIC
void
Context::setBackgroundThread(BackgroundThread* nbgt)
{
    bgt=nbgt;
}

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
Context::setChooser(Chooser *nlcc)
{
    lcc=nlcc;
}

void
Context::setMainWindow(MainWindow* nmw)
{
    mw=nmw;
}

void
Context::setNavigator(Navigator* nssh)
{
    ssh=nssh;
}

void
Context::setScreenStack(ScreenStack* nst)
{
    st=nst;
}

void
Context::setTab(SBTab *ntab)
{
    tab=ntab;
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
    st=NULL;
    tab=NULL;
}
