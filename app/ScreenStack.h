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
    SBID popScreen();

    SBID currentScreen();
    SBID nextScreen();
    SBID previousScreen();

    int getCurrentScreenID() const;
    int getScreenCount() const;
    void removeCurrentScreen();
    void removeForward();
    void removeScreen(const SBID& id);
    void updateCurrentScreen(const SBID& id);
    void updateSBIDInStack(const SBID& id);

    void debugShow(const QString& c);

private:
    int currentScreenID;
    QList<SBID> stack;

    void init();
};

#endif // SCREENSTACK_H
