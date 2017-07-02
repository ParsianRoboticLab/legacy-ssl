#ifndef MIXTEAMSENDER_H
#define MIXTEAMSENDER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QUdpSocket>
#include <proto/multi_team_communication.pb.h>

class MixTeamSender : public QObject
{
    Q_OBJECT
public:
    explicit MixTeamSender(QObject *parent = 0);
    virtual ~MixTeamSender();

    QMutex mutex;
    multi_team_comm::TeamPlan *packet;
    bool flag;
    QTimer *timer;

signals:

public slots:
    void sendData();

private:
    QUdpSocket *socket;

};

#endif // MIXTEAMSENDER_H
