#include "mixteamreader.h"

MixTeamReader::MixTeamReader(QObject *parent) :
    QObject(parent)
{
    socket = new QUdpSocket(this);
    socket->bind(1234);//if we just use port, it can receive!
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

MixTeamReader::~MixTeamReader()
{

}

void MixTeamReader::readyRead()
{
    qDebug() << "read";
    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());
    qDebug() << "size : " << socket->pendingDatagramSize();

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
