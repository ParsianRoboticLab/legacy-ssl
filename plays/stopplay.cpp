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

    for (int i = 0;i < 6 ; i++) {
        if(conf()->LocalSettings_LineUpPosition() == "OurCornerL") {
            rolePosition[i] = Vector2D(0.25*i-0.5, _FIELD_HEIGHT/2);

        } else if(conf()->LocalSettings_LineUpPosition() == "OurCornerR") {
            rolePosition[i] = Vector2D(0.25*i-0.5, -_FIELD_HEIGHT/2);

        } else if(conf()->LocalSettings_LineUpPosition() == "parsian") {
            if (conf()->LocalSettings_OurTeamSide() == "Left") {
                rolePosition[i] = Vector2D(0.25*i-1, -_FIELD_HEIGHT/2);

            } else {
                rolePosition[i] = Vector2D((-0.25*i+1), -_FIELD_HEIGHT/2);

            }
        }

    }

    for(int i = 0; i < agentsID.size(); i++) {
        gpa[i]->init(rolePosition[i],wm->field->oppGoal());
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
