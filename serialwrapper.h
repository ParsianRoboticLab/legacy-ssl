#ifndef SERIALWRAPPER
#define SERIALWRAPPER

#include <libserial/SerialStream.h>

#include <QString>

#include <fstream>
class CSerialWrapper  
{
private:
    std::ofstream *sss;
public:
        LibSerial::SerialStream serial_port;
	void sendString (char *s, int len);
	void sendByte(char ch);
	char receiveByte();
        CSerialWrapper(const char* pcCommPort, LibSerial::SerialStreamBuf::BaudRateEnum baudRate, LibSerial::SerialStreamBuf::CharSizeEnum charSize, LibSerial::SerialStreamBuf::ParityEnum parity,short int stopBits, QString &err);
	virtual ~CSerialWrapper();

};

#endif
