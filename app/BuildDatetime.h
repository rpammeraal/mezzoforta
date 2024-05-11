#ifndef BUILDDATETIME_H
#define BUILDDATETIME_H

#include <QString>
const QString COMPILE_TIME =  QStringLiteral(__DATE__ " " __TIME__);

#endif // BUILDDATETIME_H
