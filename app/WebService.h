#ifndef MYHTTPSERVER
#define MYHTTPSERVER

#include <QHttpServer>
#include <QHttpServerResponse>
class WebService : public QObject
{
    Q_OBJECT
public:
    explicit WebService();
    ~WebService();
private:
    QHttpServer 		_httpServer;

};

#endif
