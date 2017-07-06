#ifndef CHALFTIMELINEUP_H
#define CHALFTIMELINEUP_H

#include "masterplay.h"

class CHalftimeLineup : public CMasterPlay{
public:
    CHalftimeLineup();
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
    void lineUpAllAgents();
    QList<CSkillGotoPointAvoid *> lineup;
    CSkillGotoPointAvoid * lineupAgent;
    QList<CAgent *> agents;
    QList<int> matchpoints;
    QList<Vector2D> points;
};

#endif // CHALFTIMELINEUP_H
