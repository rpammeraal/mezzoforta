#include "SBTabSongsAll.h"

#include "Context.h"
#include "MainWindow.h"

SBTabSongsAll::SBTabSongsAll() : SBTab()
{
}

bool
SBTabSongsAll::handleEscapeKey() const
{
    return 0;
}

SBID
SBTabSongsAll::populate(const SBID &id)
{
    return SBTab::populate(id);
}
