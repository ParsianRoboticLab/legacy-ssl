#include "mixteamthread.h"
#include <QDebug>

MixTeamThread::MixTeamThread(QObject *parent)
{
    qDebug() <<"debuge chert";
    socket = new QUdpSocket(this);
    //socket->bind(QHostAddress::LocalHost, 1234);
    packet = NULL;
    flag = false;
    //connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    timer = new QTimer();
    timer->start(2000);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendData()));
}
MixTeamThread::~MixTeamThread()
{

}

void MixTeamThread::run()
{
    //sendData
    //timer->start();
}

void MixTeamThread::sendData()
{
    qDebug() << "S";
    QByteArray datagram;
    if(packet != NULL && flag)
    {
        datagram.resize(packet->ByteSize());
        bool success = packet->SerializeToArray(datagram.data(), datagram.size());
        //success = false;
        if(!success) {
            //TODO: print useful info
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

void MixTeamThread::readyRead()
{
    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    multi_team_comm::TeamPlan tp;
    if( !tp.ParseFromArray(buffer, buffer.size())){
        qDebug() << "ERROR";
        return;
    }
    qDebug() << "plan size:" << tp.plans_size();
}
