#ifndef MYHTTPSERVER
#define MYHTTPSERVER
#include <QCoreApplication>
#include <QNetworkInterface>
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
    void request(int timeout=1000) const;

private:
    QTcpServer*				_server;
    QDateTime       		_startDateTime;
    QHash<QString,QString>	_availableImages;

    void					_home(QTcpSocket* s) const;

    void					_init();
    void					_fourOhFour(QTcpSocket* s) const;

    bool					_isImage(const QString& path) const;
    const QString			_processHTML(const QString& rPath, const QHash<QString,QString>& hash) const;
    const QString			_retrievePath(const QString& header) const;
    void					_serveImage(QTcpSocket* s,const QString &path) const;
    void					_writeBody(QTcpSocket* s,const QString& contents) const;
signals:
};

#endif
