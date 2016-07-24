#include "plays/theirkickoff.h"
#include "roles/playmake.h"
#include "soccer.h"

CTheirKickOff::CTheirKickOff(){
}

CTheirKickOff::~CTheirKickOff(){

}

void CTheirKickOff::reset(){
	executedCycles = 0;
}

void CTheirKickOff::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
	setAgentsID(_agents);
	setEditData(_editData);
	initMaster();

	if( knowledge->getLastPlayExecuted() != TheirKickOffPlay ){
		reset();
	}
	knowledge->setLastPlayExecuted(TheirKickOffPlay);
}

void CTheirKickOff::execute_0(){

}

void CTheirKickOff::execute_1(){
	executedCycles++;

	chooseBlocker();
}

void CTheirKickOff::execute_2(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}

void CTheirKickOff::execute_3(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}

void CTheirKickOff::execute_4(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}

void CTheirKickOff::execute_5(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}

void CTheirKickOff::execute_6(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}
