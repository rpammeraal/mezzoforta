#include <QJsonObject>
#include <QApplication>
#include "WebService.h"

using namespace Qt::StringLiterals;

static inline QString
host(const QHttpServerRequest &request)
{
    return QString::fromLatin1(request.value("Host"));
}

WebService::WebService()
{
    _httpServer.route("/", []() {
        return "Hello world";
    });

    _httpServer.route("/query", [] (const QHttpServerRequest &request) {
        return host(request) + u"/query/"_s;
    });

    _httpServer.route("/query/", [] (qint32 id, const QHttpServerRequest &request) {
        return u"%1/query/%2"_s.arg(host(request)).arg(id);
    });

    _httpServer.route("/query/<arg>/log", [] (qint32 id, const QHttpServerRequest &request) {
        return u"%1/query/%2/log"_s.arg(host(request)).arg(id);
    });

    _httpServer.route("/query/<arg>/log/", [] (qint32 id, float threshold,
                                              const QHttpServerRequest &request) {
        return u"%1/query/%2/log/%3"_s.arg(host(request)).arg(id).arg(threshold);
    });

    _httpServer.route("/user/", [] (const qint32 id) {
        return u"User "_s + QString::number(id);
    });

    _httpServer.route("/user/<arg>/detail", [] (const qint32 id) {
        return u"User %1 detail"_s.arg(id);
    });

    _httpServer.route("/user/<arg>/detail/", [] (const qint32 id, const qint32 year) {
        return u"User %1 detail year - %2"_s.arg(id).arg(year);
    });

    _httpServer.route("/json/", [] {
        return QJsonObject{
            {
                {"key1", "1"},
                {"key2", "2"},
                {"key3", "3"}
            }
        };
    });

    _httpServer.route("/assets/<arg>", [] (const QUrl &url) {
        return QHttpServerResponse::fromFile(u":/assets/"_s + url.path());
    });

    _httpServer.route("/remote_address", [](const QHttpServerRequest &request) {
        return request.remoteAddress().toString();
    });

    // Basic authentication example (RFC 7617)
    _httpServer.route("/auth", [](const QHttpServerRequest &request) {
        auto auth = request.value("authorization").simplified();

        if (auth.size() > 6 && auth.first(6).toLower() == "basic ") {
            auto token = auth.sliced(6);
            auto userPass = QByteArray::fromBase64(token);

            if (auto colon = userPass.indexOf(':'); colon > 0) {
                auto userId = userPass.first(colon);
                auto password = userPass.sliced(colon + 1);

                if (userId == "Aladdin" && password == "open sesame")
                    return QHttpServerResponse("text/plain", "Success\n");
            }
        }
        QHttpServerResponse response("text/plain", "Authentication required\n",
                                     QHttpServerResponse::StatusCode::Unauthorized);
        response.setHeader("WWW-Authenticate", R"(Basic realm="Simple example", charset="UTF-8")");
        return response;
    });

    _httpServer.afterRequest([](QHttpServerResponse &&resp) {
        resp.setHeader("Server", "Qt HTTP Server");
        return std::move(resp);
    });

    const auto port = _httpServer.listen(QHostAddress::Any,80);
    if (!port) {
        qWarning() << QApplication::translate("QHttpServerExample",
                                                  "Server failed to listen on a port.");
        return;	//	CWIP: something else but -1;
    }

    qInfo().noquote()
        << QApplication::translate("QHttpServerExample",
                                       "Running on http://127.0.0.1:%1/"
                                       "(Press CTRL+C to quit)").arg(port);
}

WebService::~WebService()
{
}
