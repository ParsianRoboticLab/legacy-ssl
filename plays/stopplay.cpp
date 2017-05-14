#include "stopplay.h"

CStopPlay::CStopPlay() : CMasterPlay() {

}

CStopPlay::~CStopPlay(){

}

void CStopPlay::reset(){
    position.reset();
    executedCycles = 0;
}

void CStopPlay::init(QList<int> _agents, QMap<QString, EditData *> *_editData){
    setAgentsID(_agents);
    setEditData(_editData);
    initMaster();

    if( knowledge->getLastPlayExecuted() != StopPlay ){
        reset();
    }
    knowledge->setLastPlayExecuted(StopPlay);
}

void CStopPlay::stopPosition() {
    executedCycles++;
    appendRemainingsAgents(positionAgents);
    setFormation("OurF");
}

void CStopPlay::execute_0(){
    stopPosition();
}

void CStopPlay::execute_1(){
    stopPosition();

}

void CStopPlay::execute_2(){
    stopPosition();

}

void CStopPlay::execute_3(){
    stopPosition();

}

void CStopPlay::execute_4(){
    stopPosition();

}

void CStopPlay::execute_5(){
    stopPosition();

}

void CStopPlay::execute_6(){
    stopPosition();

}
