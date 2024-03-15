// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QMessageBox>
#include <QString>
#include "OSXNSEventFunctions.h"
#include "SBIDBase.h"

#include "Controller.h"
#include "WebService.h"

using namespace Qt::StringLiterals;

#ifdef Q_OS_WIN

QFile _logFile;

#include <iostream>
using namespace std;

void msgHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QTextStream out(&_logFile);
    out << localMsg.constData() << endl;;
    switch (type) {
    case QtDebugMsg:
        out << localMsg.constData();
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}
#endif

int main(int argc, char *argv[])
{
    QCoreApplication::addLibraryPath("./");
#ifdef Q_OS_WIN
    QDir tmpDir("/tmp");
    if(!tmpDir.exists())
    {
        QDir().mkdir("/tmp");
    }
    _logFile.setFileName("/tmp/mezzaforte.out");
    _logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

    qInstallMessageHandler(msgHandler);
#endif
    QApplication app(argc, argv);

#ifdef Q_OS_MACOS
    OSXSetupSleepCallback();
#endif //	Q_OS_MACOS

    //	Set up names
    app.setOrganizationName("roam-corp");
    app.setOrganizationDomain("roam-corp.com");
    app.setApplicationName("MezzoForta!");
    QIcon icon=QIcon(":/resources/images/squarelogo.png");
    app.setWindowIcon(icon);

    //	Set up types
    qRegisterMetaType<SBIDBase>();
    qDebug() << SB_DEBUG_INFO;

    //	Set up system
    Controller c(argc, argv, &app);
    if(c.initSuccessFull())
    {
        WebService server;
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
