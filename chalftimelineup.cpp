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





    if( knowledge->getLastPlayExecuted() != HalfTimeLineUp ){
        reset();
    }
    knowledge->setLastPlayExecuted(HalfTimeLineUp);
}

void CHalftimeLineup::lineUpAllAgents(){

    agents.clear();
    points.clear();
    for(int i=0;i<agentsID.count();i++){
        lineupAgent=new CSkillGotoPointAvoid(knowledge->getAgent(agentsID.at(i)));
        lineup.append(lineupAgent);
        agents.append(knowledge->getAgent(agentsID.at(i)));
    }

    for(int i=0;i<agentsID.length();i++){
        if(conf()->LocalSettings_LineUpPosition() == "OurCornerL")
            points.append(wm->field->ourCornerL()+Vector2D(0.25*i+0.1,-0.2));
        else if(conf()->LocalSettings_LineUpPosition() == "OurCornerR")
            points.append(wm->field->ourCornerR()+Vector2D(0.25*i+0.1,0.2));
        else if(conf()->LocalSettings_LineUpPosition() == "parsian")
            points.append(wm->field->center()+Vector2D(i*-0.25,-3.3));

    }

    if(!haltRobots){
        debug("residan",D_NADIA);
        knowledge->Matching(agents,points,matchpoints);

        for(int i=0;i<agentsID.length();i++){
            lineup.at(i)->init(points[matchpoints.at(i)],Vector2D(0,1));
            lineup.at(i)->setAvoidPenaltyArea(false);
            lineup.at(i)->execute();
        }

        for(int i=0;i<agentsID.length();i++){
            if(Circle2D(knowledge->getAgent(agentsID.at(i))->pos(),0.1).contains(points[matchpoints.at(i)]))
                locatedRobots++;
        }

        if(locatedRobots==agentsID.length()){
            haltRobots=true;
        }
        else{
            locatedRobots=0;
            haltRobots=false;
        }
    }
    else{
        debug("reset flag", D_NADIA);
        haltRobots=false;
        locatedRobots=0;
        wm->gs->transition('H');
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
