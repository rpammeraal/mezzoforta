#include <QDebug>

#include "Common.h"
#include "ScreenStack.h"

ScreenStack::ScreenStack()
{
    init();
}

ScreenStack::~ScreenStack()
{

}

void
ScreenStack::clear()
{
    init();
    qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID << "count=" << stack.count();
}

void
ScreenStack::pushScreen(const SBID& id)
{
    bool p=0;

    if(stack.count()==0)
    {
        p=1;
    }
    else
    {
        SBID current=currentScreen();
        if(!(current==id))
        {
            p=1;
        }
    }

    if(p)
    {
        stack.append(id);
        currentScreenID++;
    }
    qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID << "count=" << stack.count();
}

const SBID&
ScreenStack::currentScreen()
{
    return stack.at(currentScreenID);
}

const SBID&
ScreenStack::nextScreen()
{
    if(currentScreenID+1<stack.length())
    {
        currentScreenID++;
    }
    return stack.at(currentScreenID);
}

const SBID&
ScreenStack::previousScreen()
{
    debugShow();
    qDebug() << SB_DEBUG_INFO << "**************************************************************";
    qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID << "count=" << stack.count();
    qDebug() << SB_DEBUG_INFO << "currentScreenID=" << stack.at(currentScreenID);
    currentScreenID--;
    qDebug() << SB_DEBUG_INFO << "currentScreenID=" << stack.at(currentScreenID);
    return stack.at(currentScreenID);
}

int
ScreenStack::getCurrentScreenID() const
{
    qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID << "count=" << stack.count();
    return currentScreenID;
}

int
ScreenStack::getScreenCount() const
{
    qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID << "count=" << stack.count();
    return stack.length();
}

///	PRIVATE
void
ScreenStack::init()
{
    currentScreenID=-1;
    stack.clear();
}

void
ScreenStack::debugShow()
{
    for(int i=0; i<stack.size(); i++)
    {
        qDebug() << SB_DEBUG_INFO << stack.at(i);
    }
}
