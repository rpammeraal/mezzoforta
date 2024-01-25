#ifndef MYHTTPSERVER
#define MYHTTPSERVER
#include <QCoreApplication>
#include <QNetworkInterface>
#include <iostream>
#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>

class myHTTPserver : public QObject
{
    Q_OBJECT
public:
    explicit myHTTPserver(QObject *parent = 0);
    ~myHTTPserver();
    QTcpSocket *socket ;
public slots:
    void myConnection();
private:
    qint64 bytesAvailable() const;
    QTcpServer *server;
signals:
};

#endif
