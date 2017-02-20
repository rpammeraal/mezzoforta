#ifndef OSXNSEVENTFUNCTIONS_H
#define OSXNSEVENTFUNCTIONS_H

#include <QtGlobal>

#ifdef Q_OS_OSX

//	Returns 1 if a key is pressed
bool OSXretrieveKeyPressed(void* event,int& key);

#endif //	Q_OS_OSX


#endif // OSXNSEVENTFUNCTIONS_H
