#ifndef AUTOMATEDREFEREE_H
#define AUTOMATEDREFEREE_H

#include <QObject>
#include <QString>

#include "net/netraw.h"
#include "logger.h"
#include "gamestate.h"
#include "base.h"

class CAutomatedReferee : public QObject
{
    Q_OBJECT
private:
    Net::UDP mc; // multicast server
    int _port;
    QString _net_address;
    QString _net_interface;

    char* generatePacket();
    GameStatePacket info;

public:
   CAutomatedReferee(QObject *parent = 0);
   ~CAutomatedReferee();
   bool open();
   void close();
   bool send(char command);
   void reConfigure(int port = 10011, QString net_address = "224.5.23.1", QString net_interface = "");

signals:

public slots:



};

#endif // AUTOMATEDREFEREE_H
