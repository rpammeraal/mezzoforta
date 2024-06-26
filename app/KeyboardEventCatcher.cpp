#include <QtGlobal>

#include "KeyboardEventCatcher.h"
#include "OSXNSEventFunctions.h"


#include "Context.h"
#include "Common.h"

KeyboardEventCatcher::KeyboardEventCatcher()
{
}

bool
KeyboardEventCatcher::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    bool returnCode=0;
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);

#ifdef Q_OS_MACOS
    if(eventType=="mac_generic_NSEvent")
    {
        int key;
        if(OSXretrieveKeyPressed(message,key))
        {
            switch(key)
            {
            case SB_OSX_KEY_FORWARD:
                emit songNext();
                returnCode=1;
                break;

            case SB_OSX_KEY_PLAYPAUSE:
                emit songPlayPauseToggle();
                returnCode=1;
                break;

            case SB_OSX_KEY_PREVIOUS:
                emit songPrevious();
                returnCode=1;
                break;

            default:
                break;
            }
        }
    }
#endif //	Q_OS_MACOS
    return returnCode;
}

///	Protected methods
void
KeyboardEventCatcher::doInit()
{
    _init();
}

///	Private methods
void
KeyboardEventCatcher::_init()
{
    //	Connect signals to PlayManager
    connect(this, SIGNAL(songNext()),
            Context::instance()->playManager(), SLOT(playerNext()));
    connect(this, SIGNAL(songPlayPauseToggle()),
            Context::instance()->playManager(), SLOT(playerPlay()));
    connect(this, SIGNAL(songPrevious()),
            Context::instance()->playManager(), SLOT(playerPrevious()));
}
