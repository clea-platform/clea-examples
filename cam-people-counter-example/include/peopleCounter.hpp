
#pragma once


#include <QtCore/QObject>

#include <hemeraoperation.h>
#include <AstarteDeviceSDK.h>

/*namespace Hemera {
class Operation;
}

class QTimer;

class AstarteDeviceSDK;*/

class PeopleCounter : public QObject
{
    Q_OBJECT

public:
    PeopleCounter(const QByteArray &interface, const QByteArray &path, const QString &function, const QString &device, int interval = 1, double scale = 1, QObject *parent = nullptr);
    ~PeopleCounter();

private slots:
    void checkInitResult(Hemera::Operation *op);
    void sendValues();

    void handleIncomingData(const QByteArray &interface, const QByteArray &path, const QVariant &value);

private:
    AstarteDeviceSDK *m_sdk;
    QByteArray m_interface;
    QByteArray m_path;
    QString m_function;
    QTimer *m_updateTimer;
    double m_scale;
    double m_xValue;
    bool m_ready;

    static int randomInterval();
};
