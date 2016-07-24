#include "serialwrapper.h"

#include <QDebug>

#include <fstream>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace LibSerial;

CSerialWrapper::CSerialWrapper(const char* pcCommPort, LibSerial::SerialStreamBuf::BaudRateEnum baudRate, LibSerial::SerialStreamBuf::CharSizeEnum charSize, LibSerial::SerialStreamBuf::ParityEnum parity,short int stopBits, QString &err)
{
    serial_port.Open( pcCommPort ) ;
    if ( ! serial_port.good() )
    {
        err = QString("Could not open port.");
        return;
    }
    serial_port.SetBaudRate( baudRate ) ;
    if ( ! serial_port.good() )
    {
        err = QString("Could not set baud rate.");
        return;
    }
    serial_port.SetCharSize( charSize ) ;
    if ( ! serial_port.good() )
    {
        err = QString("Error: Could not set the character size.");
        return;
    }
    serial_port.SetParity( parity ) ;
    if ( ! serial_port.good() )
    {
        err = QString("Error: Could not disable the parity.");
        return;
    }
    serial_port.SetNumOfStopBits( stopBits ) ;
    if ( ! serial_port.good() )
    {
        err = QString("Error: Could not set the number of stop bits.");
        return;
    }
    //
    // Turn on hardware flow control.
    //
    serial_port.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE ) ;
    if ( ! serial_port.good() )
    {
        err = QString("Error: Could not use hardware flow control.");
        return;
    }
    //
    // Do not skip whitespace characters while reading from the
    // serial port.
    //
    // serial_port.unsetf( std::ios_base::skipws ) ;
    sss = new std::ofstream("./data");
}

CSerialWrapper::~CSerialWrapper()
{
    delete sss;
}

char CSerialWrapper::receiveByte(){
	char buf[1];
        serial_port.read(buf,1);
	return buf[0];
}
void CSerialWrapper::sendByte(char ch)
{
        char buf[1];
        buf[0] = ch;
        serial_port.write(buf,1);
}

void CSerialWrapper::sendString(char *s, int len)
{

    (*sss) << s;
        qDebug() << "sending";
        serial_port.write(s,len);

        qDebug() << "sent:" << s;
}
