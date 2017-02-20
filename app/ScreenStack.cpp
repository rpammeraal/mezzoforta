#include <QDebug>

#include "Common.h"
#include "Context.h"
#include "ScreenStack.h"

#include <QMessageBox>

ScreenStack::ScreenStack()
{
    _initDoneFlag=0;
}

ScreenStack::~ScreenStack()
{
}

void
ScreenStack::clear()
{
    _currentScreenID=-1;
    _stack.clear();
}

int
ScreenStack::count() const
{
    return _stack.count();
}

ScreenItem
ScreenStack::popScreen()
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    ScreenItem id;

    if(_stack.isEmpty()==0)
    {
        id=currentScreen();
        _stack.removeLast();

        if(_currentScreenID>getScreenCount()-1)
        {
            _currentScreenID=getScreenCount()-1;
        }
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
    return id;
}

void
ScreenStack::pushScreen(const ScreenItem& id)
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << id << _currentScreenID;
    bool doPush=0;

    if(_stack.count()==0)
    {
        doPush=1;
    }
    else
    {
        ScreenItem current=currentScreen();
        if(current!=id)
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
        _stack.append(id);
        _currentScreenID++;
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
}

ScreenItem
ScreenStack::currentScreen() const
{
    ScreenItem id;

    if(_stack.isEmpty()==0)
    {
        id=_stack.at(_currentScreenID);
    }
    return id;
}

ScreenItem
ScreenStack::nextScreen()
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << _currentScreenID;
    ScreenItem id;

    if(_stack.isEmpty()==0)
    {
        if(_currentScreenID+1<_stack.length())
        {
            _currentScreenID++;
        }
        id=_stack.at(_currentScreenID);
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
    return id;
}

ScreenItem
ScreenStack::previousScreen()
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << _currentScreenID;
    ScreenItem id;

    if(_currentScreenID>0)
    {
        _currentScreenID--;
        id=_stack.at(_currentScreenID);
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
    return id;
}

int
ScreenStack::getCurrentScreenID() const
{
    return _currentScreenID;
}

int
ScreenStack::getScreenCount() const
{
    return _stack.length();
}

//	Remove the current screen from stack.
void
ScreenStack::removeCurrentScreen()
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    if(_stack.count()>1)
    {
        _stack.removeLast();
        _currentScreenID--;
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
}

void
ScreenStack::removeForward()
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << _currentScreenID;
    while(_stack.count()-1>_currentScreenID)
    {
        _stack.removeLast();
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
}

void
ScreenStack::removeScreen(const ScreenItem &id, bool editOnlyFlag)
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";
    qDebug() << SB_DEBUG_INFO << id << editOnlyFlag << _currentScreenID << _stack.count();
    for(int i=_stack.count()-1;i>=0;i--)
    {
        const ScreenItem& currentID=_stack.at(i);
        if(currentID.compare(id,1))
        {
            if(
                (editOnlyFlag==0) ||
                (
                    editOnlyFlag==1 &&
                    currentID.editFlag()==id.editFlag() &&
                    currentID.editFlag()==1
                )
            )
            {
                _stack.removeAt(i);
                if(i<=_currentScreenID)
                {
                    _currentScreenID--;
                }
            }
        }
    }
    qDebug() << SB_DEBUG_INFO << "sssssssssssssssssssssssssssssssssssssssssssssssssssssss";
}

//	Only update if ID's are equal.
void
ScreenStack::updateCurrentScreen(const ScreenItem &id)
{
    qDebug() << SB_DEBUG_INFO << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS";

    qDebug() << SB_DEBUG_INFO << "id=" << id;
    qDebug() << SB_DEBUG_INFO << "curr=" << currentScreen();

    if(_currentScreenID>=0 && _currentScreenID<_stack.count())
    {
        if(id==currentScreen())
        {
            _stack[_currentScreenID]=id;
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
ScreenStack::updateSBIDInStack(const ScreenItem &si)
{
    for(int i=0;i<_stack.size();i++)
    {
        if(si==_stack.at(i))
        {
            _stack.replace(i,si);
        }
    }
}

void
ScreenStack::debugShow(const QString& c)
{
    qDebug() << SB_DEBUG_INFO << c << "num screens" << _stack.size();
    if(_stack.size()>0)
    {
        for(int i=0; i<_stack.size(); i++)
        {
            QString isCurrent="";
            if(_currentScreenID==i)
            {
                isCurrent="***Current***";
            }
            else
            {
                isCurrent="             ";
            }
            qDebug() << SB_DEBUG_INFO
                     << isCurrent << i << _stack.at(i)
                     << "subtabID=" << _stack.at(i).subtabID()
                     << "isEditFlag:" << _stack.at(i).editFlag()
                     << "sortColumn:" << _stack.at(i).sortColumn()
            ;
        }
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "<empty>";
    }
    qDebug() << SB_DEBUG_INFO << "_currentScreenID=" << _currentScreenID;
}

///	PUBLIC SLOTS
void
ScreenStack::schemaChanged()
{
    this->clear();
}

///	PROTECTED
void
ScreenStack::doInit()
{
    _init();
}

///	PRIVATE
void
ScreenStack::_init()
{
    clear();

    if(_initDoneFlag==0)
    {
        connect(Context::instance()->getDataAccessLayer(),SIGNAL(schemaChanged()),
                this, SLOT(schemaChanged()));
        _initDoneFlag=1;
    }
}
