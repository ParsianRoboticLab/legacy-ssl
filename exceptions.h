#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <QString>

class Exception
{
private:
    QString message;
    QString invoker;
    QString name;
public:
    Exception(QString _name, QString _message, QString _invoker);
    QString getInvoker();
    QString getMessage();
    QString getName();
    QString toString();
};

class EOutOfBound : public Exception
{
public:
    EOutOfBound(int i, int min, int max, QString _invoker);
};

#endif // EXCEPTIONS_H
