#ifndef MIXTEAMTHREAD_H
#define MIXTEAMTHREAD_H
#include <QThread>
#include <QUdpSocket>
#include <proto/multi_team_communication.pb.h>
#include <QMutex>
#include <QTimer>

class MixTeamThread : public QThread
{
    Q_OBJECT
public:
    explicit MixTeamThread(QObject *parent = 0);
    virtual ~MixTeamThread();
    void run();

    void HelloUDP();
    QMutex mutex;
    multi_team_comm::TeamPlan *packet;
    bool flag;
    QTimer *timer;

signals:

public slots:
    void readyRead();
    void sendData();

private:
    QUdpSocket *socket;
};

#endif // MIXTEAMTHREAD_H
