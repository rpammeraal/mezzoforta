#include <QApplication>
#include <QMessageBox>

#include "MainWindow.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    app.setOrganizationName("Moose!");
    app.setOrganizationDomain("bargie.net");
    app.setApplicationName("Songbase");
    MainWindow mainWin;

    if(mainWin.getErrorState()==0)
    {
        mainWin.show();

        mainWin.resize(1200,600);
        return app.exec();
    }
    else
    {
        QMessageBox::critical(NULL,mainWin.getErrorDescription(),mainWin.getErrorDescription());
    }
    return -1;
}
