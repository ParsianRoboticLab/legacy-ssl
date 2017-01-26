#include "plays/ourballplacement.h"

COurBallPlacement::COurBallPlacement(){

}

COurBallPlacement::~COurBallPlacement(){

}

void COurBallPlacement::reset(){

}

void COurBallPlacement::init(QList<int> _agents, QMap<QString, EditData *> *_editData){
    setAgentsID(_agents);
    setEditData(_editData);
    initMaster();

    if( knowledge->getLastPlayExecuted() != OurBallPlacement ){
        reset();
    }
    knowledge->setLastPlayExecuted(OurBallPlacement);
}


void COurBallPlacement::execute_0(){

}

void COurBallPlacement::execute_1(){

}

void COurBallPlacement::execute_2(){

}

void COurBallPlacement::execute_3(){

}

void COurBallPlacement::execute_4(){

}

void COurBallPlacement::execute_5(){

}

void COurBallPlacement::execute_6(){

}
