#include "SBTabSongsAll.h"

#include "Context.h"
#include "MainWindow.h"
#include "ScreenStack.h"

SBTabSongsAll::SBTabSongsAll(QWidget* parent) : SBTab(parent,0)
{
}

bool
SBTabSongsAll::handleEscapeKey()
{
    init();
    return 1;
}

QTableView*
SBTabSongsAll::tableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.allSongsList;
}

///	Private methods
void
SBTabSongsAll::init()
{
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        _initDoneFlag=1;

        connect(mw->ui.allSongsList->horizontalHeader(),SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));
    }
}

SBID
SBTabSongsAll::_populate(const SBID &id)
{
    init();
    return id;
}
