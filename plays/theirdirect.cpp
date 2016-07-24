#include "plays/theirdirect.h"
#include "roles/playmake.h"
#include "soccer.h"

CTheirDirect::CTheirDirect(){
}

CTheirDirect::~CTheirDirect(){

}

void CTheirDirect::reset(){
	executedCycles = 0;

}

void CTheirDirect::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
	setAgentsID(_agents);
	setEditData(_editData);
	initMaster();

	if( knowledge->getLastPlayExecuted() != TheirDirectPlay ){
		reset();
	}
	knowledge->setLastPlayExecuted(TheirDirectPlay);
}

void CTheirDirect::execute_0(){

}

void CTheirDirect::execute_1(){
	executedCycles++;

	chooseBlocker();
}

void CTheirDirect::execute_2(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}

void CTheirDirect::execute_3(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}

void CTheirDirect::execute_4(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}

void CTheirDirect::execute_5(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}

void CTheirDirect::execute_6(){
	executedCycles++;

	chooseBlocker();

	appendRemainingsAgents(markAgents);
}
