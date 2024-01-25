#ifndef MYHTTPSERVER
#define MYHTTPSERVER
#include <QCoreApplication>
#include <QNetworkInterface>
#include <iostream>
#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QDateTime>

class WebService : public QObject
{
    Q_OBJECT
public:
    explicit WebService(QObject *parent = 0);
    ~WebService();

public slots:
    void request(int timeout=100);

private:
    QTcpServer*		_server;
    QDateTime       _startDateTime;

    void			_home(QTcpSocket* s) const;
    void			_fourOhFour(QTcpSocket* s) const;

    const QString	_retrievePath(const char* header) const;
    void			_writeBody(QTcpSocket* s,const QString& contents) const;
signals:
};

#endif
