#include "plays/ourindirect.h"
#include "roles/playmake.h"
#include "soccer.h"

COurIndirect::COurIndirect(){
}

COurIndirect::~COurIndirect(){

}

void COurIndirect::reset(){
	position.reset();
	executedCycles = 0;
}

void COurIndirect::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
	setAgentsID(_agents);
	setEditData(_editData);
	initMaster();

	if( knowledge->getLastPlayExecuted() != OurIndirectPlay ){
		reset();
	}
	knowledge->setLastPlayExecuted(OurIndirectPlay);
}

void COurIndirect::execute_0(){

}

void COurIndirect::execute_1(){

}

void COurIndirect::execute_2(){

}

void COurIndirect::execute_3(){
}

void COurIndirect::execute_4(){

}

void COurIndirect::execute_5(){

}

void COurIndirect::execute_6(){

}
