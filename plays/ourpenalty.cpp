#include "plays/ourpenalty.h"
#include "roles/playmake.h"
#include "soccer.h"

COurPenalty::COurPenalty(){
}

COurPenalty::~COurPenalty(){

}

void COurPenalty::reset(){
	position.reset();
	executedCycles = 0;
}

void COurPenalty::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
	setAgentsID(_agents);
	setEditData(_editData);
	initMaster();

	if( knowledge->getLastPlayExecuted() != OurPenaltyPlay ){
		reset();
	}
	knowledge->setLastPlayExecuted(OurPenaltyPlay);
}

void COurPenalty::penaltyKick(){
    executedCycles++;
    if (wm->gs->penalty_shootout()) {
        playOnFlag = false;
    }
    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    setFormation("OurP");

}

void COurPenalty::execute_0(){
    penaltyKick();
}

void COurPenalty::execute_1(){
    penaltyKick();
}

void COurPenalty::execute_2(){
    penaltyKick();
}

void COurPenalty::execute_3(){
    penaltyKick();
}

void COurPenalty::execute_4(){
    penaltyKick();
}

void COurPenalty::execute_5(){
    penaltyKick();
}

void COurPenalty::execute_6(){
    penaltyKick();
}
