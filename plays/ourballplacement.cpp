#include "plays/ourballplacement.h"
#include "skills/autoballplacement.h"

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
    debug("execute_0 is running", D_ATOUSA);
    debug("ballPlacement execute_0", D_ERROR);
}

void COurBallPlacement::execute_1(){

    debug("execute_1 is running", D_ATOUSA);

    static CSkillAutoBallPlacement *abp = new CSkillAutoBallPlacement(knowledge->getAgent(agentsID.at(0)));
    abp->execute();
}

void COurBallPlacement::execute_2(){
    debug("execute_2 is running", D_ATOUSA);

    Vector2D ballPos = Vector2D(0,3);//??????
    float distt = 0;
    float minDist = 50;
    int minIndex = 0;
    for( int i = 0 ; i < agentsID.size(); i++ ){
        distt = knowledge->getAgent(agentsID.at(i))->pos().dist(ballPos);
        if( distt < minDist ){
            minDist = distt;
            minIndex = i;
        }
    }
    debug("*************************", D_ATOUSA);
    static CSkillAutoBallPlacement *abp = new CSkillAutoBallPlacement(knowledge->getAgent(agentsID.at(minIndex)));
    abp->execute();
}

void COurBallPlacement::execute_3(){
    debug("execute_3 is running", D_ATOUSA);

    Vector2D ballPos = Vector2D(0,3);//??????
    float distt = 0;
    float minDist = 50;
    int minIndex = 0;
    for( int i = 0 ; i < agentsID.size(); i++ ){
        distt = knowledge->getAgent(agentsID.at(i))->pos().dist(ballPos);
        if( distt < minDist ){
            minDist = distt;
            minIndex = i;
        }
    }
    debug("*************************", D_ATOUSA);
    static CSkillAutoBallPlacement *abp = new CSkillAutoBallPlacement(knowledge->getAgent(agentsID.at(minIndex)));
    abp->execute();
}

void COurBallPlacement::execute_4(){
    debug("execute_4 is running", D_ATOUSA);

    Vector2D ballPos = Vector2D(0,3);//??????
    float distt = 0;
    float minDist = 50;
    int minIndex = 0;
    for( int i = 0 ; i < agentsID.size(); i++ ){
        distt = knowledge->getAgent(agentsID.at(i))->pos().dist(ballPos);
        if( distt < minDist ){
            minDist = distt;
            minIndex = i;
        }
    }
    debug("*************************", D_ATOUSA);
    static CSkillAutoBallPlacement *abp = new CSkillAutoBallPlacement(knowledge->getAgent(agentsID.at(minIndex)));
    abp->execute();

}

void COurBallPlacement::execute_5(){
    debug("execute_5 is running", D_ATOUSA);

    Vector2D ballPos = Vector2D(0,3);//??????
    float distt = 0;
    float minDist = 50;
    int minIndex = 0;
    for( int i = 0 ; i < agentsID.size(); i++ ){
        distt = knowledge->getAgent(agentsID.at(i))->pos().dist(ballPos);
        if( distt < minDist ){
            minDist = distt;
            minIndex = i;
        }
    }
    debug("*************************", D_ATOUSA);
    static CSkillAutoBallPlacement *abp = new CSkillAutoBallPlacement(knowledge->getAgent(agentsID.at(minIndex)));
    abp->execute();
}

void COurBallPlacement::execute_6(){
    debug("execute_6 is running", D_ATOUSA);

    Vector2D ballPos = wm->ball->pos;
    float distt = 0;
    float minDist = 50;
    int minIndex = 0;
    for( int i = 0 ; i < agentsID.size(); i++ ){
        distt = knowledge->getAgent(agentsID.at(i))->pos().dist(ballPos);
        if( distt < minDist ){
            minDist = distt;
            minIndex = i;
        }
    }
    debug("*************************", D_ATOUSA);
    static CSkillAutoBallPlacement *abp = new CSkillAutoBallPlacement(knowledge->getAgent(agentsID.at(minIndex)));
    abp->execute();
}
