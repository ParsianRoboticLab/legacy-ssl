#include "plays/ourkickoff.h"

COurKickOff::COurKickOff(){
}

COurKickOff::~COurKickOff(){
}

void COurKickOff::reset(){
        resetPositioning();
        executedCycles = 0;
        resetPlayMaker();
        oneToucherID = -1;
        staticPoints.clear();
        for( int i = 0 ; i < MAX_POSITIONERS ; i++ )
        {
                staticVec[i].clear();
                cyclesToWait[i].clear();
                escapeRadius[i].clear();
                facePoints[i].clear();
        }
}

void COurKickOff::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
        setAgentsID(_agents);
        setEditData(_editData);
        initMaster();

        if( knowledge->getLastPlayExecuted() != OurKickOffPlay ){
                reset();
        }

        knowledge->setLastPlayExecuted(OurKickOffPlay);
}

void COurKickOff::passOrNot()
{
        int id = getAgent(oneToucherID);
        if( id == -1 )
        {
                return;
        }
        if( (positionAgents.at(id)->pos()+positionAgents.at(id)->vel()*.3).dist(pointForPass) < .2 || executedCycles > MAX_WEIGHT_CYCLES )
                playMakeRole.setNoKick(false);
}

int COurKickOff::getAgent(int id)
{
        for( int i = 0 ; i < positionAgents.count() ; i++ )
        {
                if( positionAgents.at(i)->id() == id )
                        return i;
        }
        return -1;
}

int COurKickOff::getNearestAgent(Vector2D p)
{
        double dist = 1e6;
        int idx = -1;
        for( int i = 0 ; i < positionAgents.count() ; i++ )
        {
                if( dist > positionAgents.at(i)->pos().dist(p) ){
                        dist = positionAgents.at(i)->pos().dist(p);
                        idx = i;
                }
        }
        return idx;
}

void COurKickOff::setPosIntentions()
{
        for( int i = 0 ; i < positionAgents.count() ; i++ )
        {
                positionAgents.at(i)->positionIntent.assign(positionAgents.at(i)->self()->id , knowledge->frameCount , 600);
                positionAgents.at(i)->intention = &positionAgents.at(i)->positionIntent;
        }
}

void COurKickOff::addNearestRobot2Mid(Vector2D p)
{
        QList<int> ourDefenses;
        for( int i = 0 ; i < knowledge->defenseAgents.count() ; i++ )
                ourDefenses.append(knowledge->defenseAgents.at(i)->id());
        int nearestDefense = knowledge->getNearestAgentToPoint(p,&ourDefenses);

        CAgent* defFront = knowledge->defenseAgents.at(nearestDefense);

        knowledge->defenseAgents.removeAt(nearestDefense);

        defFront->positionIntent.assign(defFront->id(),knowledge->frameCount,600);
        defFront->intention = &defFront->positionIntent;

        positionAgents.push_back(defFront);
}

void COurKickOff::swapNearestDefAndMid(Vector2D p, int midIdx)
{
        QList<int> ourDefenses;
        for( int i = 0 ; i < knowledge->defenseAgents.count() ; i++ )
                ourDefenses.append(knowledge->defenseAgents.at(i)->id());
        int nearestDefense = knowledge->getNearestAgentToPoint(p,&ourDefenses);

        CAgent* defFront = knowledge->defenseAgents.at(nearestDefense);
        CAgent* posFront = positionAgents.at(midIdx);

        positionAgents.removeAt(midIdx);
        knowledge->defenseAgents.removeAt(nearestDefense);

        defFront->positionIntent.assign(defFront->id(),knowledge->frameCount);
        defFront->intention = &defFront->positionIntent;

        posFront->defIntent.assign(posFront->id(),knowledge->frameCount);
        posFront->intention = &posFront->defIntent;

        positionAgents.push_front(defFront);
        knowledge->defenseAgents.push_front(posFront);
}

