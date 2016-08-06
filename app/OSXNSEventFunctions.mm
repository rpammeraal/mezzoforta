#import <iostream>
#import <AppKit/AppKit.h>
#import <IOKit/IOKitKeys.h>

#include "OSXNSEventFunctions.h"

bool retrieveKeyPressed(void* event,int& key)
{
    bool keyPressed=0;
    NSEvent* nsevent=reinterpret_cast<NSEvent *>(event);
    if(([nsevent type]==NSSystemDefined) && ([nsevent subtype]==NSScreenChangedEventType))
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
