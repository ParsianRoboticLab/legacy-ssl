#include "plays/theirpenalty.h"
#include "roles/playmake.h"
#include "soccer.h"

CTheirPenalty::CTheirPenalty(){
}

CTheirPenalty::~CTheirPenalty(){

}

void CTheirPenalty::reset(){
	position.reset();
	executedCycles = 0;
}

void CTheirPenalty::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
	setAgentsID(_agents);
	setEditData(_editData);
	initMaster();

	if( knowledge->getLastPlayExecuted() != TheirPenaltyPlay ){
		reset();
	}
	knowledge->setLastPlayExecuted(TheirPenaltyPlay);
}

void CTheirPenalty::execute_0(){

}

void CTheirPenalty::execute_1(){
	executedCycles++;
        if (wm->gs->penalty_shootout()) {
            choosePlayMaker();
        }
            appendRemainingsAgents(positionAgents);

    setFormation("TheirP");
}

void CTheirPenalty::execute_2(){
	executedCycles++;

	appendRemainingsAgents(positionAgents);

    setFormation("TheirP");
}

void CTheirPenalty::execute_3(){
	executedCycles++;

	appendRemainingsAgents(positionAgents);

    setFormation("TheirP");
}

void CTheirPenalty::execute_4(){
	executedCycles++;

	appendRemainingsAgents(positionAgents);

    setFormation("TheirP");
}

void CTheirPenalty::execute_5(){
	executedCycles++;

	appendRemainingsAgents(positionAgents);

    setFormation("TheirP");
}

void CTheirPenalty::execute_6(){
	executedCycles++;

	appendRemainingsAgents(positionAgents);

    setFormation("TheirP");
}
