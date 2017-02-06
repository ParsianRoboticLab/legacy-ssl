#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H
#include <QString>
#include <QUdpSocket>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QTime>
#include <net/netraw.h>
#include "base.h"
#include "knowledge.h"

class CMySerialPort;

class CBaseCommunicator{
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
};

class CSend : public CBaseCommunicator
{
//    QQueue< QByteArray > dataQueue;

public:
	CSend();
	~CSend();

    void sendString(const char* s,int len);
    void sendByte(char c);
    void enqueueString(char* s,int len);
    void dequeueAll();
};


class CRecvThread : public QThread , public CBaseCommunicator
{
	QMutex mutex;
public:
	CRecvThread();
	~CRecvThread();
	void run();

	bool closeRecv , recvClosed;
	QString readData();
	bool shootSensor[_MAX_NUM_PLAYERS];
};


class CCommunicator{
public:
	CCommunicator();
	~CCommunicator();

	CRecvThread *recvThread;
	CSend *send;
};

extern QMutex *recvThreadMutex;

#endif // COMMUNICATOR_H
