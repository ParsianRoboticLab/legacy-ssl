#include "robotstatussender.h"

CRobotStatusSender::CRobotStatusSender(int ms, QObject* parent) : QObject(parent) {
    socket = new QUdpSocket(this);
    packet = NULL;
//    dataReady = false;

    timer = new QTimer();
    timer->start(ms);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendData()));
}

CRobotStatusSender::~CRobotStatusSender() {

}

void CRobotStatusSender::sendData() {
    if (!conf()->LocalSettings_sendRobotStatus()) return;

    QByteArray datagram;
    packet = new robot_status::Status;
    fillPacket(packet);

    if(packet != NULL)
    {
        datagram.resize(packet->ByteSize());
        bool success = packet->SerializeToArray(datagram.data(), datagram.size());
        if(!success) {
            qDebug() << "Serializing packet to array failed.";
        }
        mutex.lock();
        quint64 bytes_sent = socket->writeDatagram(datagram, QHostAddress(QString::fromStdString(conf()->LocalSettings_MixTeamIP())), conf()->LocalSettings_MixTeamPort());
        qDebug() << "sent = " << bytes_sent << ",   real = " << datagram.size();
        mutex.unlock();
        if (bytes_sent != datagram.size()) {
            qDebug() << QString("Sending UDP datagram failed (maybe too large?). Size was: %1 byte(s).").arg(datagram.size());
        }

//        dataReady = false;

    }
    delete packet;
    packet = NULL;
}

void CRobotStatusSender::fillPacket(robot_status::Status *_packet) {
    for (int i = 0; i < knowledge->getActiveAgents().size(); i++) {
        CAgent* temp = knowledge->getActiveAgents().at(i);
        robot_status::Robot_Packet* robot = _packet->add_robots();
        robot->set_id(temp->self()->id);
        robot->set_battery(temp->status.battery);
        robot->set_spin(temp->status.spin);
        robot->set_shootsensor(temp->status.shotSensor);
        robot->set_shootvoltage(temp->status.capCharge);
        robot->set_boardid(temp->status.boardID);
        robot->set_fault(temp->status.fault);
        robot->set_failed(temp->status.faild);
        robot->set_halt(temp->status.halt);
        robot->set_datalost(temp->status.dataLost);

        robot_status::Fault* tempFault = robot->mutable_faultmessage();
        tempFault->set_shootboard(temp->status.shotBoard);
        tempFault->set_kickfault(temp->status.kickFault);
        tempFault->set_chipfault(temp->status.chipFault);
        tempFault->set_encodermotor1(temp->status.encoderFault[0]);
        tempFault->set_encodermotor2(temp->status.encoderFault[1]);
        tempFault->set_encodermotor3(temp->status.encoderFault[2]);
        tempFault->set_encodermotor4(temp->status.encoderFault[3]);
        tempFault->set_shootsensor(temp->status.shotSensorFault);
        tempFault->set_beep(temp->status.beep);

        robot_status::Fail* tempFail = robot->mutable_failedmessage();
        tempFail->set_motor1(temp->status.motorFault[0]);
        tempFail->set_motor2(temp->status.motorFault[1]);
        tempFail->set_motor3(temp->status.motorFault[2]);
        tempFail->set_motor4(temp->status.motorFault[3]);
        tempFail->set_motor5(temp->status.motorFault[4]);
    }
}
