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
}

void
ScreenStack::pushScreen(const SBID& id)
{
    bool doPush=0;

    if(stack.count()==0)
    {
        doPush=1;
    }
    else
    {
        SBID current=currentScreen();
        if(!(current==id))
        {
            doPush=1;
            while(getCurrentScreenID()+1<=getScreenCount()-1)
            {
                popScreen();
            }
        }
    }

    if(doPush)
    {
        stack.append(id);
        currentScreenID++;
    }
}

SBID
ScreenStack::popScreen()
{
    SBID id;

    if(stack.isEmpty()==0)
    {
        id=currentScreen();
        stack.removeLast();

        if(currentScreenID>getScreenCount()-1)
        {
            currentScreenID=getScreenCount()-1;
        }
    }
    return id;
}

SBID
ScreenStack::currentScreen()
{
    SBID id;

    if(stack.isEmpty()==0)
    {
        id=stack.at(currentScreenID);
    }
    return id;
}

SBID
ScreenStack::nextScreen()
{
    SBID id;

    if(stack.isEmpty()==0)
    {
        if(currentScreenID+1<stack.length())
        {
            currentScreenID++;
        }
        id=stack.at(currentScreenID);
    }
    return id;
}

SBID
ScreenStack::previousScreen()
{
    SBID id;

    if(currentScreenID>0)
    {
        currentScreenID--;
        id=stack.at(currentScreenID);
    }
    return id;
}

int
ScreenStack::getCurrentScreenID() const
{
    return currentScreenID;
}

int
ScreenStack::getScreenCount() const
{
    return stack.length();
}

void
ScreenStack::updateCurrentScreen(const SBID &id)
{
    stack[currentScreenID]=id;
}

///	PRIVATE
void
ScreenStack::init()
{
    currentScreenID=-1;
    stack.clear();
}

void
ScreenStack::debugShow(const QString& c)
{
    qDebug() << SB_DEBUG_INFO << c;
    for(int i=0; i<stack.size(); i++)
    {
        if(currentScreenID==i)
        {
            qDebug() << SB_DEBUG_INFO << "***CURRENT***" << i << stack.at(i) << "wiki=" << stack.at(i).wiki;
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << "             " << i << stack.at(i) << "wiki=" << stack.at(i).wiki;
        }
    }
}
