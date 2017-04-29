#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H
#include <QString>
#include <QUdpSocket>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QTime>
#include <QTimer>
#include <net/netraw.h>
#include "base.h"
#include "knowledge.h"
#include "libserial/qextserialport.h"

class CMySerialPort;

class CBaseCommunicator : public QObject
{
    Q_OBJECT
protected:
        CMySerialPort* p;
        Net::UDP udpsocket;
        QString _port;
        QString err;
        bool error;
        bool use_udp,use_serial;
        bool udp_open,serial_open;
        Net::Address _addr;
        int _udpport;
        QextSerialPort *rec_serialPort;
public:
        CBaseCommunicator();
        ~CBaseCommunicator();

        void setSerialParams(unsigned int baud, unsigned int charsize, unsigned int parity, short int stopbits);
        void connectSerial(const char* port);
        void connectUdp(const char* addr,int port);
        void activateUdp();
        void deactivateUdp();
        void activateSerial();
        void deactivateSerial();
        void closeSerial();
        void closeUdp();
        bool isUdpConnected();
        bool isSerialConnected();
        bool errorOccured();
        QString getError();
        QString getSerialPort();
public slots:
        void readData();
private:
        char a;
        QTimer *recTime;
        QByteArray recDataFlow;
        unsigned char rPack[13];
        unsigned char robotPacket[16][13];
};

class CCommunicator : public CBaseCommunicator
{
//    QQueue< QByteArray > dataQueue;
    Q_OBJECT
public:
        CCommunicator();
        ~CCommunicator();

    void sendString(const char* s,int len);
    void sendByte(char c);
    void enqueueString(char* s,int len);
    void dequeueAll();
};

#endif // COMMUNICATOR_H
