#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QDateTime>

#include "Common.h"
#include "Controller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("Moose!");
    app.setOrganizationDomain("bargie.net");
    app.setApplicationName("Songbase");

//    QFile file(":/qss/default");
//    file.open(QFile::ReadOnly);
//    QString styleSheet = QLatin1String(file.readAll());
//    app.setStyleSheet(styleSheet);

    qsrand(QDateTime::currentMSecsSinceEpoch());
    Controller c(argc, argv);
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
