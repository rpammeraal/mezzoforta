#include <QDirIterator>
#include <QFile>
#include <QBuffer>
#include "WebService.h"
#include "Common.h"

WebService::WebService(QObject *parent) : QObject(parent)
{
    _server = new QTcpServer(this);
    // waiting for the web brower to make contact,this will emit signal
    connect(_server, SIGNAL(newConnection()),this, SLOT(request()));
    int port=80;
    if(!_server->listen(QHostAddress::Any,port))
    {
        qDebug() << SB_DEBUG_INFO << "Web server could not start";
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "Web server is waiting for a connection on port" << port;
        _startDateTime=QDateTime::currentDateTime();
    }
}

WebService::~WebService()
{
}

void
WebService::request(int timeout)
{
    QTcpSocket* socket = _server->nextPendingConnection();
    while(!(socket->waitForReadyRead(timeout)));  //waiting for data to be read from web browser

    static char rawHeader[10000];
    int sv=socket->read(rawHeader,10000);
    QString header=QString(rawHeader).left(sv);
    QString path=_retrievePath(header);
    qDebug() << SB_DEBUG_INFO << "path=" << path;

    if(path==QString("/") || path==QString(""))
    {
        this->_home(socket);
    }
    else if(path==QString("/favicon.ico"))
    {
        this->_favIcon(socket);
    }
    else
    {
        this->_fourOhFour(socket);
    }
    socket->close();
}

void
WebService::_home(QTcpSocket* s) const
{
    QDateTime currentDateTime=QDateTime::currentDateTime();
    const qint64 msDiff = _startDateTime.msecsTo(currentDateTime);

    int tsDiff=msDiff/1000;
    int mDiff=tsDiff/60;
    int sDiff=tsDiff - (mDiff * 60);

    QHash<QString,QString> hash;
    hash["___SB_DURATION___"]=QString("%1m:%2s").arg(mDiff).arg(sDiff);

    QString body = _processHTML(":www/index.html",hash);
    this->_writeBody(s,body);
}

void
WebService::_favIcon(QTcpSocket* s) const
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QImage qp(":/images/squarelogo");
    qp.save(&buffer, "PNG");
    QString encoded = buffer.data().toBase64();
    QString body=QString("<img src=\"data:image/png;base64,\n%1\">").arg(encoded);
    this->_writeBody(s,body);
}

void
WebService::_fourOhFour(QTcpSocket* s) const
{
    QString body=QString("<I><CENTER>404: This URL was not found...<CENTER></I><br>");

    this->_writeBody(s,body);
}

const QString
WebService::_processHTML(const QString& path, const QHash<QString,QString>& hash) const
{
    QString body=QString("");
    QFile f=QFile(path);
    if(!f.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << SB_DEBUG_ERROR << "Could not open file for read: " << path;
        return body;
    }
    QTextStream in(&f);
    body = in.readAll();
    for (auto i = hash.cbegin(), end = hash.cend(); i != end; ++i)
    {
        body=body.replace(i.key(),i.value());
    }
    return body;
}

const QString
WebService::_retrievePath(const QString& header) const
{
    QStringList headers=header.split(QString("\r\n"));
    QString hostArg;
    QString pathArg;
    for (const auto & i : headers)
    {

        if(i.left(3)==QString("GET"))
        {
            QString j=i;
            pathArg=j.mid(4,j.length()-13);
        }
        else
        {
            QStringList parameter=i.split(QString(": "));
            if(parameter.size()==2)
            {
                if(parameter.at(0)==QString("Host"))
                {
                    hostArg=parameter.at(1);
                }
            }
        }
    }
    return pathArg.replace(QString("http://") + hostArg,QString());
}

void
WebService::_writeBody(QTcpSocket* socket,const QString& body) const
{
    socket->write("HTTP/1.1 200 OK\r\n");       // \r needs to be before \n
    socket->write("Content-Type: text/html\r\n");
    socket->write("Connection: close\r\n");
    socket->write("\r\n\r\n");

    socket->write("<!DOCTYPE html>\r\n");
    socket->write("<html><body>");
    socket->write(body.toLatin1());
    socket->write(" </body>\n</html>\n");

    socket->flush();
    connect(socket, SIGNAL(disconnected()),socket, SLOT(deleteLater()));
    socket->disconnectFromHost();

}
