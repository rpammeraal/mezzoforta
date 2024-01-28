#include <QHash>
#ifndef MYHTTPSERVER
#define MYHTTPSERVER

#include <QHttpServer>
#include <QHttpServerResponse>

static QHash<QString,QString>		_availableImages;

class WebService : public QObject
{
    Q_OBJECT
public:
    explicit WebService();
    ~WebService();
private:
    QHttpServer 					_httpServer;

    void							_init();
    bool							_isImage(const QString& path) const;

    static QHttpServerResponse		_handleImage(QString path, const QHttpServerRequest& r);
};

#endif
