#include <QDebug>
#include "communicator.h"
QMutex *recvThreadMutex;
#ifndef Q_OS_MAC

#include "libserial/qextserialport.h"
#include <sstream>
#include <fstream>

#include <logger.h>

using namespace std;


/*************************************** CMySerialPort Class ******************************************/

class CMySerialPort
{
public:
	QextSerialPort *serial_port;
    PortSettings portSettings;
	CMySerialPort() {
        setSerialParams(0, 0, 0, 0); //setup default values
        serial_port = NULL;
    }

    void setSerialParams(unsigned int _baud, unsigned int _charsize, unsigned int _parity, short int _stopbits)
    {
        switch (_baud)
        {
            case 50:    portSettings.BaudRate = BAUD50; break;
            case 75:    portSettings.BaudRate = BAUD75; break;
            case 110:   portSettings.BaudRate = BAUD110; break;
            case 134:   portSettings.BaudRate = BAUD134; break;
            case 150:   portSettings.BaudRate = BAUD150; break;
            case 200:   portSettings.BaudRate = BAUD200; break;
            case 300:   portSettings.BaudRate = BAUD300; break;
            case 600:   portSettings.BaudRate = BAUD600; break;
            case 1200:  portSettings.BaudRate = BAUD1200; break;
			case 1800:  portSettings.BaudRate = BAUD1800; break;
            case 2400:  portSettings.BaudRate = BAUD2400; break;
            case 4800:  portSettings.BaudRate = BAUD4800; break;
            case 9600:  portSettings.BaudRate = BAUD9600; break;
            case 19200: portSettings.BaudRate = BAUD19200; break;
            case 38400: portSettings.BaudRate = BAUD38400; break;
            case 57600: portSettings.BaudRate = BAUD57600; break;
            case 115200:portSettings.BaudRate = BAUD115200; break;
#ifndef Q_OS_MAC
                       case 230400:  portSettings.BaudRate = BAUD230400; break;
			case 460800:  portSettings.BaudRate = BAUD460800; break;
			case 500000:  portSettings.BaudRate = BAUD500000; break;
			case 576000:  portSettings.BaudRate = BAUD576000; break;
			case 921600: portSettings.BaudRate = BAUD921600; break;
			case 1000000: portSettings.BaudRate = BAUD1000000; break;
			case 1152000: portSettings.BaudRate = BAUD1152000; break;
			case 1500000:portSettings.BaudRate = BAUD1500000; break;
			case 2000000:  portSettings.BaudRate = BAUD2000000; break;
			case 2500000:  portSettings.BaudRate = BAUD2500000; break;
			case 3000000:  portSettings.BaudRate = BAUD3000000; break;
			case 3500000:  portSettings.BaudRate = BAUD3500000; break;
			case 4000000: portSettings.BaudRate = BAUD4000000; break;
#endif
            default:    portSettings.BaudRate = BAUD57600; break;
        }
        switch (_charsize)
        {
            case 5 : portSettings.DataBits = DATA_5; break;
            case 6 : portSettings.DataBits = DATA_6; break;
            case 7 : portSettings.DataBits = DATA_7; break;
            case 8 : portSettings.DataBits = DATA_8; break;
            default: portSettings.DataBits = DATA_8; break;
        }
        switch (_parity)
        {
            case 0 : portSettings.Parity = PAR_NONE; break;
            case 1 : portSettings.Parity = PAR_ODD; break;
            case 2 : portSettings.Parity = PAR_EVEN; break;
            default: portSettings.Parity = PAR_NONE; break;
        }
        // ********* check this part **********//
        switch (_stopbits){
            case 1: portSettings.StopBits = STOP_1; break;
#if defined(Q_OS_WIN) || defined(qdoc)
            case 33: portSettings.StopBits = STOP_1_5; break;
#endif
            case 2: portSettings.StopBits = STOP_2; break;
            default: portSettings.StopBits = STOP_1; break;
        }
        portSettings.FlowControl = FLOW_OFF;
    }
};

/*************************************** CBaseCommunicator Class ******************************************/

CBaseCommunicator::CBaseCommunicator()
{
    use_serial = false;
    use_udp    = false;
    udp_open   = false;
    serial_open= false;
    error      = false;
	p = new CMySerialPort();
    p->serial_port = NULL;
}

