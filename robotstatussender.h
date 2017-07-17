#ifndef CROBOTSTATUSSENDER_H
#define CROBOTSTATUSSENDER_H


#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QUdpSocket>
#include <proto/robot_status.pb.h>
#include <varswidget.h>
#include <knowledge.h>


class CRobotStatusSender : public QObject
{

    Q_OBJECT

public:
    explicit CRobotStatusSender(int ms, QObject* parent);
    virtual ~CRobotStatusSender();

    QMutex mutex;
    robot_status::Status *packet;
    bool dataReady;
    QTimer *timer;

signals:

public slots:
    void sendData();

private:
    QUdpSocket *socket;

    void fillPacket(robot_status::Status*);


};

#endif // CROBOTSTATUSSENDER_H
