#include "plays/theirballplacement.h"

CTheirBallPlacement::CTheirBallPlacement(){

}

CTheirBallPlacement::~CTheirBallPlacement(){

}

void CTheirBallPlacement::reset(){

}

void CTheirBallPlacement::init(QList<int> _agents, QMap<QString, EditData *> *_editData){
    setAgentsID(_agents);
    setEditData(_editData);
    initMaster();

    if( knowledge->getLastPlayExecuted() != TheirBallPlacement ){
        reset();
    }
    knowledge->setLastPlayExecuted(TheirBallPlacement);
}


void CTheirBallPlacement::execute_0(){

}

void CTheirBallPlacement::execute_1(){

}

void CTheirBallPlacement::execute_2(){

}

void CTheirBallPlacement::execute_3(){

}

void CTheirBallPlacement::execute_4(){

}

void CTheirBallPlacement::execute_5(){

}

void CTheirBallPlacement::execute_6(){

}
