#ifndef OURPENALTY_H
#define OURPENALTY_H

#include "masterplay.h"

class COurPenalty : public CMasterPlay{
public:
	COurPenalty();
	~COurPenalty();
        void penaltyKick();
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

#endif // OURPENALTY_H
