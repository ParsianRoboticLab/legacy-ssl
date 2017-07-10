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
    agents.clear();
    points.clear();
    for(int i=0;i<agentsID.count();i++){
        lineupAgent=new CSkillGotoPointAvoid(knowledge->getAgent(agentsID.at(i)));
        lineup.append(lineupAgent);
        agents.append(knowledge->getAgent(agentsID.at(i)));
    }




    if( knowledge->getLastPlayExecuted() != HalfTimeLineUp ){
        reset();
    }
    knowledge->setLastPlayExecuted(HalfTimeLineUp);
}

void CHalftimeLineup::lineUpAllAgents(){

    for(int i=0;i<agentsID.length();i++){
        if(conf()->LocalSettings_LineUpPosition() == "OurCornerL")
            points.append(wm->field->ourCornerL()+Vector2D(0.25*i+0.1,-0.2));
        else if(conf()->LocalSettings_LineUpPosition() == "OurCornerR")
            points.append(wm->field->ourCornerR()+Vector2D(0.25*i+0.1,0.2));
        else if(conf()->LocalSettings_LineUpPosition() == "parsian")
            points.append(wm->field->center()+Vector2D(i*-0.25,-3.3));



    }
    if (agents.size() == 0) return;
    knowledge->Matching(agents,points,matchpoints);
    for(int i=0;i<agentsID.length();i++){
        lineup.at(i)->init(points[matchpoints.at(i)],Vector2D(0,1));
        lineup.at(i)->setAvoidPenaltyArea(false);
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
