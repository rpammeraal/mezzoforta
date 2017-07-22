#ifndef OSXNSEVENTFUNCTIONS_H
#define OSXNSEVENTFUNCTIONS_H

#include <QtGlobal>

#ifdef Q_OS_OSX

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include <mach/mach_port.h>
#include <mach/mach_interface.h>
#include <mach/mach_init.h>

#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>



//	Returns 1 if a key is pressed
bool OSXretrieveKeyPressed(void* event,int& key);
void OSXSleepCallback(void* refCon,io_service_t service,natural_t messageType,void * messageArgument );
int  OSXSetupSleepCallback();

#endif //	Q_OS_OSX


#endif // OSXNSEVENTFUNCTIONS_H
