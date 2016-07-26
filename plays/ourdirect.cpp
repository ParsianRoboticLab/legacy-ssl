#include "plays/ourdirect.h"
#include "roles/playmake.h"
#include "soccer.h"

COurDirect::COurDirect(){
//	lastPlayExeced = &COurDirect::middleCenterAreaOffensive3;
}

COurDirect::~COurDirect(){

}

void COurDirect::reset(){
	position.reset();
	cyclesToExecute = 0;
	playMakeRole.resetPlayMake();
	executedCycles = 0;
	lastPlayExeced = NULL;
	oneToucherID = -1;
	staticPoints.clear();
	for( int i = 0 ; i < MAX_POSITIONERS ; i++ )
	{
		////////some spot for each robot
		staticVec[i].clear();
		///////
		cyclesToWait[i].clear();
		/////////where to look
		facePoints[i].clear();
		////////
		escapeRadius[i].clear();
	}
}

void COurDirect::init(QList<int> _agents , QMap<QString , EditData*> *_editData){
	setAgentsID(_agents);
	setEditData(_editData);
	initMaster();

	if( knowledge->getLastPlayExecuted() != OurDirectPlay ){
		reset();
	}
	knowledge->setLastPlayExecuted(OurDirectPlay);
}

void COurDirect::passOrNot()
{
	int id = getAgent(oneToucherID);
	if( id == -1 )
	{
		return;
	}
//        draw((positionAgents.at(id)->pos()+positionAgents.at(id)->vel()*.3),D_MOHAMMED,"red");
//        debug(QString("shart:%1").arg((positionAgents.at(id)->pos()+positionAgents.at(id)->vel()*.3).dist(pointForPass)<.4),D_MOHAMMED,"red");
        if( (positionAgents.at(id)->pos()+positionAgents.at(id)->vel()*.3).dist(pointForPass) < .4 || executedCycles > MAX_WEIGHT_CYCLES )
		playMakeRole.setNoKick(false);

}

void COurDirect::setPosIntentions()
{
	for( int i = 0 ; i < positionAgents.count() ; i++ )
	{
		positionAgents.at(i)->positionIntent.assign(positionAgents.at(i)->self()->id , knowledge->frameCount , 600);
		positionAgents.at(i)->intention = &positionAgents.at(i)->positionIntent;
	}
}

QList<directPlayFunction> COurDirect::getValidPlays(QList<QPair<directPlayFunction,directPlayCondition> > candidates)
{
	QList<directPlayFunction> validPlays;
	validPlays.clear();
	for( int i = 0 ; i < candidates.size() ; i++ )
	{
		if( runWithSymmetry((this->*candidates[i].second)) )
			validPlays.append(candidates[i].first);
	}
	return validPlays;
}

void COurDirect::newMiddleCenterAreaOffensive3(int symmetry)
{
    draw(QString("newMiddleCenterAreaOffensive3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass = Vector2D(wm->ball->pos.x-.5,-1*symmetry);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution*1.2);

        staticVec[0] << Vector2D(.5,-1.3*symmetry) << pointForPass;
        cyclesToWait[0] << waitBeforeExecution/3 << 100;
        escapeRadius[0] << 0 << 0;
        facePoints[0] << TOOPPGOAL << TOOPPGOAL;

        staticVec[1] << Vector2D(.5,1*symmetry) << Vector2D(1.2,.5*symmetry);
        cyclesToWait[1] << waitBeforeExecution << 200;
        escapeRadius[1] << .5 << .5;
        facePoints[1] << TOOPPGOAL << TOOPPGOAL;

        staticVec[2] << Vector2D(1.5,-.3*symmetry) << Vector2D(2.2,-1*symmetry);
        cyclesToWait[2] << 0 << 200;
        escapeRadius[2] << .5 << .5;
        facePoints[2] << TOOPPGOAL << TOOPPGOAL;

        staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[1]));
        oneToucherID = positionAgents.at(0)->id();
        staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));

        setPosIntentions();
    }
    executedCycles++;

    if( executedCycles == waitBeforeExecution )
    {
            addNearestRobot2Mid(Vector2D(0,0));
            staticPoints.append(holdingPoints(positionAgents.last(),staticVec[2],cyclesToWait[2],escapeRadius[2],facePoints[2]));
    }

    setStaticPoints(staticPoints);

    passOrNot();

    playMakeRole.setChip(!canPassWithoutChip());

    setFormation("OurFreeKick6");

    lastPlayExeced = &COurDirect::newMiddleCenterAreaOffensive3;
}

bool COurDirect::conditionNewMiddleCenterAreaOffensive3(int symmetry)
{
    return true;
}

