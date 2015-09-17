#include "SBTabSongsAll.h"

#include "Context.h"
#include "MainWindow.h"

SBTabSongsAll::SBTabSongsAll() : SBTab()
{
}

bool
SBTabSongsAll::handleEscapeKey()
{
    return 1;
}

SBID
SBTabSongsAll::populate(const SBID &id)
{
    return SBTab::populate(id);
}
