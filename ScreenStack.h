#ifndef SCREENSTACK_H
#define SCREENSTACK_H

#include <QList>

#include <SBID.h>

///
/// \brief The ScreenStack class
///
/// Maintains a traversible stack of screens that has been visited,
/// until home() is being called, which resets the screen stack.
///
class ScreenStack
{
public:
    ScreenStack();
    ~ScreenStack();

    void clear();
    void pushScreen(const SBID& id);

    const SBID& currentScreen();
    const SBID& nextScreen();
    const SBID& previousScreen();

    int getCurrentScreenID() const;
    int getScreenCount() const;

private:
    int currentScreenID;
    QList<SBID> stack;

    void init();
    void debugShow();
};

#endif // SCREENSTACK_H
