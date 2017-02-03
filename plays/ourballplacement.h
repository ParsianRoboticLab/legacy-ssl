#ifndef OURBALLPLACEMENT_H
#define OURBALLPLACEMENT_H

#include "masterplay.h"

class COurBallPlacement : public CMasterPlay{
public:
        COurBallPlacement();
        ~COurBallPlacement();
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

#endif // OURBALLPLACEMENT_H