void COurDirect::theirOneThirdFlankArea2(int symmetry)
{
	draw(QString("theirOneThirdFlankArea2"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D((_FIELD_WIDTH/2),-0.175*symmetry)+Vector2D::polar2vector(drand48()*_GOAL_RAD/2+_GOAL_RAD/2,AngleDeg(symmetry*(drand48()*60)+180));
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(true);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);//should be handled inside playmake

		staticVec[0] << Vector2D(2,symmetry*-.20) << Vector2D(2.2,symmetry*.20) << Vector2D(2,symmetry*-.20) << pointForPass;
		cyclesToWait[0] << 0 << 0 << 0 << 200;
		escapeRadius[0] << 0 << 0 << 0 << 0;
		facePoints[0] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		oneToucherID = positionAgents.at(0)->id();

		setPosIntentions();
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::theirOneThirdFlankArea2;
}

bool COurDirect::conditionTheirOneThirdFlankArea2(int symmetry)
{
	return true;
}

void COurDirect::theirOneThirdCenterArea3(int symmetry)
{
	draw(QString("theirOneThirdCenterArea3"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(wm->ball->pos.x+.2,-.5*symmetry);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(true);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << pointForPass;
		cyclesToWait[0] << waitBeforeExecution;
		escapeRadius[0] << 0;
		facePoints[0] << TOOPPGOAL;

		staticVec[1] << Vector2D(pointForPass.x,-pointForPass.y);
		cyclesToWait[1] << waitBeforeExecution;
		escapeRadius[1] << .5;
		facePoints[1] << TOOPPGOAL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
		oneToucherID = positionAgents.at(0)->id();

		setPosIntentions();
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::theirOneThirdCenterArea3;
}

bool COurDirect::conditionTheirOneThirdCenterArea3(int symmetry)
{
	return true;
}

bool COurDirect::canPassWithoutChip()
{
    Segment2D ballLine(wm->ball->pos,pointForPass);
    bool flag = true;
    for( int i = 0 ; i < wm->opp.activeAgentsCount() ; i++ )
    {
        if( ballLine.dist(wm->opp[i]->pos) < 0.2 ){
            flag = false;
            break;
        }
    }
    if( flag == false || (wm->ball->pos.x > 0 && pointForPass.x < 0) || (wm->ball->pos.x < 0 && pointForPass.x > 0) )
        return false;
    return true;
}

void COurDirect::theirOneThirdFlankAreaSimple3(int symmetry)
{
	draw(QString("theirOneThirdFlankAreaSimple3"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(2,symmetry*-.7);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setNoKick(true);
		playMakeRole.setSpinBool(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << Vector2D(1.6,symmetry*-.3);
		cyclesToWait[0] << 200;
		escapeRadius[0] << .5;
		facePoints[0] << TOOPPGOAL;

		staticVec[1] << Vector2D(1.7,symmetry*-.6) << pointForPass;
		cyclesToWait[1] << waitBeforeExecution << 200;
		escapeRadius[1] << .5 << .5;
		facePoints[1] << TOOPPGOAL << TOOPPGOAL;

		int nearestIdx = getNearestAgent(staticVec[1].first());
		staticPoints.append(holdingPoints(positionAgents.at(nearestIdx),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
		staticPoints.append(holdingPoints(positionAgents.at(1-nearestIdx),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		oneToucherID = positionAgents.at(nearestIdx)->id();
		setPosIntentions();
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

        playMakeRole.setChip(!canPassWithoutChip());

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::theirOneThirdFlankAreaSimple3;
}

bool COurDirect::conditionTheirOneThirdFlankAreaSimple3(int symmetry)
{
	return true;
}

void COurDirect::theirOneThirdFlankAreaNoAdd3(int symmetry)
{
	draw(QString("theirOneThirdFlankAreaNoAdd3"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(2.5,symmetry*-.5);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setNoKick(true);
		playMakeRole.setSpinBool(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << Vector2D(2,symmetry*.20) << Vector2D(1.79,symmetry*.092) << Vector2D(1.8,symmetry*-.203) << Vector2D(1.89,symmetry*-.512) << pointForPass;
		cyclesToWait[0] << 30 << 0 << 0 << 0 << 70;
		escapeRadius[0] << 0 << 0 << 0 << 0 << 0;
		facePoints[0] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL ;

		staticVec[1] << Vector2D(2,symmetry*-.20) << Vector2D(2,0) << Vector2D(2,symmetry*.2);
		cyclesToWait[1] << 40 << 0 << 50;
		escapeRadius[1] << 0 << 0 << 0;
		facePoints[1] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL;

		int nearestIdx = getNearestAgent(staticVec[1].first());
		staticPoints.append(holdingPoints(positionAgents.at(1-nearestIdx),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		staticPoints.append(holdingPoints(positionAgents.at(nearestIdx),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
		oneToucherID = positionAgents.at(1-nearestIdx)->id();

		setPosIntentions();
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

        playMakeRole.setChip(true);

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::theirOneThirdFlankAreaNoAdd3;
}

bool COurDirect::conditionTheirOneThirdFlankAreaNoAdd3(int symmetry)
{
	Rect2D emptyArea(Vector2D(1.7,.5),.3,1);

	for( int i = 0 ; i < wm->opp.activeAgentsCount() ; i++ )
	{
		if( emptyArea.contains(wm->opp[i]->pos) )
			return false;
	}

	return true;
}

void COurDirect::middleFlankAreaDefensive3(int symmetry)
{
	draw(QString("middleFlankAreaDefensive3"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
        {
		pointForPass = Vector2D(2.2,-1.5*symmetry);
                draw(Circle2D(pointForPass,.5),D_MOHAMMED,"cyan");
		waitBeforeExecution = 100;
		cyclesToExecute = 300;
		Circle2D aroundBall(wm->ball->pos,.5);
		Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
		Vector2D intersects[2];
		if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
			onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(false);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);//should be handled inside playmake

		staticVec[0] << Vector2D(2,symmetry*-1) << pointForPass;
                cyclesToWait[0] << waitBeforeExecution << 0;
		escapeRadius[0] << .5 << 0;
		facePoints[0] << TOOPPGOAL << TOOPPGOAL;

		staticVec[1] << onTheBallLine << Vector2D(2.5,symmetry*1.3);
                cyclesToWait[1] << waitBeforeExecution << 0;
		escapeRadius[1] << 0 << .5;
		facePoints[1] << TOOPPGOAL << TOBALL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
			staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));

		oneToucherID = positionAgents.at(0)->id();

                setPosIntentions();
        }

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::middleFlankAreaDefensive3;
}

bool COurDirect::conditionMiddleFlankAreaDefensive3(int symmetry)
{
	return true;
}


void COurDirect::middleFlankAreaDefensive2(int symmetry)
{
	draw(QString("middleFlankAreaDefensive2"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(wm->ball->pos.x-.5,symmetry*-1);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;
		Circle2D aroundBall(wm->ball->pos,.5);
		Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
		Vector2D intersects[2];
		if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
			onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(false);
		playMakeRole.setNoKick(false);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << onTheBallLine << pointForPass;
		cyclesToWait[0] << 20 << 0;
		escapeRadius[0] << 0 << 0;
		facePoints[0] << TOOPPGOAL << TOBALL;
		if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
			staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

		oneToucherID = positionAgents.at(0)->id();

		setPosIntentions();
	}

	executedCycles++;

	passOrNot();

	setStaticPoints(staticPoints);

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::middleFlankAreaDefensive2;
}

bool COurDirect::conditionMiddleFlankAreaDefensive2(int symmetry)
{
	return true;
}

void COurDirect::addNearestRobot2Mid(Vector2D p)
{
	if( knowledge->defenseAgents.count() == 0 )
		return;
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

void COurDirect::middleFlankAreaOffensive2(int symmetry)
{
	draw(QString("middleFlankAreaOffensive2"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(wm->ball->pos.x-0.75,-1*symmetry);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(false);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << Vector2D(2,symmetry*-1.5) << Vector2D(1,symmetry*-1.5);
		cyclesToWait[0] << waitBeforeExecution << 0;
		escapeRadius[0] << .5 << 0;
		facePoints[0] << TOBALL << TOBALL;

		staticVec[1] << pointForPass;
		cyclesToWait[1] << waitBeforeExecution;
		escapeRadius[1] << 0;
		facePoints[1] << TOBALL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

		setPosIntentions();
	}

	executedCycles++;

	if( executedCycles == waitBeforeExecution )
	{
		addNearestRobot2Mid(Vector2D(0,0));					//debug(QString("ddddddd:%1").arg(positionAgents.count()),D_MOHAMMED);
		staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
		oneToucherID = positionAgents.at(1)->id();
	}

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::middleFlankAreaOffensive2;
}

bool COurDirect::conditionMiddleFlankAreaOffensive2(int symmetry)
{
	return true;
}

int COurDirect::getAgent(int id)
{
	for( int i = 0 ; i < positionAgents.count() ; i++ )
	{
		if( positionAgents.at(i)->id() == id )
			return i;
	}
	return -1;
}

void COurDirect::theirOneThirdFlankAreaTwoDummies3(int symmetry)
{
    draw(QString("theirOneThirdFlankAreaTwoDummies"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass = Vector2D(2,symmetry*-1);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToPass(pointForPass);
        playMakeRole.setKickMode(FixedPass);
        playMakeRole.setChip(true);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << Vector2D(1.6,symmetry*.5) << Vector2D(1,symmetry*1.1);
        cyclesToWait[0] << waitBeforeExecution << 300;
        escapeRadius[0] << .5 << .5;
        facePoints[0] << TOBALL << TOOPPGOAL;

        staticVec[1] << Vector2D(1.6,symmetry*-.5) << Vector2D(1,symmetry*.1);
        cyclesToWait[1] << waitBeforeExecution << 300;
        escapeRadius[1] << .5 << .5;
        facePoints[1] << TOBALL << TOOPPGOAL;

        staticVec[2] << pointForPass;
        cyclesToWait[2] << waitBeforeExecution;
        escapeRadius[2] << 0;
        facePoints[2] << TOOPPGOAL;

        int nearest = getNearestAgent(staticVec[0].first());
        staticPoints.append(holdingPoints(positionAgents.at(nearest),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        staticPoints.append(holdingPoints(positionAgents.at(1-nearest),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));

        setPosIntentions();
    }

    executedCycles++;

    if( executedCycles == waitBeforeExecution )
    {
        addNearestRobot2Mid(pointForPass);
        oneToucherID = positionAgents.last()->id();
        staticPoints.append(holdingPoints(positionAgents.at(getAgent(oneToucherID)),staticVec[2],cyclesToWait[2],escapeRadius[2],facePoints[2]));
    }

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &COurDirect::theirOneThirdFlankAreaTwoDummies3;
}

bool COurDirect::conditionTheirOneThirdFlankAreaTwoDummies3(int symmetry)
{
    Circle2D emptyArea(Vector2D(2,symmetry*-1),.3);

    for( int i = 0 ; i < wm->opp.activeAgentsCount() ; i++ )
    {
            if( emptyArea.contains(wm->opp[i]->pos) )
                    return false;
    }

    return true;
}

void COurDirect::theirOneThirdFlankAreaShootThemInTheAss3(int symmetry)
{
    draw(QString("theirOneThirdFlankAreaShootThemInTheAss3"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass = Vector2D(2,1.3);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(false);
        playMakeRole.setKickMode(FixedShoot);
        playMakeRole.setPointToShoot(pointForPass);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << pointForPass;
        cyclesToWait[0] << 300;
        escapeRadius[0] << .5;
        facePoints[0] << TOOPPGOAL;

        staticVec[1] << Vector2D(1.7,1.7);
        cyclesToWait[1] << 300;
        escapeRadius[1] << .5;
        facePoints[1] << TOOPPGOAL;

        int nearest = getNearestAgent(staticVec[0].first());
        staticPoints.append(holdingPoints(positionAgents.at(nearest),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        oneToucherID = positionAgents.last()->id();
        staticPoints.append(holdingPoints(positionAgents.at(1-nearest),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &COurDirect::theirOneThirdFlankAreaShootThemInTheAss3;
}

bool COurDirect::conditionTheirOneThirdFlankAreaShootThemInTheAss3(int symmetry)
{
    vector<Vector2D> vec;
    vec.clear();
    vec.push_back(Vector2D(2.04,-.09*symmetry));
    vec.push_back(Vector2D(2.36,-.84*symmetry));
    vec.push_back(Vector2D(2.02,-1.1*symmetry));
    vec.push_back(Vector2D(1.76,-.73*symmetry));
    vec.push_back(Vector2D(1.76,-.28*symmetry));
    Polygon2D emptyArea(vec);
    for( int i = 0 ; i < wm->opp.activeAgentsCount() ; i++ )
    {
            if( emptyArea.contains(wm->opp[i]->pos) )
                    return false;
    }

    return true;
}

void COurDirect::theirOneThirdFlankAreaShootThemInTheAss2(int symmetry)
{
    draw(QString("theirOneThirdFlankAreaShootThemInTheAss2"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
        pointForPass = Vector2D(2,1.3);
        waitBeforeExecution = 100;
        cyclesToExecute = 300;

        playMakeRole.setSlow(true);
        playMakeRole.setPointToShoot(pointForPass);
        playMakeRole.setKickMode(FixedShoot);
        playMakeRole.setChip(false);
        playMakeRole.setNoKick(true);
        playMakeRole.setCyclesToWait(waitBeforeExecution);

        staticVec[0] << pointForPass;
        cyclesToWait[0] << 300;
        escapeRadius[0] << .5;
        facePoints[0] << TOOPPGOAL;

        int nearest = getNearestAgent(staticVec[0].first());
        staticPoints.append(holdingPoints(positionAgents.at(nearest),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
        oneToucherID = positionAgents.last()->id();

        setPosIntentions();
    }

    executedCycles++;

    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurFreeKick6");

    lastPlayExeced = &COurDirect::theirOneThirdFlankAreaShootThemInTheAss2;
}

bool COurDirect::conditionTheirOneThirdFlankAreaShootThemInTheAss2(int symmetry)
{
    vector<Vector2D> vec;
    vec.clear();
    vec.push_back(Vector2D(2.04,-.09*symmetry));
    vec.push_back(Vector2D(2.36,-.84*symmetry));
    vec.push_back(Vector2D(2.02,-1.1*symmetry));
    vec.push_back(Vector2D(1.76,-.73*symmetry));
    vec.push_back(Vector2D(1.76,-.28*symmetry));
    Polygon2D emptyArea(vec);
    for( int i = 0 ; i < wm->opp.activeAgentsCount() ; i++ )
    {
            if( emptyArea.contains(wm->opp[i]->pos) )
                    return false;
    }

    return true;
}


void COurDirect::middleFlankAreaOffensive3(int symmetry)
{
	draw(QString("middleFlankAreaOffensive3"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(wm->ball->pos.x-0.75,-1*symmetry);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;
		Circle2D aroundBall(wm->ball->pos,.5);
		Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
		Vector2D intersects[2];
		if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
			onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(false);
                playMakeRole.setNoKick(false);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		Vector2D reboundPos1 = ((Vector2D(0,0)-wm->ball->pos)*4+wm->field->oppGoal()-wm->ball->pos)*.3+wm->ball->pos;
		staticVec[0] << reboundPos1 << reboundPos1+Vector2D(1,symmetry*-1);
    cyclesToWait[0] << waitBeforeExecution/2 << 3000;
    escapeRadius[0] << .5 << .5;
		facePoints[0] << TOBALL << TOOPPGOAL;

                Vector2D reboundPos2 = (wm->field->oppGoal()-onTheBallLine)*.5+onTheBallLine+(Vector2D(0,0)-onTheBallLine)*.1+Vector2D(0,symmetry);
		staticVec[1] << onTheBallLine << reboundPos2;
    cyclesToWait[1] << waitBeforeExecution/2 << 3000;
    escapeRadius[1] << 0 << .3;
		facePoints[1] << TOOPPGOAL << TOOPPGOAL;

		staticVec[2] << pointForPass;
		cyclesToWait[2] << 200;
		escapeRadius[2] << 0;
                facePoints[2] << TOOPPGOAL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
		{
			staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
			staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
		} else
			staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

		setPosIntentions();
	}
//	draw(onTheBallLine,D_MOHAMMED,"cyan");
	executedCycles++;

	if( executedCycles == waitBeforeExecution )
	{
		addNearestRobot2Mid(Vector2D(0,0));
		oneToucherID = positionAgents.last()->id();
		staticPoints.append(holdingPoints(positionAgents.at(getAgent(oneToucherID)),staticVec[getAgent(oneToucherID)],cyclesToWait[getAgent(oneToucherID)],escapeRadius[getAgent(oneToucherID)],facePoints[getAgent(oneToucherID)]));
	}

	setStaticPoints(staticPoints);

	passOrNot();

        playMakeRole.setChip(!canPassWithoutChip());

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::middleFlankAreaOffensive3;
}

bool COurDirect::conditionMiddleFlankAreaOffensive3(int symmetry)
{
	return true;
}

void COurDirect::middleCenterAreaDefensive1(int symmetry)
{
	draw(QString("middleCenterAreaDefensive1"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(wm->field->oppGoal());
		playMakeRole.setKickMode(FixedShoot);
		playMakeRole.setChip(false);
		playMakeRole.setNoKick(false);
		playMakeRole.setCyclesToWait(waitBeforeExecution);
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::middleCenterAreaDefensive1;
}

bool COurDirect::conditionMiddleCenterAreaDefensive1(int symmetry)
{
	return true;
}

void COurDirect::middleCenterAreaOffensive2(int symmetry)
{
	middleFlankAreaOffensive2(symmetry);

	lastPlayExeced = &COurDirect::middleCenterAreaOffensive2;
}

bool COurDirect::conditionMiddleCenterAreaOffensive2(int symmetry)
{
	return true;
}

void COurDirect::middleCenterAreaDefensive3(int symmetry)
{
	middleFlankAreaDefensive3(symmetry);

	lastPlayExeced = &COurDirect::middleCenterAreaDefensive3;
}

bool COurDirect::conditionMiddleCenterAreaDefensive3(int symmetry)
{
	return true;
}

int COurDirect::getNearestAgent(Vector2D p)
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

void COurDirect::middleCenterAreaOffensive3(int symmetry)
{
	draw(QString("middleCenterAreaOffensive3"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = whoIsBetterForPass(symmetry);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(false);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << Vector2D(.6,-1.3*symmetry) << Vector2D(2.3,-1.4*symmetry);
		cyclesToWait[0] << waitBeforeExecution/2 << 200;
		escapeRadius[0] << 0 << 0;
		facePoints[0] << TOOPPGOAL << TOOPPGOAL;

		Vector2D reboundPos = Vector2D(wm->ball->pos.x+.8,symmetry*.5);
		staticVec[1] << Vector2D(.6,1.3*symmetry) << reboundPos;
		cyclesToWait[1] << waitBeforeExecution/2 << 200;
		escapeRadius[1] << 0 << 0;
		facePoints[1] << TOOPPGOAL << TOOPPGOAL;

		staticVec[2] << pointForPass;
		cyclesToWait[2] << 200;
		escapeRadius[2] << 0;
		facePoints[2] << TOBALL;

		staticVec[3] << Vector2D(playMakeAgent->pos().x,playMakeAgent->pos().y+symmetry*.8) << Vector2D(playMakeAgent->pos().x+1.2,playMakeAgent->pos().y+symmetry*1);
		cyclesToWait[3] << 0 << 200;
		escapeRadius[3] << 0 << 0;
		facePoints[3] << TOBALL << TOBALL;

		int nearestIdx = getNearestAgent(staticVec[0].first());
		staticPoints.append(holdingPoints(positionAgents.at(nearestIdx),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		staticPoints.append(holdingPoints(positionAgents.at(1-nearestIdx),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
		setPosIntentions();
	}

	executedCycles++;

	if( executedCycles == waitBeforeExecution )
	{
		addNearestRobot2Mid(staticVec[2].first());
		oneToucherID = positionAgents.last()->id();
		addNearestRobot2Mid(staticVec[3].first());
		staticPoints.append(holdingPoints(positionAgents.at(2),staticVec[2],cyclesToWait[2],escapeRadius[2],facePoints[2]));
		staticPoints.append(holdingPoints(positionAgents.at(3),staticVec[3],cyclesToWait[3],escapeRadius[3],facePoints[3]));
	}

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::middleCenterAreaOffensive3;
}

bool COurDirect::conditionMiddleCenterAreaOffensive3(int symmetry)
{
	return true;
}

Vector2D COurDirect::whoIsBetterForPass(int symmetry)
{
	return Vector2D(wm->ball->pos.x-1,symmetry*-1.5);
}

void COurDirect::ourOneThirdAreaDefensive2(int symmetry)
{
	draw(QString("ourOneThirdAreaDefensive2"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(-1.5,symmetry*-1);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;
		Circle2D aroundBall(wm->ball->pos,.5);
		Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
		Vector2D intersects[2];
		if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
			onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(false);
		playMakeRole.setNoKick(false);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << onTheBallLine << pointForPass;
		cyclesToWait[0] << waitBeforeExecution/3 << 0;
		escapeRadius[0] << 0 << 0;
		facePoints[0] << TOOPPGOAL << TOBALL;

		if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
			staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		oneToucherID = positionAgents.last()->id();

		setPosIntentions();
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::ourOneThirdAreaDefensive2;
}

bool COurDirect::conditionOurOneThirdAreaDefensive2(int symmetry)
{
	return true;
}

void COurDirect::ourOneThirdAreaOffensive2(int symmetry)
{
	middleFlankAreaOffensive2(symmetry);
	lastPlayExeced = &COurDirect::middleFlankAreaOffensive2;
}

bool COurDirect::conditionOurOneThirdAreaOffensive2(int symmetry)
{
	return true;
}

void COurDirect::ourOneThirdAreaOffensive3(int symmetry)
{
	draw(QString("ourOneThirdAreaOffensive3"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(wm->ball->pos.x+0.75,-1*symmetry);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;
		Circle2D aroundBall(wm->ball->pos,.5);
		Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
		Vector2D intersects[2];
		if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
			onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(false);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		Vector2D reboundPos1 = ((Vector2D(0,0)-wm->ball->pos)*4+wm->field->oppGoal()-wm->ball->pos)*.3+wm->ball->pos;
		staticVec[0] << reboundPos1 << reboundPos1+Vector2D(1,symmetry*-1);
		cyclesToWait[0] << waitBeforeExecution/2 << 300;
		escapeRadius[0] << .5 << .5;
		facePoints[0] << TOBALL << TOOPPGOAL;

		Vector2D reboundPos2 = (wm->field->oppGoal()-onTheBallLine)*.1+onTheBallLine+(Vector2D(0,0)-onTheBallLine)*.5;
		staticVec[1] << onTheBallLine << reboundPos2;
		cyclesToWait[1] << waitBeforeExecution/2 << 300;
		escapeRadius[1] << 0 << .3;
		facePoints[1] << TOOPPGOAL << TOOPPGOAL;

		staticVec[2] << pointForPass;
		cyclesToWait[2] << 200;
		escapeRadius[2] << 0;
		facePoints[2] << TOOPPGOAL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
		{
			staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
			staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
		} else
			staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

		setPosIntentions();
	}
//	draw(onTheBallLine,D_MOHAMMED,"cyan");
	executedCycles++;

	if( executedCycles == waitBeforeExecution )
	{
		addNearestRobot2Mid(Vector2D(0,0));
		oneToucherID = positionAgents.last()->id();
		staticPoints.append(holdingPoints(positionAgents.at(getAgent(oneToucherID)),staticVec[getAgent(oneToucherID)],cyclesToWait[getAgent(oneToucherID)],escapeRadius[getAgent(oneToucherID)],facePoints[getAgent(oneToucherID)]));
	}

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");
	lastPlayExeced = &COurDirect::ourOneThirdAreaOffensive3;
}

bool COurDirect::conditionOurOneThirdAreaOffensive3(int symmetry)
{
	return true;
}

void COurDirect::middleFlankAreaOnlyTwoAgents(int symmetry)
{
	middleCenterAreaOnlyTwoAgents(symmetry);
	lastPlayExeced = &COurDirect::middleFlankAreaOnlyTwoAgents;
}

bool COurDirect::conditionMiddleFlankAreaOnlyTwoAgents(int symmetry)
{
	return true;
}

void COurDirect::middleCenterAreaOnlyTwoAgents(int symmetry)
{
	draw(QString("middleCenterAreaOnlyTwoAgents"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(wm->ball->pos.x-.4,-1*symmetry);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(true);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << pointForPass;
		cyclesToWait[0] << waitBeforeExecution*2;
		escapeRadius[0] << .3;
		facePoints[0] << TOOPPGOAL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		oneToucherID = positionAgents.last()->id();

		setPosIntentions();
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::middleCenterAreaOnlyTwoAgents;
}

bool COurDirect::conditionMiddleCenterAreaOnlyTwoAgents(int symmetry)
{
	return true;
}

void COurDirect::theirOneThirdCenterArea2(int symmetry)
{
	draw(QString("theirOneThirdCenterArea2"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(wm->ball->pos.x+.2,-.5*symmetry);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(true);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << pointForPass;
		cyclesToWait[0] << waitBeforeExecution;
		escapeRadius[0] << 0;
		facePoints[0] << TOOPPGOAL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		oneToucherID = positionAgents.last()->id();

		setPosIntentions();
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::theirOneThirdCenterArea2;

}

bool COurDirect::conditionTheirOneThirdCenterArea2(int symmetry)
{
	return true;
}

void COurDirect::ourOneThirdAreaDefensive3(int symmetry)
{
	draw(QString("ourOneThirdAreaDefensive3"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(-1,-1.5*symmetry);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;
		Circle2D aroundBall(wm->ball->pos,.5);
		Segment2D ballLine2Goal(wm->ball->pos,wm->field->oppGoal());
		Vector2D intersects[2];
		if( aroundBall.intersection(ballLine2Goal,&intersects[0],&intersects[1]) > 0  )
			onTheBallLine = intersects[0].isValid() ? intersects[0] : intersects[1];

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(false);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << Vector2D(1,symmetry*-1) << pointForPass;
                cyclesToWait[0] << waitBeforeExecution/2 << 0;
		escapeRadius[0] << .5 << 0;
		facePoints[0] << TOBALL << TOBALL;

		staticVec[1] << onTheBallLine << Vector2D(2.5,symmetry*1.3);
		cyclesToWait[1] << waitBeforeExecution << 0;
		escapeRadius[1] << 0 << .5;
		facePoints[1] << TOOPPGOAL << TOBALL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		if( !canScore() || knowledge->getGameState() == CKnowledge::OurIndirectKick )
			staticPoints.append(holdingPoints(positionAgents.at(1),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));

		oneToucherID = positionAgents.first()->id();

		setPosIntentions();
        }

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

        lastPlayExeced = &COurDirect::ourOneThirdAreaDefensive3;
}

bool COurDirect::conditionOurOneThirdAreaDefensive3(int symmetry)
{
	return true;
}

void COurDirect::swapNearestDefAndMid(Vector2D p, int midIdx)
{
	if( knowledge->defenseAgents.count() == 0 )
		return;
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

void COurDirect::theirOneThirdFlankAreaAddDefense3(int symmetry)
{
	draw(QString("theirOneThirdFlankAreaAddDefense3"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(2.5,symmetry*-.5);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setNoKick(true);
		playMakeRole.setSpinBool(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << Vector2D(2,symmetry*.20) << Vector2D(1.79,symmetry*.092) << Vector2D(1.8,symmetry*-.203) << Vector2D(1.89,symmetry*-.512) << pointForPass;
		cyclesToWait[0] << 30 << 0 << 0 << 0 << 70;
		escapeRadius[0] << 0 << 0 << 0 << 0 << 0;
		facePoints[0] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL ;

		staticVec[1] << Vector2D(2,symmetry*-.20) << Vector2D(2,0) << Vector2D(2,symmetry*.2);
		cyclesToWait[1] << 40 << 0 << 50;
		escapeRadius[1] << 0 << 0 << 0;
		facePoints[1] << TOOPPGOAL << TOOPPGOAL << TOOPPGOAL;

		staticVec[2] << Vector2D(.3,symmetry*-1);
		cyclesToWait[2] << 200;
		escapeRadius[2] << .5;
		facePoints[2] << TOBALL;

		int nearestIdx = getNearestAgent(staticVec[1].first());debug(QString("id:%1").arg(positionAgents.at(nearestIdx)->id()),D_MOHAMMED);
		staticPoints.append(holdingPoints(positionAgents.at(1-nearestIdx),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		staticPoints.append(holdingPoints(positionAgents.at(nearestIdx),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
		oneToucherID = positionAgents.at(1-nearestIdx)->id();

		setPosIntentions();
	}

//	foreach( Vector2D v , staticVec[0] )
//		draw(v,D_MOHAMMED,"cyan");
	if( executedCycles == waitBeforeExecution  )
	{
		addNearestRobot2Mid(staticVec[2].first());
		staticPoints.append(holdingPoints(positionAgents.at(2),staticVec[2],cyclesToWait[2],escapeRadius[2],facePoints[2]));
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

        playMakeRole.setChip(!canPassWithoutChip());

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::theirOneThirdFlankAreaAddDefense3;
}

bool COurDirect::conditionTheirOneThirdFlankAreaAddDefense3(int symmetry)
{
	Rect2D emptyArea(Vector2D(1.7,.5),.3,1);

	for( int i = 0 ; i < wm->opp.activeAgentsCount() ; i++ )
	{
		if( emptyArea.contains(wm->opp[i]->pos) )
			return false;
	}

	return true;
}

void COurDirect::middleCenterAreaDefensive2(int symmetry)
{
	draw(QString("middleCenterAreaDefensive2"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(wm->ball->pos.x-.3,-1.5*symmetry);
		waitBeforeExecution = 100;
		cyclesToExecute = 300;

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << Vector2D(-0.1,symmetry*1.7);
		cyclesToWait[0] << 0;
		escapeRadius[0] << 0;
		facePoints[0] << TOBALL;

		staticVec[1] << pointForPass;
		cyclesToWait[1] << 200;
		escapeRadius[1] << 0;
		facePoints[1] << TOBALL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));

		setPosIntentions();
	}

	executedCycles++;

	if( executedCycles == waitBeforeExecution && knowledge->defenseAgents.count() > 0 )
	{
		staticPoints.clear();
		swapNearestDefAndMid(pointForPass,getAgent(positionAgents.first()->id()));
		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
		oneToucherID = positionAgents.at(0)->id();
	}
	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurKickOff");

	lastPlayExeced = &COurDirect::middleCenterAreaDefensive2;
}

bool COurDirect::conditionfalsePlay(int symmetry)
{
    return true;
}

void COurDirect::falsePlay(int symmetry)
{
    draw(QString("false"),Vector2D(-3,-2),"red");

    choosePlayMaker();

    appendRemainingsAgents(positionAgents);

    if( executedCycles == 0 )
    {
            pointForPass = Vector2D(0.5,symmetry*0);
            waitBeforeExecution = 100;
            cyclesToExecute = 300;

            playMakeRole.setSlow(true);
            playMakeRole.setPointToPass(pointForPass);
            playMakeRole.setKickMode(FixedPass);
            playMakeRole.setNoKick(true);
            playMakeRole.setCyclesToWait(waitBeforeExecution);

            staticVec[0] << pointForPass;
            cyclesToWait[0] << 0;
            escapeRadius[0] << 0;
            facePoints[0] << TOBALL;

//            staticVec[1] << ;
//            cyclesToWait[1] << 200;
//            escapeRadius[1] << 0;
//            facePoints[1] << TOBALL;

            staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
            oneToucherID = positionAgents.at(0)->id();
            setPosIntentions();
    }

    executedCycles++;

//    if( executedCycles == waitBeforeExecution && knowledge->defenseAgents.count() > 0 )
//    {
//            staticPoints.clear();
//            swapNearestDefAndMid(pointForPass,getAgent(positionAgents.first()->id()));
//            staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[1],cyclesToWait[1],escapeRadius[1],facePoints[1]));
//            oneToucherID = positionAgents.at(0)->id();
//    }
    setStaticPoints(staticPoints);

    passOrNot();

    setFormation("OurKickOff");

    lastPlayExeced = &COurDirect::falsePlay;
}

bool COurDirect::conditionMiddleCenterAreaDefensive2(int symmetry)
{
	return true;
}

void COurDirect::ourOneThirdAreaOnlyTwoAgents(int symmetry)
{
	draw(QString("ourOneThirdAreaOnlyTwoAgents"),Vector2D(-3,-2),"red");

	choosePlayMaker();

	appendRemainingsAgents(positionAgents);

	if( executedCycles == 0 )
	{
		pointForPass = Vector2D(wm->ball->pos.x+.4,-1*symmetry);
		waitBeforeExecution = 100;

		playMakeRole.setSlow(true);
		playMakeRole.setPointToPass(pointForPass);
		playMakeRole.setKickMode(FixedPass);
		playMakeRole.setChip(true);
		playMakeRole.setNoKick(true);
		playMakeRole.setCyclesToWait(waitBeforeExecution);

		staticVec[0] << pointForPass;
		cyclesToWait[0] << waitBeforeExecution*2;
		escapeRadius[0] << .3;
		facePoints[0] << TOOPPGOAL;

		staticPoints.append(holdingPoints(positionAgents.at(0),staticVec[0],cyclesToWait[0],escapeRadius[0],facePoints[0]));
		oneToucherID = positionAgents.last()->id();

		setPosIntentions();
	}

	executedCycles++;

	setStaticPoints(staticPoints);

	passOrNot();

	setFormation("OurFreeKick6");

	lastPlayExeced = &COurDirect::ourOneThirdAreaOnlyTwoAgents;
}

bool COurDirect::conditionOurOneThirdAreaOnlyTwoAgents(int symmetry)
{
	return true;
}

void COurDirect::execute_0(){

}

void COurDirect::execute_1(){

}

void COurDirect::execute_2(){
	if( cyclesToExecute > 0 )
	{
		runWithSymmetry((this->*lastPlayExeced));
		cyclesToExecute--;
	}
	else {
		if( wm->ball->pos.x > _FIELD_WIDTH/6 )
		{
			if( fabs(wm->ball->pos.y) > 1.5 ){
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {
					defPlaysCan.clear();
                                        defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			} else if( fabs(wm->ball->pos.y) < 1 ){
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea2,&COurDirect::conditionTheirOneThirdCenterArea2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea2,&COurDirect::conditionTheirOneThirdCenterArea2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea2,&COurDirect::conditionTheirOneThirdCenterArea2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea2,&COurDirect::conditionTheirOneThirdCenterArea2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea2,&COurDirect::conditionTheirOneThirdCenterArea2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea2,&COurDirect::conditionTheirOneThirdCenterArea2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			} else {
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankArea2,&COurDirect::conditionTheirOneThirdFlankArea2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			}
		} else if( wm->ball->pos.x > _FIELD_WIDTH*-1/6 )
		{
			if( fabs(wm->ball->pos.y) > 1.5 ){
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::middleFlankAreaOnlyTwoAgents,&COurDirect::conditionMiddleFlankAreaOnlyTwoAgents);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::middleFlankAreaOnlyTwoAgents,&COurDirect::conditionMiddleFlankAreaOnlyTwoAgents);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive2,&COurDirect::conditionMiddleFlankAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::middleFlankAreaOffensive2,&COurDirect::conditionMiddleFlankAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {//offensive plays with 2 defense
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive2,&COurDirect::conditionMiddleFlankAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::middleFlankAreaOffensive2,&COurDirect::conditionMiddleFlankAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			} else if( fabs(wm->ball->pos.y) < 1 ){
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::middleCenterAreaOnlyTwoAgents,&COurDirect::conditionMiddleCenterAreaOnlyTwoAgents);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::middleCenterAreaOnlyTwoAgents,&COurDirect::conditionMiddleCenterAreaOnlyTwoAgents);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive2,&COurDirect::conditionMiddleCenterAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::middleCenterAreaOffensive2,&COurDirect::conditionMiddleCenterAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {//offensive plays with 2 defense
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive2,&COurDirect::conditionMiddleCenterAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::middleCenterAreaOffensive2,&COurDirect::conditionMiddleCenterAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			} else {
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::middleCenterAreaOnlyTwoAgents,&COurDirect::conditionMiddleCenterAreaOnlyTwoAgents);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::middleCenterAreaOnlyTwoAgents,&COurDirect::conditionMiddleCenterAreaOnlyTwoAgents);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive2,&COurDirect::conditionMiddleFlankAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::middleFlankAreaOffensive2,&COurDirect::conditionMiddleFlankAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {//offensive plays with 2 defense
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive2,&COurDirect::conditionMiddleFlankAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::middleFlankAreaOffensive2,&COurDirect::conditionMiddleFlankAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			}
		} else
		{
			if( fabs(wm->ball->pos.y) > 1.5 ){
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOnlyTwoAgents,&COurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOnlyTwoAgents,&COurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive2,&COurDirect::conditionOurOneThirdAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive2,&COurDirect::conditionOurOneThirdAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {//offensive plays with 2 defense
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive2,&COurDirect::conditionOurOneThirdAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive2,&COurDirect::conditionOurOneThirdAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			} else if( fabs(wm->ball->pos.y) < 1 ){
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOnlyTwoAgents,&COurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOnlyTwoAgents,&COurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive2,&COurDirect::conditionOurOneThirdAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive2,&COurDirect::conditionOurOneThirdAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {//offensive plays with 2 defense]
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive2,&COurDirect::conditionOurOneThirdAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive2,&COurDirect::conditionOurOneThirdAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			} else {
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOnlyTwoAgents,&COurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOnlyTwoAgents,&COurDirect::conditionOurOneThirdAreaOnlyTwoAgents);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive2,&COurDirect::conditionOurOneThirdAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive2,&COurDirect::conditionOurOneThirdAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {//offensive plays with 2 defense
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive2,&COurDirect::conditionOurOneThirdAreaDefensive2);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive2,&COurDirect::conditionOurOneThirdAreaOffensive2);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			}
		}
	}
}

void COurDirect::execute_3(){
        if( cyclesToExecute > 0 )
	{
		runWithSymmetry((this->*lastPlayExeced));
		cyclesToExecute--;
	}
	else {
		if( wm->ball->pos.x > _FIELD_WIDTH/6 )
		{//check whether f collides with execute_2 function
			if( fabs(wm->ball->pos.y) > 1.5 ){
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
                                        defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&COurDirect::theirOneThirdFlankAreaSimple3,&COurDirect::conditionTheirOneThirdFlankAreaSimple3);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&COurDirect::theirOneThirdFlankAreaSimple3,&COurDirect::conditionTheirOneThirdFlankAreaSimple3);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&COurDirect::theirOneThirdFlankAreaSimple3,&COurDirect::conditionTheirOneThirdFlankAreaSimple3);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
                                        offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaTwoDummies3,&COurDirect::conditionTheirOneThirdFlankAreaTwoDummies3) << qMakePair(&COurDirect::theirOneThirdFlankAreaAddDefense3,&COurDirect::conditionTheirOneThirdFlankAreaAddDefense3) << qMakePair(&COurDirect::theirOneThirdFlankAreaSimple3,&COurDirect::conditionTheirOneThirdFlankAreaSimple3);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&COurDirect::theirOneThirdFlankAreaSimple3,&COurDirect::conditionTheirOneThirdFlankAreaSimple3);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
                                        offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaTwoDummies3,&COurDirect::conditionTheirOneThirdFlankAreaTwoDummies3) << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3) << qMakePair(&COurDirect::theirOneThirdFlankAreaAddDefense3,&COurDirect::conditionTheirOneThirdFlankAreaAddDefense3) << qMakePair(&COurDirect::theirOneThirdFlankAreaSimple3,&COurDirect::conditionTheirOneThirdFlankAreaSimple3);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			} else if( fabs(wm->ball->pos.y) < 1 ){
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
                                        defPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea3,&COurDirect::conditionTheirOneThirdCenterArea3);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
                                        offPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea3,&COurDirect::conditionTheirOneThirdCenterArea3);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
                                        defPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea3,&COurDirect::conditionTheirOneThirdCenterArea3);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
                                        offPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea3,&COurDirect::conditionTheirOneThirdCenterArea3);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {
					defPlaysCan.clear();
                                        defPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea3,&COurDirect::conditionTheirOneThirdCenterArea3);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
                                        offPlaysCan << qMakePair(&COurDirect::theirOneThirdCenterArea3,&COurDirect::conditionTheirOneThirdCenterArea3);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			} else {
				if( knowledge->defenseAgents.count() == 0 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else if( knowledge->defenseAgents.count() == 1 ) {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				} else {
					defPlaysCan.clear();
					defPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
					defPlays = getValidPlays(defPlaysCan);
					offPlaysCan.clear();
					offPlaysCan << qMakePair(&COurDirect::theirOneThirdFlankAreaNoAdd3,&COurDirect::conditionTheirOneThirdFlankAreaNoAdd3);
					offPlays = getValidPlays(offPlaysCan);
					decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
				}
			}
		} else if( wm->ball->pos.x > _FIELD_WIDTH*-1/6 )
		{
			if( fabs(wm->ball->pos.y) > 1.5 )
			{
				if( lastPlayExeced == &COurDirect::middleFlankAreaOffensive2 || lastPlayExeced == &COurDirect::middleCenterAreaOffensive2 )
					runWithSymmetry((this->*lastPlayExeced));
				else {
					if( knowledge->defenseAgents.count() == 0 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive3,&COurDirect::conditionMiddleFlankAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive3,&COurDirect::conditionMiddleFlankAreaDefensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else if( knowledge->defenseAgents.count() == 1 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive3,&COurDirect::conditionMiddleFlankAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::middleFlankAreaOffensive3,&COurDirect::conditionMiddleFlankAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else {//plays with 2 defenses
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive3,&COurDirect::conditionMiddleFlankAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::middleFlankAreaOffensive3,&COurDirect::conditionMiddleFlankAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					}
				}
			}
			else if( fabs(wm->ball->pos.y) < 1 )
			{
				if( lastPlayExeced == &COurDirect::middleFlankAreaOffensive2 || lastPlayExeced == &COurDirect::middleCenterAreaOffensive2 )
					runWithSymmetry((this->*lastPlayExeced));
				else {
					if( knowledge->defenseAgents.count() == 0 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive3,&COurDirect::conditionMiddleCenterAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive3,&COurDirect::conditionMiddleCenterAreaDefensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else if( knowledge->defenseAgents.count() == 1 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive3,&COurDirect::conditionMiddleCenterAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
                                                offPlaysCan << qMakePair(&COurDirect::newMiddleCenterAreaOffensive3,&COurDirect::conditionNewMiddleCenterAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else {//plays with 2 defenses
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive3,&COurDirect::conditionMiddleCenterAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
                                                offPlaysCan << qMakePair(&COurDirect::middleCenterAreaOffensive3,&COurDirect::conditionMiddleCenterAreaOffensive3) << qMakePair(&COurDirect::newMiddleCenterAreaOffensive3,&COurDirect::conditionNewMiddleCenterAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					}
				}
			}
			else{//this is hysteresis
				if( lastPlayExeced == &COurDirect::middleFlankAreaOffensive3 || lastPlayExeced == &COurDirect::middleCenterAreaOffensive3 ){
					runWithSymmetry((this->*lastPlayExeced));
				}
				else{//this is for the first time
//					if( fabs(wm->ball->pos.y) > 1.25 ) {
						if( knowledge->defenseAgents.count() == 0 ) {
							defPlaysCan.clear();
							defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive3,&COurDirect::conditionMiddleFlankAreaDefensive3);
							defPlays = getValidPlays(defPlaysCan);
							offPlaysCan.clear();
							offPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive3,&COurDirect::conditionMiddleFlankAreaDefensive3);
							offPlays = getValidPlays(offPlaysCan);
							decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
						} else if( knowledge->defenseAgents.count() == 1 ) {
							defPlaysCan.clear();
							defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive3,&COurDirect::conditionMiddleFlankAreaDefensive3);
							defPlays = getValidPlays(defPlaysCan);
							offPlaysCan.clear();
							offPlaysCan << qMakePair(&COurDirect::middleFlankAreaOffensive3,&COurDirect::conditionMiddleFlankAreaOffensive3);
							offPlays = getValidPlays(offPlaysCan);
							decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
						} else {//plays with 2 defenses
							defPlaysCan.clear();
							defPlaysCan << qMakePair(&COurDirect::middleFlankAreaDefensive3,&COurDirect::conditionMiddleFlankAreaDefensive3);
							defPlays = getValidPlays(defPlaysCan);
							offPlaysCan.clear();
							offPlaysCan << qMakePair(&COurDirect::middleFlankAreaOffensive3,&COurDirect::conditionMiddleFlankAreaOffensive3);
							offPlays = getValidPlays(offPlaysCan);
							decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
						}
//					}
//					else {
//						if( knowledge->defenseAgents.count() == 0 ) {
//							defPlaysCan.clear();
//							defPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive3,&COurDirect::conditionMiddleCenterAreaDefensive3);
//							defPlays = getValidPlays(defPlaysCan);
//							offPlaysCan.clear();
//							offPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive3,&COurDirect::conditionMiddleCenterAreaDefensive3);
//							offPlays = getValidPlays(offPlaysCan);
//							decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
//						} else if( knowledge->defenseAgents.count() == 1 ) {
//							defPlaysCan.clear();
//							defPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive3,&COurDirect::conditionMiddleCenterAreaDefensive3);
//							defPlays = getValidPlays(defPlaysCan);
//							offPlaysCan.clear();
//							offPlaysCan << qMakePair(&COurDirect::middleCenterAreaOffensive3,&COurDirect::conditionMiddleCenterAreaOffensive3);
//							offPlays = getValidPlays(offPlaysCan);
//							decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
//						} else {//plays with 2 defenses
//							defPlaysCan.clear();
//							defPlaysCan << qMakePair(&COurDirect::middleCenterAreaDefensive3,&COurDirect::conditionMiddleCenterAreaDefensive3);
//							defPlays = getValidPlays(defPlaysCan);
//							offPlaysCan.clear();
//							offPlaysCan << qMakePair(&COurDirect::middleCenterAreaOffensive3,&COurDirect::conditionMiddleCenterAreaOffensive3);
//							offPlays = getValidPlays(offPlaysCan);
//							decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
//						}
//					}
				}
			}
		} else {//check whether f collides with execute_2 function
                    runWithSymmetry(falsePlay);
                    return;
			if( fabs(wm->ball->pos.y) > 1.5 ){
				if( lastPlayExeced == &COurDirect::middleFlankAreaOffensive2 )
					runWithSymmetry((this->*lastPlayExeced));
				else {
					if( knowledge->defenseAgents.count() == 0 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else if( knowledge->defenseAgents.count() == 1 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive3,&COurDirect::conditionOurOneThirdAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else {//plays with 2 defenses
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive3,&COurDirect::conditionOurOneThirdAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					}
				}
			} else if( fabs(wm->ball->pos.y) < 1 ){
				if( lastPlayExeced == &COurDirect::middleFlankAreaOffensive2 )
					runWithSymmetry((this->*lastPlayExeced));
				else {
					if( knowledge->defenseAgents.count() == 0 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else if( knowledge->defenseAgents.count() == 1 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive3,&COurDirect::conditionOurOneThirdAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive3,&COurDirect::conditionOurOneThirdAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					}
				}
			} else {
				if( lastPlayExeced == &COurDirect::middleFlankAreaOffensive2 )
					runWithSymmetry((this->*lastPlayExeced));
				else {
					if( knowledge->defenseAgents.count() == 0 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else if( knowledge->defenseAgents.count() == 1 ) {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive3,&COurDirect::conditionOurOneThirdAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					} else {
						defPlaysCan.clear();
						defPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaDefensive3,&COurDirect::conditionOurOneThirdAreaDefensive3);
						defPlays = getValidPlays(defPlaysCan);
						offPlaysCan.clear();
						offPlaysCan << qMakePair(&COurDirect::ourOneThirdAreaOffensive3,&COurDirect::conditionOurOneThirdAreaOffensive3);
						offPlays = getValidPlays(offPlaysCan);
						decideDefOrOff(defPlays,defPlays.count(),offPlays,offPlays.count())
					}
				}
			}
		}
	}
}

void COurDirect::execute_4(){
	if( cyclesToExecute > 0 )
	{
		runWithSymmetry((this->*lastPlayExeced));
		cyclesToExecute--;
	}
	else {
		if( wm->ball->pos.x > _FIELD_WIDTH/6 )
		{

		} else if( wm->ball->pos.x > _FIELD_WIDTH*-1/6 )
		{
			if( lastPlayExeced )
				runWithSymmetry((this->*lastPlayExeced));
		} else {
			if( lastPlayExeced )
				runWithSymmetry((this->*lastPlayExeced));
		}
	}
}

void COurDirect::execute_5(){
	if( cyclesToExecute > 0 )
	{
		runWithSymmetry((this->*lastPlayExeced));
		cyclesToExecute--;
	}
	else {
		if( wm->ball->pos.x > _FIELD_WIDTH/6 )
		{
		} else if( wm->ball->pos.x > _FIELD_WIDTH*-1/6 )
		{
			if( lastPlayExeced )
				runWithSymmetry((this->*lastPlayExeced));
		} else {
		}
	}
}

void COurDirect::execute_6(){

}
