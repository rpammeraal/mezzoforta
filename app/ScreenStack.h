#ifndef SCREENSTACK_H
#define SCREENSTACK_H

#include <QList>

#include <ScreenItem.h>

///
/// \brief The ScreenStack class
///
/// Maintains a traversible stack of screens that has been visited,
/// until home() is being called, which resets the screen stack.
///
class ScreenStack : public QObject
{
    Q_OBJECT

public:
    ScreenStack();
    ~ScreenStack();

    void clear();
    int count() const;
    ScreenItem popScreen();
    void pushScreen(const ScreenItem& id);

    ScreenItem currentScreen() const;
    ScreenItem nextScreen();
    ScreenItem previousScreen();

    int getCurrentScreenID() const;
    int getScreenCount() const;
    void replace(const ScreenItem& from, const ScreenItem& to);
    void removeCurrentScreen();
    void removeForward();
    void removeScreen(const ScreenItem& id,bool editOnlyFlag=0);
    void updateCurrentScreen(const ScreenItem& id);
    void updateSBIDInStack(const ScreenItem& id);

    void debugShow(const QString& c);

public slots:
    void schemaChanged();

protected:
    friend class Context;
    void doInit();	//	Init done by Context::

private:
    int _currentScreenID;
    QList<ScreenItem> _stack;
    bool _initDoneFlag;

    void _init();
};

#endif // SCREENSTACK_H
