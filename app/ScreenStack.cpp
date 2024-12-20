#include <QDebug>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "ScreenStack.h"

#include <QMessageBox>

#include "Context.h"

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
    return id;
}

void
ScreenStack::pushScreen(const ScreenItem& id)
{
    bool doPush=0;

    if(_stack.count()==0)
    {
        doPush=1;
    }
    else
    {
        //	Remove all items from stack after _currentScreenID;
        while(_stack.length()-1>_currentScreenID)
        {
            _stack.removeLast();
        }

        //	Check for possible duplicates
        if(id==topScreen())
        {
            qDebug() << SB_DEBUG_WARNING << "Screen already added";
            return;
        }
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
    ScreenItem id;

    if(_stack.isEmpty()==0)
    {
        if(_currentScreenID+1<_stack.length())
        {
            _currentScreenID++;
        }
        id=_stack.at(_currentScreenID);
    }
    return id;
}

ScreenItem
ScreenStack::previousScreen()
{
    ScreenItem id;

    if(_currentScreenID>0)
    {
        _currentScreenID--;
        id=_stack.at(_currentScreenID);
    }
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

void
ScreenStack::replace(const ScreenItem &from, const ScreenItem &to)
{
    for(int i=0;i<_stack.size();i++)
    {
        if(from==_stack.at(i))
        {
            _stack.replace(i,to);
        }
    }
    _dedupe();
}

//	Remove the current screen from stack.
void
ScreenStack::removeCurrentScreen()
{
    if(_stack.count()>1)
    {
        _stack.removeLast();
        _currentScreenID--;
    }
}

void
ScreenStack::removeForward()
{
    while(_stack.count()-1>_currentScreenID)
    {
        _stack.removeLast();
    }
}

void
ScreenStack::removeScreen(const ScreenItem &id, bool editOnlyFlag)
{
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
}

ScreenItem
ScreenStack::topScreen() const
{
    if(!_stack.isEmpty())
    {
        return _stack.last();
    }
    return ScreenItem();
}

//	Only update if ID's are equal.
void
ScreenStack::updateCurrentScreen(const ScreenItem &id)
{
    if(_currentScreenID>=0 && _currentScreenID<_stack.count())
    {
        if(id==currentScreen())
        {
            _stack[_currentScreenID]=id;
        }
        else
        {
            this->debugShow("error:");
            qDebug() << SB_DEBUG_ERROR << "Parameter ID <> current screenstack ID!";
            qDebug() << SB_DEBUG_ERROR << id ;
            qDebug() << SB_DEBUG_ERROR << currentScreen();
            QMessageBox msgBox;
            msgBox.setText("ScreenStack::updateCurrentScreen: Parameter ID <> current screenstack ID!");
            msgBox.exec();
        }
    }
    else
    {
        qDebug() << SB_DEBUG_ERROR << "NO SCREENS ON STACK";
    }
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
        qDebug() << SB_DEBUG_WARNING << "<empty>";
    }
}

///	PUBLIC SLOTS
void
ScreenStack::databaseSchemaChanged()
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
        connect(Context::instance()->controller(),SIGNAL(databaseSchemaChanged()),
                this, SLOT(databaseSchemaChanged()));
        _initDoneFlag=1;
    }
}

void
ScreenStack::_dedupe()
{
    if(_stack.length()>2)
    {
        for(int i=0;i<_stack.length()-1;i++)
        {
            ScreenItem current=_stack.at(i);
            ScreenItem next=_stack.at(i+1);
            while(current==next)
            {
                _stack.removeAt(i+1);
                if(_currentScreenID>i)
                {
                    _currentScreenID--;
                }
                if(i<_stack.length()-1)
                {
                    next=_stack.at(i+1);
                }
                else
                {
                    next=ScreenItem();
                }
            }
        }
    }
}
