#include <QDebug>
#include "communicator.h"
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
        portSettings.Timeout_Millisec = 40;
    }
};

/*************************************** CBaseCommunicator Class ******************************************/

CBaseCommunicator::CBaseCommunicator() : QObject()
{
    use_serial = false;
    use_udp    = false;
    udp_open   = false;
    serial_open= false;
    error      = false;
    p = new CMySerialPort();
    recTime = new QTimer;
    recTime->setInterval(100);
    recTime->start();
    p->serial_port = NULL;
    PortSettings settings = {BAUD115200, DATA_8, PAR_NONE, STOP_1, FLOW_OFF,40};
    rec_serialPort = new QextSerialPort(conf()->LocalSettings_SerialRec().c_str(),settings,QextSerialPort::EventDriven);
    //rec_serialPort->open(QIODevice::ReadOnly);
    recDataFlow.clear();
    for(int i = 0 ; i < 16 ; i++)
    {
        for(int j = 0 ; j < 13 ; j ++)
        {
            robotPacket[i][j]=0;
        }
    }

    for(int i = 0 ; i < 12 ; i++)
    {
        onlineRobotsTimer[i].start();
    }
    //connect(p->serial_port,SIGNAL(readyRead()),this,SLOT(readData()));
}

void CBaseCommunicator::readData()
{

    if(wm->getIsSimulMode())
    {

        p->serial_port->flush();
        return;
    }
    QByteArray dataFlow;
    if(p->serial_port->bytesAvailable())
        dataFlow = p->serial_port->read(p->serial_port->bytesAvailable());
    p->serial_port->flush();
    recDataFlow.append(dataFlow);
    if(recDataFlow.size() > 100)
    {
        recDataFlow.remove(0,recDataFlow.size()/2);
    }

    debug(QString("SIZE : %1").arg(recDataFlow.size()), D_MHMMD);
    if(recDataFlow.size())
    {

        for(int i = 0 ; i < recDataFlow.size() ; i++) {
            if(recDataFlow[i] == 0x99)
            {

                if(i >= 12)
                {

                    for(int j = i-1 ; j >= i - 12 ; j--)
                    {
                        robotPacket[recDataFlow[i-12]][j] = recDataFlow[j];
                        onlineRobotsTimer[recDataFlow[i-12]].restart();
                    }
                    if (!recDataFlow.isEmpty())
                    {
                        recDataFlow.remove(0,i+1);
                        i = 0;
                    }

                }
                else
                {
                    if (!recDataFlow.isEmpty())
                    {
                        recDataFlow.remove(0,i+1);
                        i =0;
                    }
                }

            }
        }
    }

    knowledge->onlineRobots.clear();
    for(int i = 0 ; i < 12 ; i++)
    {
        knowledge->onlineRobots.append(i);
        if(onlineRobotsTimer[i].elapsed() > 200)
        {
            knowledge->onlineRobots.removeOne(i);
        }
    }

    debug(QString("id : %1").arg((int)robotPacket[4][1]), D_MHMMD);
    for (int i = 0; i < _MAX_NUM_PLAYERS; i++) {
        knowledge->getAgent(i)->setShootSensor(robotPacket[i][1] & 0x01);
        knowledge->getAgent(i)->changeIsNeeded = robotPacket[i][4] & 0b11000000;
        if (knowledge->getAgent(i)->changeIsNeeded) {
            debug(QString("[ROBOT FAULT] %1"), D_ERROR);
        }
    }
}

CBaseCommunicator::~CBaseCommunicator()
{
    rec_serialPort->close();
    delete rec_serialPort;
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
        connect(recTime,SIGNAL(timeout()),this,SLOT(readData()));
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

CCommunicator::CCommunicator() : CBaseCommunicator()
{
    //	dataQueue.clear();
}

CCommunicator::~CCommunicator()
{
}

void CCommunicator::sendString(const char* s,int len)
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

void CCommunicator::sendByte(char c)
{
    char ch = c;
    sendString(&ch,1);
}

void CCommunicator::enqueueString(char *s, int len)
{
    /*    mutex.lock();
 dataQueue.enqueue( QByteArray::fromRawData(s,len) );
 mutex.unlock();*/
}

void CCommunicator::dequeueAll()
{
    //	dataQueue.clear();
}
