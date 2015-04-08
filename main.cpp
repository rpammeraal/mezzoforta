#include <QApplication>
#include <QMessageBox>

#include "MainWindow.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    app.setOrganizationName("Moose!");
    app.setOrganizationDomain("bargie.net");
    app.setApplicationName("Songbase [Crown Princess Demo]");
    MainWindow mainWin;

    if(mainWin.getErrorState()==0)
    {
        mainWin.show();

        return app.exec();
    }
    else
    {
        QMessageBox::critical(NULL,mainWin.getErrorDescription(),mainWin.getErrorDescription());
    }
}
