#include "automatedreferee.h"
#include <QDebug>

CAutomatedReferee::CAutomatedReferee(QObject *parent) :
    QObject(parent)
{
    info.cmd = 0;
    info.cmd_counter = 0;
    info.goals_blue = 0;
    info.goals_yellow = 0;
    info.time_remaining = 0;
}

CAutomatedReferee::~CAutomatedReferee()
{

}

void CAutomatedReferee::close()
{
    mc.close();
}

bool CAutomatedReferee::open() {
  close();

  if(!mc.open(_port,true,true)) {
    debug(QString("[Automated Referee] Unable to open UDP network port: %1").arg(_port), D_GAME, QColor("red"));
    return(false);
  }

  Net::Address multiaddr,interface;
  multiaddr.setHost(_net_address.toLocal8Bit().data(),_port);
  if(_net_interface.length() > 0){
    interface.setHost(_net_interface.toLocal8Bit().data(),_port);
  }else{
    interface.setAny();
  }

  if(!mc.addMulticast(multiaddr,interface)) {
    debug(QString("[Automated Referee] Unable to setup UDP multicast."),D_GAME,QColor("orange"));
  }
  debug(QString("[Automated Referee] UDP network successfully configured. (Multicast address: %1:%2)").arg(_net_address).arg(_port),D_MANI,QColor("green"));
  return(true);
}

char* CAutomatedReferee::generatePacket()
{
    char buffer[6];
    buffer[0] = info.cmd;
    buffer[1] = info.cmd_counter;
    buffer[2] = info.goals_blue;
    buffer[3] = info.goals_yellow;
    buffer[4] = (unsigned char) (info.time_remaining) & 0x00FF;
    buffer[5] = (unsigned char) (info.time_remaining >> 8) & 0x00FF;
    return buffer;
}

bool CAutomatedReferee::send(char command)
{
    Net::Address multiaddr;

    if (command != info.cmd)
    {
        info.cmd = command;
        info.cmd_counter++;
        if (info.cmd_counter > 255)
        {
            info.cmd_counter = 0;
        }
    }

    char data[6];
    memcpy(data, generatePacket(), 6);

    multiaddr.setHost(_net_address.toLocal8Bit().data(),_port);
    bool result;
    result=mc.send(data, 6, multiaddr);
    if (result==false) {
      debug(QString("[Automated Referee] Sending UDP datagram failed."),D_GAME,QColor("red"));
    }
    return(result);
}

void CAutomatedReferee::reConfigure(int port, QString net_address, QString net_interface)
{
    this->close();
//    qDebug() << port << net_address;
    _port = port;
    _net_address=net_address;
    _net_interface=net_interface;
    this->open();
}
