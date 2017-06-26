#include "chalftimelineup.h"

CHalftimeLineup::CHalftimeLineup()
{



}
void CHalftimeLineup::reset(){

}

void CHalftimeLineup::init(QList<int> _agents, QMap<QString, EditData *> *_editData){
    setAgentsID(_agents);
    setEditData(_editData);
    initMaster();

    if( knowledge->getLastPlayExecuted() != HalfTimeLineUp ){
        reset();
    }
    knowledge->setLastPlayExecuted(HalfTimeLineUp);
}
void CHalftimeLineup::execute_0(){

}

void CHalftimeLineup::execute_1(){

}

void CHalftimeLineup::execute_2(){

}

void CHalftimeLineup::execute_3(){

}

void CHalftimeLineup::execute_4(){

}

void CHalftimeLineup::execute_5(){

}

void CHalftimeLineup::execute_6(){

}
