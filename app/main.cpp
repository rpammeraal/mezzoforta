#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QDateTime>

#include "Controller.h"
#include "OSXNSEventFunctions.h"
#include "SBIDBase.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#ifdef Q_OS_OSX
    qDebug() << SB_DEBUG_INFO << "Calling OSXSetupSleepCallback";
    OSXSetupSleepCallback();
#endif //	Q_OS_OSX

    //	Set up names
    app.setOrganizationName("Moose!");
    app.setOrganizationDomain("bargie.net");
    app.setApplicationName("Songbase");

    //	Set up types
    //qRegisterMetaType<SBIDBase>();

    //	Set up randomizer
    qsrand(QDateTime::currentMSecsSinceEpoch());
    qDebug() << SB_DEBUG_INFO << app.platformName();

    //	Set up system
    Controller c(argc, argv, &app);
    if(c.initSuccessFull())
    {
        app.exec();
    }
    else
    {
        QMessageBox d;
        d.setText("No database selected. Terminating");
        d.exec();
    }
    return 0;
}
