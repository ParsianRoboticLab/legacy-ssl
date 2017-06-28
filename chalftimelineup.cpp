#include "chalftimelineup.h"
#include "ui/widgets.h"

CHalftimeLineup::CHalftimeLineup()
{



}
void CHalftimeLineup::reset(){

}

void CHalftimeLineup::init(QList<int> _agents, QMap<QString, EditData *> *_editData){
    setAgentsID(_agents);
    setEditData(_editData);
    initMaster();
    for(int i=0;i<agentsID.length();i++){
        lineupAgent=new CSkillGotoPointAvoid(knowledge->getAgent(agentsID.at(i)));
        lineup.append(lineupAgent);
    }



    if( knowledge->getLastPlayExecuted() != HalfTimeLineUp ){
        reset();
    }
    knowledge->setLastPlayExecuted(HalfTimeLineUp);
}

void CHalftimeLineup::lineUpAllAgents(){

    for(int i=0;i<agentsID.length();i++){
        if(conf()->LocalSettings_LineUpPosition() == "OurCornerL")
            lineup.at(i)->init(wm->field->ourCornerL()+Vector2D(0.25*i,0.2),Vector2D(1,0));
        else if(conf()->LocalSettings_LineUpPosition() == "OurCornerR")
            lineup.at(i)->init(wm->field->ourCornerR()+Vector2D(0.25*i,0.2),Vector2D(1,0));
        else if(conf()->LocalSettings_LineUpPosition() == "parsian")
            lineup.at(i)->init(wm->field->center()+Vector2D(i*-0.25,-3.5),Vector2D(1,0));


        lineup.at(i)->execute();

    }
}

void CHalftimeLineup::execute_0(){
    lineUpAllAgents();

}

void CHalftimeLineup::execute_1(){

    lineUpAllAgents();
}

void CHalftimeLineup::execute_2(){
    lineUpAllAgents();

}

void CHalftimeLineup::execute_3(){
    lineUpAllAgents();

}

void CHalftimeLineup::execute_4(){
    lineUpAllAgents();

}

void CHalftimeLineup::execute_5(){
    lineUpAllAgents();

}

void CHalftimeLineup::execute_6(){
    lineUpAllAgents();

}
