#ifndef CREFREETHREAD_H
#define CREFREETHREAD_H

#include <QThread>
#include <QUDPSocket>

#include "worldmodel.h"
#include "base.h"


class CRefreeThread : public QThread
{
    Q_OBJECT

public:
    CRefreeThread();

protected:
    void run();

private:
    CWorldModel* wm;
    QUDPSocket* refSock;
    int mode;
    int counter;
};


struct GameStatePacket{
    char cmd;                      // current referee command
    unsigned char cmd_counter;     // increments each time new command is set
    unsigned char goals_blue;      // current score for blue team
    unsigned char goals_yellow;    // current score for yellow team
    unsigned short time_remaining; // seconds remaining for current game stage (network byte order)
};

#endif // CREFREETHREAD_H
