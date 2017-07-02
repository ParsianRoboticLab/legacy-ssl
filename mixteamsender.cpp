#include "mixteamsender.h"
#include <QDebug>

MixTeamSender::MixTeamSender(QObject *parent) :
    QObject(parent)
{
    socket = new QUdpSocket(this);
    packet = NULL;
    flag = false;

    timer = new QTimer();
    timer->start(2000);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendData()));
}

MixTeamSender::~MixTeamSender()
{

}

void MixTeamSender::sendData()
{
    qDebug() << "S";
    QByteArray datagram;
    if(packet != NULL && flag)
    {
        datagram.resize(packet->ByteSize());
        bool success = packet->SerializeToArray(datagram.data(), datagram.size());
        if(!success) {
            qDebug() << "Serializing packet to array failed.";
        }
        mutex.lock();
        quint64 bytes_sent = socket->writeDatagram(datagram, QHostAddress::LocalHost, 1234);
        qDebug() << "sent = " << bytes_sent << ",   real = " << datagram.size();
        mutex.unlock();
        if (bytes_sent != datagram.size()) {
            qDebug() << QString("Sending UDP datagram failed (maybe too large?). Size was: %1 byte(s).").arg(datagram.size());
        }
    }
    delete packet;
    packet=NULL;
}