void COurKickOff::chipToOppGoal(){

        executedCycles++;

        choosePlayMaker();

        appendRemainingsAgents(positionAgents);

        playMakeRole.setKickoffmode(true);
        playMakeRole.setSlow(true);
        playMakeRole.setChipToOppGoal(true);

        setFormation("OurKickOff");
}

void COurKickOff::kickoffOnlyTwo2(int symmetry)
{
        draw(QString("kickoffOnlyTwo2"),Vector2D(-3,-2),"red");

        choosePlayMaker();

        appendRemainingsAgents(positionAgents);

        if( executedCycles == 0 )
        {
                pointForPass = Vector2D(0,symmetry*1.2);
                waitBeforeExecution = 100;

                playMakeRole.setSlow(true);
                playMakeRole.setPointToPass(pointForPass);
                playMakeRole.setKickMode(FixedPass);
                playMakeRole.setNoKick(true);
                playMakeRole.setKickoffmode(true);
                playMakeRole.setCyclesToWait(waitBeforeExecution);

                staticVec[0] << pointForPass;
                cyclesToWait[0] << 100;
                escapeRadius[0] << .3;
                facePoints[0] << TOOPPGOAL;

                oneToucherID = positionAgents.last()->id();
        }

        executedCycles++;

        if( executedCycles == waitBeforeExecution )
        {
                staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        }

        setStaticPoints(staticPoints);

        passOrNot();

        setFormation("OurKickOff");

        lastPlayExeced = &COurKickOff::kickoffOnlyTwo2;
}

