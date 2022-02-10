
#include <streamingServer.hpp>
#include <base64.hpp>

#include <opencv2/imgcodecs.hpp>

#include <QJsonDocument>
#include <QJsonObject>




StreamingServer::StreamingServer (Scene &scene_settings, uint http_port, uint ws_port,
                                    QString http_root_dir, QObject *parent) :
                                                                                QObject(parent),
                                                                                m_scene_settings (scene_settings),
                                                                                m_ws_port (ws_port),
                                                                                m_ws_server (nullptr),
                                                                                m_http_port (http_port),
                                                                                m_http_root_dir (http_root_dir) {
    // Registering http server routes
    m_http_server.Static ("/", m_http_root_dir.toStdString().c_str());

    // Building websocket object
    m_ws_server = std::unique_ptr<QWebSocketServer> (new QWebSocketServer (QStringLiteral("People Counter Streamer"),
                                                        QWebSocketServer::NonSecureMode, this));
    connect (m_ws_server.get(), &QWebSocketServer::newConnection, this, &StreamingServer::on_new_connection);
}




StreamingServer::~StreamingServer () {
    // TODO
}




void StreamingServer::start () {
    // TODO Handle errors!

    if (m_http_server.start (m_http_port) == 0) {
        qDebug() << "HTTP sever il listening!";
    }

    if (m_ws_server->listen (QHostAddress::Any, m_ws_port)) {
        qDebug() << "Websocket is listening!";
        connect(m_ws_server.get(), &QWebSocketServer::newConnection, this, &StreamingServer::on_new_connection);
    }
    
}




void StreamingServer::stop () {
    // TODO Closing websocket server
    // TODO Closing websocket clients
    // TODO Closing http server
}




void StreamingServer::dispatch_frame (cv::Mat &frame, Detections &current_detections) {
    /* JSON payload:
        {
            message-type:str,      (scene-settigns || detections)
            img:base64,
            data:[
                {
                    id:#,
                    confidence:#,
                    pos_zone:{
                        id:#
                        name:str
                    }
                }
            ]
        }
     */

    // Converting frame to base64
    std::vector<uchar> buffer;
    buffer.resize(static_cast<size_t>(frame.rows) * static_cast<size_t>(frame.cols));
    cv::imencode(".jpg", frame, buffer);
    QString base64_frame    = QString::fromUtf8 (base64_encode (buffer.data(), buffer.size()).c_str());
    
    // Preparing message to sent
    QVariantHash variant_msg;
    QVariantList variant_list;
    variant_msg["img"]          = base64_frame;
    variant_msg["message-type"] = "detections";

    for (auto &person : current_detections.detections) {
        QVariantHash item;
        QVariantHash pos_zone;
        pos_zone["id"]      = person.zone_id;
        pos_zone["name"]    = person.zone_name;
        item["pos_zone"]    = pos_zone;
        item ["id"]         = person.person_id;
        item ["confidence"] = person.confidence;
        variant_list.push_back (item);
    }

    variant_msg["data"] = variant_list;


    // Sending message to each client
    QJsonObject json_msg    = QJsonObject::fromVariantHash (variant_msg);
    QString str_msg         = QJsonDocument(json_msg).toJson(QJsonDocument::Compact);

    {
        std::unique_lock<std::mutex> clients_lock (m_ws_clients_mutex);
        QMutableListIterator<QWebSocket*> it (m_ws_clients);
        
        while (it.hasNext()) {
            QWebSocket *client  = it.next();
            if (client && client->isValid()) {
                client->sendTextMessage(str_msg);
            }
            else {
                qDebug() << "Removing invalid element..";
                it.remove();
            }
        }
    }
}




void StreamingServer::on_new_connection () {
    QWebSocket *client_socket   = m_ws_server->nextPendingConnection();
    if (client_socket) {
        std::unique_lock<std::mutex> clients_lock (m_ws_clients_mutex);
        m_ws_clients << client_socket;

        // Sending scene list to new client
        QVariantHash variant_msg;
        QVariantList variant_settings;
        variant_msg["message-type"] = "scene-settings";
        for (auto &zone : m_scene_settings) {
            QVariantHash variant_item;
            variant_item["zone_name"]   = zone.get_zone_name();
            variant_item["zone_id"]     = zone.get_zone_id();
            variant_settings.push_back (variant_item);
        }
        variant_msg["scene-settings"]   = variant_settings;
        QString str_msg                 = QJsonDocument(QJsonObject::fromVariantHash (variant_msg)).
                                            toJson(QJsonDocument::Compact);
        client_socket->sendTextMessage(str_msg);
    }
}




void StreamingServer::on_client_disconnection () {
    QWebSocket *client_socket   = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Client disconnected!";
    
    if (client_socket) {
        std::unique_lock<std::mutex> clients_lock (m_ws_clients_mutex);
        m_ws_clients.removeAll (client_socket);
        client_socket->deleteLater();
    }
}