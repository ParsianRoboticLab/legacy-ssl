#include "exceptions.h"
#include "logger.h"

Exception::Exception(QString _name, QString _message, QString _invoker)
{
    name = _name;
    message = _message;
    invoker = _invoker;
}

QString Exception::getInvoker()
{
    return invoker;
}

QString Exception::getMessage()
{
    return message;
}

QString Exception::toString()
{
    return QString("%1 [%2]: %3").arg(name).arg(invoker).arg(message);
}

EOutOfBound::EOutOfBound(int i, int min, int max, QString _invoker)
    : Exception("EOutOfBound",
          QString("Index out of bound (%1) ; allowed range=[%2 %3]").arg(i).arg(min).arg(max),
          _invoker)
{
	debug("Exception! out of bound!", D_ERROR);
}
