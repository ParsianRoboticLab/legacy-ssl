#include "mixteamreader.h"

MixTeamReader::MixTeamReader(QObject *parent) :
    QObject(parent)
{
    /*
    if (socket != NULL)
    {
        QObject::disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        delete socket;
    }
    */

    socket = new QUdpSocket(this);
    sharedRadioMC = new Net::UDP();
    if(!sharedRadioMC->open(conf()->LocalSettings_MixTeamPort(),true,true,false))
        qDebug() << "Unable to open Common Radio UDP network port: " << conf()->LocalSettings_MixTeamPort();
    else {
        Net::Address multiaddr,interface;
        multiaddr.setHost(conf()->LocalSettings_MixTeamIP().c_str(),conf()->LocalSettings_MixTeamPort());
        interface.setAny();
        if(!sharedRadioMC->addMulticast(multiaddr,interface))
            qDebug() << "Unable to setup UDP multicast for Common Radio";
        socket->setSocketDescriptor(sharedRadioMC->getFd());
        sharedRadioMC->setFd(-1);
    }
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));


    /*
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress(QString::fromStdString(conf()->LocalSettings_MixTeamIP())),conf()->LocalSettings_MixTeamPort());
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    */

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

//    multi_team_comm::TeamPlan tp;

    if(knowledge->kPlans != NULL)
        delete knowledge->kPlans;
    knowledge->kPlans = new multi_team_comm::TeamPlan();
    if( !knowledge->kPlans->ParseFromArray(buffer, buffer.size())){
        qDebug() << "ERROR";
        return;
    }
    qDebug() << "plan size:" << knowledge->kPlans->plans_size();

    knowledge->ssize = knowledge->kPlans->plans_size();
    knowledge->ready = true;
//    knowledge->activesInField.clear();//??bashe?


    for(int i = 0 ; i < knowledge->getActiveAgents().size() ; i++){
        knowledge->activesInField.append(knowledge->getActiveAgents().at(i));
    }
//    knowledge->activesInField = knowledge->getActiveAgents();

    if (policy()->Formation_GoalieFromGUI()) {
        knowledge->mixGoaleID = policy()->Formation_Goalie();
    } else {
        knowledge->mixGoaleID = wm->our.data->goalieID;
    }
}