void COurKickOff::swapKickoff3(int symmetry)
{
        draw(QString("swapKickoff3"),Vector2D(-3,-2),"red");

        choosePlayMaker();

        appendRemainingsAgents(positionAgents);

        if( executedCycles == 0 )
        {
                Vector2D deceit1(0,symmetry*1.2);
                Vector2D deceit2(0,symmetry*1.9);
                pointForPass = Vector2D(0,symmetry*-1.6);
                waitBeforeExecution = 100;

                playMakeRole.setSlow(true);
                playMakeRole.setPointToPass(pointForPass);
                playMakeRole.setKickMode(FixedPass);
                playMakeRole.setNoKick(true);
                playMakeRole.setKickoffmode(true);
                playMakeRole.setCyclesToWait(waitBeforeExecution);

                staticVec[0] << deceit1;
                cyclesToWait[0] << 1000;
                escapeRadius[0] << .3;
                facePoints[0] << TOBALL;

                staticVec[1] << deceit2;
                cyclesToWait[1] << 1000;
                escapeRadius[1] << 0.3;
                facePoints[1] << TOOPPGOAL;

                staticVec[2] << pointForPass;
                cyclesToWait[2] << 1000;
                escapeRadius[2] << 0;
                facePoints[2] << TOOPPGOAL;

                staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
                staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));

                setPosIntentions();
        }

        executedCycles++;

        if( oneToucherID == -1 && executedCycles >= waitBeforeExecution && knowledge->getGameMode() == CKnowledge::OurKickOff )
        {                                       debug(QString("fuck"),D_MOHAMMED);
                staticPoints.clear();
                swapNearestDefAndMid(pointForPass,getNearestAgent(staticVec[0].first()));
                staticPoints.append(holdingPoints(positionAgents.at(getNearestAgent(staticVec[1].first())),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
                staticPoints.append(holdingPoints(positionAgents.at(getNearestAgent(Vector2D(-2.5,0))),staticVec[2],cyclesToWait[2],escapeRadius[2],facePoints[2]));
                oneToucherID = positionAgents.last()->id();
        }

        setStaticPoints(staticPoints);

        passOrNot();

        setFormation("OurKickOff");

        lastPlayExeced = &COurKickOff::swapKickoff3;
}

void COurKickOff::swapKickoff2(int symmetry)
{
        draw(QString("swapKickoff2"),Vector2D(-3,-2),"red");

        choosePlayMaker();

        appendRemainingsAgents(positionAgents);

        if( executedCycles == 0 )
        {
                Vector2D deceit(0,symmetry*1.5);
                pointForPass = Vector2D(0,symmetry*-1.5);
                waitBeforeExecution = 100;

                playMakeRole.setSlow(true);
                playMakeRole.setPointToPass(pointForPass);
                playMakeRole.setKickMode(FixedPass);
                playMakeRole.setNoKick(true);
                playMakeRole.setKickoffmode(true);
                playMakeRole.setCyclesToWait(waitBeforeExecution);

                staticVec[0] << deceit;
                cyclesToWait[0] << 50;
                escapeRadius[0] << .3;
                facePoints[0] << TOOPPGOAL;

                staticVec[1] << pointForPass;
                cyclesToWait[1] << 100;
                escapeRadius[1] << 0.3;
                facePoints[1] << TOOPPGOAL;

                staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

                setPosIntentions();
        }

        executedCycles++;

        if( executedCycles == waitBeforeExecution )
        {
                staticPoints.clear();
                swapNearestDefAndMid(pointForPass,getAgent(positionAgents.first()->id()));
                staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
                oneToucherID = positionAgents.at(0)->id();
        }
        setStaticPoints(staticPoints);

        passOrNot();

        setFormation("OurKickOff");

        lastPlayExeced = &COurKickOff::swapKickoff2;
}

void COurKickOff::execute_0(){

}

void COurKickOff::execute_1(){
        if( false )//some crappy conditions when we want to defend
                chipToOppGoal();
        else
                swapKickoff2(1);
}

void COurKickOff::execute_2(){
        if( knowledge->defenseAgents.count() == 0 ) {
                kickoffPlayFunction defPlays[] = {&COurKickOff::kickoffOnlyTwo2};
                kickoffPlayFunction offPlays[] = {&COurKickOff::kickoffOnlyTwo2};
                decideDefOrOff(defPlays,1,offPlays,1)
        } else if( knowledge->defenseAgents.count() == 1 ) {
                kickoffPlayFunction defPlays[] = {&COurKickOff::swapKickoff2};
                kickoffPlayFunction offPlays[] = {&COurKickOff::swapKickoff2};
                decideDefOrOff(defPlays,1,offPlays,1)
        } else {
                kickoffPlayFunction defPlays[] = {&COurKickOff::swapKickoff2};
                kickoffPlayFunction offPlays[] = {&COurKickOff::swapKickoff2};
                decideDefOrOff(defPlays,1,offPlays,1)
        }


}

void COurKickOff::execute_3(){
        if( knowledge->defenseAgents.count() == 0 ) {
                //doesn't occur
        } else if( knowledge->defenseAgents.count() == 1 ) {
                if( lastPlayExeced == &COurKickOff::swapKickoff2 )
                        runWithSymmetry((this->*lastPlayExeced));
                else {
                        kickoffPlayFunction defPlays[] = {&COurKickOff::swapKickoff3};
                        kickoffPlayFunction offPlays[] = {&COurKickOff::swapKickoff3};
                        decideDefOrOff(defPlays,1,offPlays,1)
                }
        } else {
                if( lastPlayExeced == &COurKickOff::swapKickoff2 )
                        runWithSymmetry((this->*lastPlayExeced));
                else {
                        kickoffPlayFunction defPlays[] = {&COurKickOff::swapKickoff3};
                        kickoffPlayFunction offPlays[] = {&COurKickOff::swapKickoff3};
                        decideDefOrOff(defPlays,1,offPlays,1)
                }
        }
}

void COurKickOff::execute_4(){
        if( knowledge->defenseAgents.count() == 0 ) {
                if( lastPlayExeced == &COurKickOff::swapKickoff3 )
                        runWithSymmetry((this->*lastPlayExeced));
        } else if( knowledge->defenseAgents.count() == 1 ) {
                if( lastPlayExeced == &COurKickOff::swapKickoff3 )
                        runWithSymmetry((this->*lastPlayExeced));
        } else {
                if( lastPlayExeced == &COurKickOff::swapKickoff3 )
                        runWithSymmetry((this->*lastPlayExeced));
        }
}

void COurKickOff::execute_5(){
}

void COurKickOff::execute_6(){

}
