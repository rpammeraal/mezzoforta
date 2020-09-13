#include <QtGlobal>

#ifdef Q_OS_OSX

#import <iostream>
#import <AppKit/AppKit.h>
#import <IOKit/IOKitKeys.h>


#include "OSXNSEventFunctions.h"
#include "Context.h"

bool OSXretrieveKeyPressed(void* event,int& key)
{
    bool keyPressed=0;
    NSEvent* nsevent=reinterpret_cast<NSEvent *>(event);
    if(([nsevent type]==NSEventTypeSystemDefined) && ([nsevent subtype]==NSEventSubtypeScreenChanged))
    {
        int data1=([nsevent data1]);
        int keyCode=(data1 & 0xFFFF0000) >> 16;
        int keyFlags=data1 & 0x0000FFFF;
        int keyState=(((keyFlags & 0xFF00) >> 8)) == 0xA;

        std::cout << keyCode << keyFlags << keyState << keyFlags;
        if(keyState)
        {
            key=keyCode;
            keyPressed=1;
        }
    }
    return keyPressed;
}

io_connect_t  root_port; // a reference to the Root Power Domain IOService

//	Do not allow system to sleep :)
void
OSXSleepCallback(void* refCon,io_service_t service,natural_t messageType,void * messageArgument )
{
    Q_UNUSED(refCon);
    Q_UNUSED(service);

    PlayManager* pm=Context::instance()->playManager();

    bool songPlayingFlag=(pm?pm->songPlayingFlag():0);

    switch(messageType)
    {

        case kIOMessageCanSystemSleep:
            /* Idle sleep is about to kick in. This message will not be sent for forced sleep.
                Applications have a chance to prevent sleep by calling IOCancelPowerChange.
                Most applications should not prevent idle sleep.

                Power Management waits up to 30 seconds for you to either allow or deny idle
                sleep. If you don't acknowledge this power change by calling either
                IOAllowPowerChange or IOCancelPowerChange, the system will wait 30
                seconds then go to sleep.
            */

            if(songPlayingFlag)
            {
                //	Cancel idle sleep
                qDebug() << SB_DEBUG_INFO << "denying idle sleep";
                IOCancelPowerChange( root_port, (long)messageArgument );
            }
            else
            {
                // Allow idle sleep
                qDebug() << SB_DEBUG_INFO << "allowing idle sleep";
                IOAllowPowerChange( root_port, (long)messageArgument );
            }
            break;

        case kIOMessageSystemWillSleep:
            /* The system WILL go to sleep. If you do not call IOAllowPowerChange or
                IOCancelPowerChange to acknowledge this message, sleep will be
                delayed by 30 seconds.

                NOTE: If you call IOCancelPowerChange to deny sleep it returns
                kIOReturnSuccess, however the system WILL still go to sleep.
            */

            IOAllowPowerChange( root_port, (long)messageArgument );
            break;

        case kIOMessageSystemWillPowerOn:
            //System has started the wake up process...
            break;

        case kIOMessageSystemHasPoweredOn:
            //System has finished waking up...
        break;

        default:
            break;

    }
}

int
OSXSetupSleepCallback()
{
    // notification port allocated by IORegisterForSystemPower
    IONotificationPortRef  notifyPortRef;

    // notifier object, used to deregister later
    io_object_t            notifierObject;
   // this parameter is passed to the callback
    void*                  refCon=NULL;

    // register to receive system sleep notifications

    root_port = IORegisterForSystemPower( refCon, &notifyPortRef, OSXSleepCallback, &notifierObject );
    if ( root_port == 0 )
    {
        printf("IORegisterForSystemPower failed\n");
        return 1;
    }

    // add the notification port to the application runloop
    CFRunLoopAddSource( CFRunLoopGetCurrent(),
            IONotificationPortGetRunLoopSource(notifyPortRef), kCFRunLoopCommonModes );

    /* Start the run loop to receive sleep notifications. Don't call CFRunLoopRun if this code
        is running on the main thread of a Cocoa or Carbon application. Cocoa and Carbon
        manage the main thread's run loop for you as part of their event handling
        mechanisms.
    */
    //CFRunLoopRun();

    //Not reached, CFRunLoopRun doesn't return in this case.
    return (0);
}

#endif //	Q_OS_OSX
