#include "stopplay.h"

CStopPlay::CStopPlay() : CMasterPlay() {
    for (int i = 0; i < 6 ; i++) {
        gpa[i] = new CSkillGotoPointAvoid(NULL);
        gpa[i]->setSlowMode(true);
        gpa[i]->setNoAvoid(false);
    }
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
    setFormation("Stop7");

    Vector2D temp;

    for (int i = 0;i < 6 ; i++) {
        if ((*editData)["Stop7"] != NULL
        && (*editData)["Stop7"]->formation()->getRoleName(i+1) == "Position") {
            temp = (*editData)["Stop7"]->formation().get()->getPosition(i+1, wm->ball->pos);
            rolePosition[i] = temp;
        }
    }

    for(int i = 0; i < agentsID.size(); i++) {
        gpa[i]->init(rolePosition[5-i],wm->field->oppGoal());
        gpa[i]->setAgent(knowledge->getAgent(agentsID.at(i)));
        gpa[i]->execute();
    }



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
