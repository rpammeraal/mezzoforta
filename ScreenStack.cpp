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
        qDebug() << SB_DEBUG_INFO << "current=" << current;
        qDebug() << SB_DEBUG_INFO << "id=" << id;
        if(!(current==id))
        {
            doPush=1;
            while(getCurrentScreenID()+1<=getScreenCount()-1)
            {
                qDebug() << SB_DEBUG_INFO << getCurrentScreenID() << getScreenCount();
                popScreen();
            }
        }
    }

    if(doPush)
    {
        stack.append(id);
        currentScreenID++;
    }
    debugShow("after pushScreen");
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
    debugShow("after popScreen");
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
    debugShow("end nextScreen");
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
    for(int i=0; i<stack.size(); i++)
    {
        if(currentScreenID==i)
        {
            //qDebug() << SB_DEBUG_INFO << "***CURRENT***" << i << "item_type=" << stack.at(i).sb_item_type << "item_id=" << stack.at(i).sb_item_id;
            qDebug() << SB_DEBUG_INFO << "***CURRENT***" << i << stack.at(i);
        }
        else
        {
            //qDebug() << SB_DEBUG_INFO << "             " << i << "item_type=" << stack.at(i).sb_item_type << "item_id=" << stack.at(i).sb_item_id;
            qDebug() << SB_DEBUG_INFO << "             " << i << stack.at(i);
        }
    }
}
