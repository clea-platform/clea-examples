
#include <QtCore/QObject>
#include <QtWebSockets/QWebSocketServer>
#include <wfrest/HttpServer.h>




class StreamingServer : public QObject {
    Q_OBJECT

public:
    StreamingServer (uint http_port, uint ws_port, QObject *parent=nullptr);
    ~StreamingServer ();


private :
    wfrest::HttpServer http_server;
    QWebSocketServer *ws_server;
};