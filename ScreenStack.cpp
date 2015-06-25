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

SBID
ScreenStack::popScreen()
{
    qDebug() << SB_DEBUG_INFO << currentScreenID;
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
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    return id;
}

void
ScreenStack::pushScreen(const SBID& id)
{
    qDebug() << SB_DEBUG_INFO << id << currentScreenID;
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
    qDebug() << SB_DEBUG_INFO << id << currentScreenID;
}

SBID
ScreenStack::currentScreen()
{
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    SBID id;

    if(stack.isEmpty()==0)
    {
        id=stack.at(currentScreenID);
    }
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    return id;
}

SBID
ScreenStack::nextScreen()
{
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    SBID id;

    if(stack.isEmpty()==0)
    {
        if(currentScreenID+1<stack.length())
        {
            currentScreenID++;
        }
        id=stack.at(currentScreenID);
    }
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    return id;
}

SBID
ScreenStack::previousScreen()
{
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    SBID id;

    if(currentScreenID>0)
    {
        currentScreenID--;
        id=stack.at(currentScreenID);
    }
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    return id;
}

int
ScreenStack::getCurrentScreenID() const
{
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    return currentScreenID;
}

int
ScreenStack::getScreenCount() const
{
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    return stack.length();
}

void
ScreenStack::removeForward()
{
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    while(stack.count()-1>currentScreenID)
    {
        stack.removeLast();
    }
}

void
ScreenStack::removeScreen(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << id << currentScreenID << stack.count();
    for(int i=stack.count()-1;i>=0;i--)
    {
        qDebug() << SB_DEBUG_INFO << "i=" << i;
        const SBID& currentID=stack.at(i);
        if(currentID==id)
        {
            qDebug() << SB_DEBUG_INFO << "remove";
            stack.removeAt(i);
            if(i<=currentScreenID)
            {
                currentScreenID--;
                qDebug() << SB_DEBUG_INFO << "adjust i=" << i;
            }
        }
    }
}

void
ScreenStack::updateCurrentScreen(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    if(currentScreenID>=0 && currentScreenID<stack.count())
    {
        stack[currentScreenID]=id;
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "NO SCREENS ON STACK";
    }
    qDebug() << SB_DEBUG_INFO << currentScreenID;
}

///	PRIVATE
void
ScreenStack::init()
{
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    currentScreenID=-1;
    stack.clear();
}

void
ScreenStack::debugShow(const QString& c)
{
    qDebug() << SB_DEBUG_INFO << c << "num screens" << stack.size();
    if(stack.size()>0)
    {
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
    else
    {
        qDebug() << SB_DEBUG_INFO << "NO SCREENS ON STACK";
    }
    qDebug() << SB_DEBUG_INFO << currentScreenID;
}