CBaseCommunicator::~CBaseCommunicator()
{
    closeSerial();
    closeUdp();
    delete p;
}


void CBaseCommunicator::setSerialParams(unsigned int baud, unsigned int charsize, unsigned int parity, short stopbits)
{
    p->setSerialParams(baud, charsize, parity, stopbits);
}

void CBaseCommunicator::connectSerial(const char* port)
{    
    closeSerial();
    _port = QString(port);
        p->serial_port = new QextSerialPort(p->portSettings);
    p->serial_port->setPortName(_port);
    if( p->serial_port->open(QIODevice::ReadWrite) == true ){
        serial_open = true;
        qDebug() << "successfully";
    }
    else{
        err = QString("Error occured, comunicator connection failed");
        serial_open = false;
        error = true;
        return;
    }
}

void CBaseCommunicator::connectUdp(const char* addr,int port)
{
    closeUdp();
    udpsocket.open(port, false, false, false);    
    _addr.setHost(addr, port);
    udp_open = true;
}

void CBaseCommunicator::closeSerial()
{
    if (p->serial_port==NULL) return;
    p->serial_port->close();
    delete p->serial_port;
    p->serial_port = NULL;
    serial_open = false;
}

void CBaseCommunicator::closeUdp()
{
    udpsocket.close();
    udp_open = false;
}


void CBaseCommunicator::activateUdp()      {use_udp = true;}
void CBaseCommunicator::deactivateUdp()    {use_udp = false;}
void CBaseCommunicator::activateSerial()   {use_serial = true;}
void CBaseCommunicator::deactivateSerial() {use_serial = false;}
bool CBaseCommunicator::errorOccured()     {return error;}
bool CBaseCommunicator::isUdpConnected()   {return udp_open;}
bool CBaseCommunicator::isSerialConnected(){return serial_open;}

QString CBaseCommunicator::getError()      {error=false;return err;}
QString CBaseCommunicator::getSerialPort() {return _port;}

/*************************************** CSend Class ******************************************/

CSend::CSend() : CBaseCommunicator()
{
//	dataQueue.clear();
}

CSend::~CSend()
{
}

void CSend::sendString(const char* s,int len)
{
	bool flag = false;
	if (use_udp && udp_open)
	{
		if (!udpsocket.send((const char *) s, len, _addr))
		{
			error=true;
			err = QString("Failed to send data to Udp socket");
		}
		else{
			flag = true;
		}
	}
	if (use_serial && serial_open)
	{
		/*if( */p->serial_port->write(s,len);/* == -1 ){
			error=true;
			err = QString("Failed to send data to Serial");
		}
		else
			flag = true;*/
	}
	if (!flag) {
		error = true;
		err  = QString("Warning: Sent nothing, first activate one of devices");
	}
}

void CSend::sendByte(char c)
{
	char ch = c;
	sendString(&ch,1);
}

void CSend::enqueueString(char *s, int len)
{
/*    mutex.lock();
	dataQueue.enqueue( QByteArray::fromRawData(s,len) );
	mutex.unlock();*/
}

void CSend::dequeueAll()
{
//	dataQueue.clear();
}

/*************************************** CRecvThread Class ******************************************/

CRecvThread::CRecvThread() : CBaseCommunicator()
{
	recvClosed = true;
}

CRecvThread::~CRecvThread()
{
}

void CRecvThread::run()
{
	bool found;
	int idx;
	stringstream ssin;
	fstream ffout;
	ffout.open("recieve.csv",ios::out);
	string vx , vy , vw;
	string packet_ID , old;

	recvThreadMutex->lock();
	recvClosed = false;
	closeRecv = false;
	recvThreadMutex->unlock();

	bool closeFlag;

	while(true)
	{
		recvThreadMutex->lock();
		closeFlag = closeRecv;
		recvThreadMutex->unlock();

		if( closeFlag )
			break;

		static QByteArray r;
		r.clear();

		recvThreadMutex->lock();
		int bytesSize = p->serial_port->bytesAvailable();
		if( bytesSize >= 18 ){
			r = p->serial_port->read(bytesSize);
//			qDebug() << "hereeeeeeee";
//			for( int i=0 ; i<r.size() ; i++ )
//				cout << (char)r.at(i);
//			cout << endl;

			found = false;
			int bakIdx;
			for( idx=8 ; idx<r.size() ; idx++ )
				if( r[idx] == ' ' && r[idx-8] == '*' ){
					found = true;
					bakIdx = idx-8;
				}

			idx = bakIdx;

			if( found && r.size() - idx >=  9 ){
				int kkk = 0;
				for( int j=0 ; j<5 ; j++ )
					kkk = kkk*10 + (quint8)(r[idx+2+j]-'0');
				if( r[idx+1] == '-' )
					kkk *= -1;

//				qDebug() << "saerargs h" << kkk ;
				knowledge->plotWidgetCustom[1] = kkk/4000.0;
//				knowledge->plotWidgetCustom[2] = res2;
	//			cout << res << endl;
			}
		}

		recvThreadMutex->unlock();
		msleep(1);
    }
	recvThreadMutex->lock();
	recvClosed = true;
	recvThreadMutex->unlock();
	qDebug() << "Reacive thread ended.";
}

