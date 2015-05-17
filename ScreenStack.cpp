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
    debugShow("start pushScreen");

    bool doPush=0;

    if(stack.count()==0)
    {
        doPush=1;
    }
    else
    {
        SBID current=currentScreen();
        qDebug() << SB_DEBUG_INFO << "current=" << current;
        qDebug() << SB_DEBUG_INFO << "id=" << id;
        if(!(current==id))
        {
            doPush=1;
            while(getCurrentScreenID()+1<=getScreenCount()-1)
            {
                qDebug() << SB_DEBUG_INFO << getCurrentScreenID() << getScreenCount();
                debugShow("before popScreen");
                popScreen();
                debugShow("after popScreen");
            }
        }
    }

    qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID;
    if(doPush)
    {
        stack.append(id);
        currentScreenID++;
    }
    debugShow("end pushScreen");
    qDebug() << SB_DEBUG_INFO << "end" << getCurrentScreenID() << getScreenCount();
}

SBID
ScreenStack::popScreen()
{
    qDebug() << SB_DEBUG_INFO << getCurrentScreenID() << getScreenCount();

        qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID;
    SBID id;

        qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID;
    if(stack.isEmpty()==0)
    {
        qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID;
        id=currentScreen();
        qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID;
        stack.removeLast();
        qDebug() << SB_DEBUG_INFO << "currentScreenID=" << currentScreenID;

        if(currentScreenID>getScreenCount()-1)
        {
            currentScreenID=getScreenCount()-1;
        }
    }
    qDebug() << SB_DEBUG_INFO << getCurrentScreenID() << getScreenCount();
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
    qDebug() << SB_DEBUG_INFO << id;
    return id;
}

SBID
ScreenStack::nextScreen()
{
    qDebug() << SB_DEBUG_INFO << getCurrentScreenID() << getScreenCount();

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
    qDebug() << SB_DEBUG_INFO << getCurrentScreenID() << getScreenCount();

    SBID id;

    if(currentScreenID>0)
    {
        currentScreenID--;
        id=stack.at(currentScreenID);
    }
    debugShow("end previousScreen");
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
    qDebug() << SB_DEBUG_INFO << "*************:start debugShow()" << c << "curr=" << getCurrentScreenID() << "cnt=" << getScreenCount();
    for(int i=0; i<stack.size(); i++)
    {
        if(currentScreenID==i)
        {
            qDebug() << SB_DEBUG_INFO << "***CURRENT***" << i << stack.at(i).performerName << stack.at(i).albumTitle << stack.at(i).songTitle;
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << "             " << i << stack.at(i).performerName << stack.at(i).albumTitle << stack.at(i).songTitle;
        }
    }
}
