#include <QDebug>

#include "Common.h"
#include "ScreenStack.h"

#include <QMessageBox>

ScreenStack::ScreenStack()
{
    clear();
}

ScreenStack::~ScreenStack()
{
}

void
ScreenStack::clear()
{
    currentScreenID=-1;
    stack.clear();
}

int
ScreenStack::count() const
{
    return stack.count();
}
SBID
ScreenStack::popScreen()
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
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
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
    return id;
}

void
ScreenStack::pushScreen(const SBID& id)
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << id << currentScreenID;
    bool doPush=0;

    if(stack.count()==0)
    {
        doPush=1;
    }
    else
    {
        SBID current=currentScreen();
        if(!(current.compareSimple(id)))	//	this needs to be a compareSimple
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
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
}

SBID
ScreenStack::currentScreen() const
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
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
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
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
    return id;
}

SBID
ScreenStack::previousScreen()
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    SBID id;

    if(currentScreenID>0)
    {
        currentScreenID--;
        id=stack.at(currentScreenID);
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
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

//	Remove the current screen from stack.
void
ScreenStack::removeCurrentScreen()
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    if(stack.count()>1)
    {
        stack.removeLast();
        currentScreenID--;
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
}

void
ScreenStack::removeForward()
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << currentScreenID;
    while(stack.count()-1>currentScreenID)
    {
        stack.removeLast();
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
}

void
ScreenStack::removeScreen(const SBID &id, bool editOnlyFlag)
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << id << editOnlyFlag << currentScreenID << stack.count();
    for(int i=stack.count()-1;i>=0;i--)
    {
        const SBID& currentID=stack.at(i);
        if(currentID.compareSimple(id))
        {
            if(
                (editOnlyFlag==0) ||
                (
                    editOnlyFlag==1 &&
                    currentID.isEditFlag==id.isEditFlag &&
                    currentID.isEditFlag==1
                )
            )
            {
                stack.removeAt(i);
                if(i<=currentScreenID)
                {
                    currentScreenID--;
                }
            }
        }
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
}

//	Only update if ID's are equal.
void
ScreenStack::updateCurrentScreen(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << id;
    qDebug() << SB_DEBUG_INFO << currentScreenID << stack.count();

    if(currentScreenID>=0 && currentScreenID<stack.count())
    {
        if(id.compareSimple(currentScreen()))
        {
            stack[currentScreenID]=id;
        }
        else
        {
            qDebug() << SB_DEBUG_ERROR << "Parameter ID <> current screenstack ID!";
            QMessageBox msgBox;
            msgBox.setText("ScreenStack::updateCurrentScreen: Parameter ID <> current screenstack ID!");
            msgBox.exec();
        }
    }
    else
    {
        qDebug() << SB_DEBUG_ERROR << "NO SCREENS ON STACK";
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
}

///
/// \brief ScreenStack::updateSBIDInStack
/// \param id
///
/// Update all instances of id in entire stack.
/// Mostly used after saving an item.
///
void
ScreenStack::updateSBIDInStack(const SBID &id)
{
    for(int i=0;i<stack.size();i++)
    {
        if(id.compareSimple(stack.at(i))==1)
        {
            stack.replace(i,id);
        }
    }
}

void
ScreenStack::debugShow(const QString& c)
{
    qDebug() << SB_DEBUG_INFO << c << "num screens" << stack.size();
    if(stack.size()>0)
    {
        for(int i=0; i<stack.size(); i++)
        {
            QString isCurrent="";
            if(currentScreenID==i)
            {
                isCurrent="***CURRENT***";
            }
            else
            {
                isCurrent="             ";
            }
            qDebug() << SB_DEBUG_INFO
                     << isCurrent << i << stack.at(i)
                     << "subtabID=" << stack.at(i).subtabID
                     << "isEditFlag:" << stack.at(i).isEditFlag
                     << "sortColumn:" << stack.at(i).sortColumn
            ;
        }
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "NO SCREENS ON STACK";
    }
    qDebug() << SB_DEBUG_INFO << currentScreenID;
}

///	PROTECTED
void
ScreenStack::doInit()
{
    clear();
}
