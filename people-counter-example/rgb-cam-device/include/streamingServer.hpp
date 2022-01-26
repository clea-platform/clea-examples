
#pragma once

#include <memory>

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <wfrest/HttpServer.h>

#include <opencv2/core/mat.hpp>

#include <commons.hpp>




class StreamingServer : public QObject {
    Q_OBJECT

public:
    StreamingServer (uint http_port, uint ws_port, QString http_root_dir, QObject *parent=nullptr);
    ~StreamingServer ();

    void start();
    void stop ();
    void dispatch_frame (cv::Mat &frame, Detections &current_detections);


private slots:
    void on_new_connection();
    void on_client_disconnection();


private :
    uint m_ws_port;
    std::unique_ptr<QWebSocketServer> m_ws_server;
    QList<QWebSocket *> m_ws_clients;
    std::mutex m_ws_clients_mutex;

    uint m_http_port;
    QString m_http_root_dir;
    wfrest::HttpServer m_http_server;
};


using StreamingServer_ptr   = std::unique_ptr<StreamingServer>;