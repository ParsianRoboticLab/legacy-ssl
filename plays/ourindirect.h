#ifndef OURINDIRECT_H
#define OURINDIRECT_H

#include "masterplay.h"

class COurIndirect : public CMasterPlay{
public:
	COurIndirect();
	~COurIndirect();
	void execute_0();
	void execute_1();
	void execute_2();
	void execute_3();
	void execute_4();
	void execute_5();
	void execute_6();
	void init(QList <int> _agents , QMap<QString , EditData*> *_editData);
private:
	void reset();
};

#endif // OURINDIRECT_H