/*************************************** CCommunicator Class ******************************************/

CCommunicator::CCommunicator(){
	send = new CSend();
	recvThread = new CRecvThread();
}

CCommunicator::~CCommunicator(){
	delete recvThread;
	delete send;
}


#else
//TODO: QextSerial should be recompiled for mac64
using namespace std;

/*************************************** CMySerialPort Class ******************************************/


/*************************************** CBaseCommunicator Class ******************************************/

CBaseCommunicator::CBaseCommunicator()
{
    use_serial = false;
    use_udp    = false;
    udp_open   = false;
    serial_open= false;
    error      = false;
}

CBaseCommunicator::~CBaseCommunicator()
{
    closeUdp();
}


void CBaseCommunicator::setSerialParams(unsigned int baud, unsigned int charsize, unsigned int parity, short stopbits)
{
}

void CBaseCommunicator::connectSerial(const char* port)
{
}

void CBaseCommunicator::connectUdp(const char* addr,int port)
{
    closeUdp();
    udpsocket.open(port, false, false, false);
    _addr.setHost(addr, port);
    udp_open = true;
}

void CBaseCommunicator::closeSerial()
{}

void CBaseCommunicator::closeUdp()
{
    udpsocket.close();
    udp_open = false;
}


void CBaseCommunicator::activateUdp()      {use_udp = true;}
void CBaseCommunicator::deactivateUdp()    {use_udp = false;}
void CBaseCommunicator::activateSerial()   {use_serial = true;}
void CBaseCommunicator::deactivateSerial() {use_serial = false;}
bool CBaseCommunicator::errorOccured()     {return error;}
bool CBaseCommunicator::isUdpConnected()   {return udp_open;}
bool CBaseCommunicator::isSerialConnected(){return serial_open;}

QString CBaseCommunicator::getError()      {error=false;return err;}
QString CBaseCommunicator::getSerialPort() {return _port;}

/*************************************** CSend Class ******************************************/

CSend::CSend() : CBaseCommunicator()
{
        dataQueue.clear();
}

CSend::~CSend()
{
}

void CSend::sendString(const char* s,int len)
{
        bool flag = false;
        if (use_udp && udp_open)
        {
                if (!udpsocket.send((const char *) s, len, _addr))
                {
                        error=true;
                        err = QString("Failed to send data to Udp socket");
                }
                else
                        flag = true;
        }
        if (!flag) {
                error = true;
                err  = QString("Warning: Sent nothing, first activate one of devices");
        }
}

void CSend::sendByte(char c)
{
        char ch = c;
        sendString(&ch,1);
}

void CSend::enqueueString(char *s, int len)
{
/*    mutex.lock();
        dataQueue.enqueue( QByteArray::fromRawData(s,len) );
        mutex.unlock();*/
}

void CSend::dequeueAll()
{
        dataQueue.clear();
}

/*************************************** CRecvThread Class ******************************************/

CRecvThread::CRecvThread() : CBaseCommunicator()
{
        recvClosed = true;
}

CRecvThread::~CRecvThread()
{
}

void CRecvThread::run()
{
}

/*************************************** CCommunicator Class ******************************************/

CCommunicator::CCommunicator(){
        send = new CSend();
        recvThread = new CRecvThread();
}

CCommunicator::~CCommunicator(){
        delete recvThread;
        delete send;
}

#endif
