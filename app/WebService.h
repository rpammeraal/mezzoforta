#include <QHash>
#ifndef MYHTTPSERVER
#define MYHTTPSERVER

#include <QHttpServer>
#include <QHttpServerResponse>

static QHash<QString,QString>		_availableResource;

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

    static QHttpServerResponse		_controlPlayer(QString unused, const QHttpServerRequest& r);
    static QHttpServerResponse		_fourOhFour();
    static QHttpServerResponse		_getImageResource(QString path, const QHttpServerRequest& r);
    static QHttpServerResponse		_getHTMLResource(QString path, const QHttpServerRequest& r);
    static QHttpServerResponse		_getResource(QString path, const QHttpServerRequest& r, bool isImage=0);
    static QString                  _populateData(const QString& resourcePath, const QString& path, const QHttpServerRequest& r);
};

#endif
