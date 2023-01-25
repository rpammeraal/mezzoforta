#ifndef WEBSERVICE_H
#define WEBSERVICE_H

#include <QObject>

class WebService : public QObject
{
    Q_OBJECT
public:
    WebService();
    ~WebService();

    static QString root();

private:
    void _init();
};

#endif // WEBSERVICE_H
