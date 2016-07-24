#include <technicalchallenge.h>

#define _FIRST 1
#define _SECOND 2
#define _THIRD 7
#define _FOURTH 5

void CMainApplication::TechnicalChallenge(){
	static CPassChallengeAtt passChallAtt(soccer);

	static CPassChallengeDef passChallDef(soccer);

	static CKhersChallengeAtt khersChallAtt(soccer);

	static CKhersChallengeDef khersChallDef(soccer);

	if( false ){
		passChallAtt.exectue();
	}
	else if( true ){
		passChallDef.exectue();
	}
	else if( true ){
		khersChallAtt.execute();
	}
	else if( false ){
		khersChallDef.execute();
	}
}

//////////////////////////////////////////////////////////////////////////////


CPassChallengeAtt::CPassChallengeAtt( CSoccer *_soccer ){

	soccer = _soccer;
	QList <Vector2D> a;
	a << Vector2D(wm->field->ourCornerR().x + 0.5 , wm->field->ourCornerR().y - 0.05) << Vector2D(-2.4 , 1.75) << Vector2D(-1.2 , -1.75) << Vector2D(-1.2 , 1.75) << Vector2D(0 , -1.75) \
	  << Vector2D(0 , 1.75) << Vector2D(1.2 , -1.75) << Vector2D(1.2 , 1.75) << wm->field->oppGoal() << Vector2D(-2.4 , -1.95);
	for( int i=0 ; i<10 ; i++ )
		tars[i] = a[i];

	if( false ){
		for( int i=0 ; i<wm->our.activeAgentsCount() && i<4 ; i++ )
			CID[i] = wm->our.active(i)->id;
	}
	else{
		CID[0] = _FIRST;
		CID[1] = _SECOND;
		CID[2] = _THIRD;
		CID[3] = _FOURTH;
	}

	for( int i=0 ; i<4 ; i++ ){
        gp[i] = new CSkillGotoPoint(soccer->agents[CID[i]]);
        kick[i] = new CSkillKick(soccer->agents[CID[i]]);
	}

	rects << wm->field->getRegion("tc2012bottom1") << wm->field->getRegion("tc2012top1") \
		  << wm->field->getRegion("tc2012bottom2") << wm->field->getRegion("tc2012top2") \
		  << wm->field->getRegion("tc2012bottom3") << wm->field->getRegion("tc2012top3") \
		  << wm->field->getRegion("tc2012bottom4") << wm->field->getRegion("tc2012top4");

	resetEverything();
}

CPassChallengeAtt::~CPassChallengeAtt(){
	for( int i=3 ; i>=0 ; i-- ){
		delete kick[i];
		delete gp[i];
	}
}

void CPassChallengeAtt::resetEverything(){
	for( int i=0 ; i<4 ; i++ ){
		lastPos[i] = i;
		moves[i] = true;
	}

	if( false ){
		for( int i=0 ; i<wm->our.activeAgentsCount() && i<4 ; i++ )
			CID[i] = wm->our.active(i)->id;
	}
	else{
		CID[0] = _FIRST;
		CID[1] = _SECOND;
		CID[2] = _THIRD;
		CID[3] = _FOURTH;
	}

    for( int i=0 ; i<4 ; i++ ){
        gp[i]->setAgent(soccer->agents[CID[i]]);
        kick[i]->setAgent(soccer->agents[CID[i]]);
    }

		kickSpeed = 4;
	robotMaxVel = 3;
	mode = 0;
	ballPosHist.clear();
	forceMode = 0;
        timeCantSendPass = false;
}

bool CPassChallengeAtt::canSendPass( int from , int to ){
	Vector2D ballPos , robotPos;
	double t;
        if( timeCantSendPass && timeOut.elapsed() > 5000 )
            return true;
	if( ballPosHist.at(0).dist(ballPosHist.last()) < 0.1 ){
		for( int j=0 ; j<1000 ; j++ ){
			t = 2.0*j/1000.0;
			ballPos = (-0.5)*(wm->ball->getBallAcc() * (tars[to]-tars[from]).norm())*(t*t) + (kickSpeed*(tars[to]-tars[from]).norm())*t + wm->ball->pos;
			if( wm->field->fieldRect().contains(ballPos) ){
				for( int i=0 ; i<wm->opp.activeAgentsCount() ; i++ ){
					robotPos = wm->opp.active(i)->vel*t + wm->opp.active(i)->pos;
					if( ballPos.dist(robotPos) < 0.3 ){
                                            if( timeCantSendPass == false ){
                                                timeCantSendPass = true;
                                                timeOut.start();
                                            }
                                            draw("Cant't Send Pass!!" , Vector2D(-0.2,2.4) , "black");
                                            return false;
					}
				}
			}
		}
                timeCantSendPass = false;
		return true;
	}
	return false;
}

void CPassChallengeAtt::sendPass( int id , int from , int to ){
	Vector2D newPos;
//	if( to == 8 ){
//		newPos = Vector2D(tars[from].x , tars[from].y);
//		static CSkillKickOneTouch *oneTouch = new CSkillKickOneTouch(soccer->agents[CID[id]]);
//		oneTouch->setTarget(tars[to]);
//		oneTouch->setWaitPos(newPos);
//		oneTouch->setKickSpeed(15);
//		oneTouch->execute();
//		draw("ONETOUCH" , Vector2D(-3,2.5) , "black");
//		lastPos[id] = from;
//		gotoPoint[id] = false;
//		return;
//	}
	newPos = Vector2D(tars[to].x , tars[to].y);
	kick[id]->setTarget(newPos);
	kick[id]->setChip(false);
		kick[id]->setKickSpeed(kick[id]->getAgent()->kickSpeedValue(kickSpeed+0.5 , false));
	kick[id]->setSpin(false);
	kick[id]->setSlow(true);
	kick[id]->setTolerance(0.1);
	kick[id]->setThroughMode(false);
	kick[id]->execute();
	lastPos[id] = from;
	gotoPoint[id] = false;
	mode = to;
}

void CPassChallengeAtt::recievePass(int id , int region , bool forceInside ){
	Vector2D pos;
	if( ballPosHist.at(0).dist(ballPosHist.last()) > 0.2 ){
		Line2D ballLine(wm->ball->pos, wm->ball->vel.norm().th());
		if( id % 2 )
			pos = ballLine.intersection(Line2D(Vector2D(-3 , 1.75) , Vector2D(-1.8,1.75)));
		else
			pos = ballLine.intersection(Line2D(Vector2D(-3 , -1.75) , Vector2D(-1.8,-1.75)));
		Vector2D direction = (wm->ball->pos - pos).norm();
		if( forceInside == false || rects.at(region).contains(pos) ){
			if( false && soccer->agents[CID[id]]->pos().dist(pos) < 0.02 || forceReceivePass ){
				forceReceivePass = true;
				double vel , t;
				double t2 , t3;
				double distToCatchPoint = 0.6;
				Vector2D catchPoint = pos + direction*(distToCatchPoint);
				Vector2D robotPos = catchPoint - direction*((CRobot::center_from_kicker_new+CBall::radius)/1.0);
				double D = wm->ball->pos.dist(catchPoint);
				double d = pos.dist(robotPos);
				if( wm->ball->vel.r2() > 2.0*(wm->ball->getBallAcc())*D ){
					vel = sqrt(wm->ball->vel.r2() - 2.0*(wm->ball->getBallAcc())*D);
					t = (wm->ball->vel.r() - vel) / wm->ball->getBallAcc();
					t2 = 0.63;
					t3 = t - t2;
					if( t3 > 0 ){
						gp[id]->init(soccer->agents[CID[id]]->pos() , Vector2D(0,(id%2 ? -1 : 1)));
						gp[id]->setLookAt(wm->ball->pos);
						gp[id]->setMaxVelocity(robotMaxVel);
						lastPos[id] = region;
						draw(QString("%1 Wait Here!!").arg(id) , Vector2D(-3.3	,2.5) , "black");
					}
					else{
						soccer->agents[CID[id]]->setRobotVel(-vel , 0 , 0);
						gotoPoint[id] = false;
						draw("Recieve Pass" , Vector2D(-3,2.5) , "black");
					}
				}
				else{
					gp[id]->init(pos , direction);
					gp[id]->setLookAt(direction + pos);
					gp[id]->setMaxVelocity(robotMaxVel);
					draw(Circle2D(pos , 0.05) , "black" , true);
				}
			}
			else{
				gp[id]->init(pos , direction);
				gp[id]->setLookAt(direction + pos);
				gp[id]->setMaxVelocity(robotMaxVel);
				draw(Circle2D(pos , 0.05) , "black" , true);
			}
		}
		else{
//                    Vector2D newPos = pos;
//                    if( pos.x < rects.at(region).minX() )
//                        newPos.x = rects.at(region).minX();
//                    else
//                        newPos.x = rects.at(region).maxX();
					gp[id]->init(soccer->agents[CID[id]]->pos() , Vector2D(0,(id%2 ? -1 : 1)));
                    gp[id]->setLookAt(wm->ball->pos);
                    gp[id]->setMaxVelocity(robotMaxVel);
                    lastPos[id] = region;
                    draw(QString("%1 Wait Here!!").arg(id) , Vector2D(-3.3	,2.5) , "black");
		}
		lastPos[id] = region;
	}
	else{
		pos = wm->ball->pos + ((wm->ball->pos - tars[region+1]).norm()*0.1);
		if( forceInside == false || rects.at(region).contains(pos) ){
			gp[id]->init(pos , tars[region+1] - wm->ball->pos);
			gp[id]->setLookAt(wm->ball->pos);
			gp[id]->setMaxVelocity(robotMaxVel);
			lastPos[id] = region;
			draw(Circle2D(pos , 0.05) , "black" , true);
		}
		else{
			gp[id]->init(soccer->agents[CID[id]]->pos() , Vector2D(0,(	id%2 ? -1 : 1)));
			gp[id]->setLookAt(wm->ball->pos);
			gp[id]->setMaxVelocity(robotMaxVel);
			lastPos[id] = region;
			draw(QString("%1 Wait Here!!").arg(id) , Vector2D(-3.3	,2.5) , "black");
		}
	}
//	soccer->agents[CID[id]]->setRoller(4);
}

void CPassChallengeAtt::position( int id , int target ){
	Vector2D newPos;
	if( target != 9 && id != target && moves[id] ){
                newPos = Vector2D(tars[target-2].x , sign(tars[target-2].y)*0.3 + tars[target-2].y);
        gp[id]->init(newPos , Vector2D(0,0));
        gp[id]->setLookAt(Vector2D(0,0) + newPos);
		gp[id]->setMaxVelocity(robotMaxVel);
        draw(Circle2D(newPos , 0.05) , "brown" , true);
		lastPos[id] = target;
                if( soccer->agents[CID[id]]->pos().dist(newPos) < 0.15 )
			moves[id] = false;
	}
	else{
		newPos = Vector2D(tars[target].x , tars[target].y);
		gp[id]->init(newPos , Vector2D(0,0));
		gp[id]->setLookAt(Vector2D(0,(id%2 ? -1 : 1)) + newPos);
		gp[id]->setMaxVelocity(robotMaxVel);
		draw(Circle2D(newPos , 0.05) , "brown" , true);
		lastPos[id] = target;
	}
}

void CPassChallengeAtt::exectue(){

	draw(wm->field->getRegion(CField::TC2012Bottom1),"red");
	draw(wm->field->getRegion(CField::TC2012Bottom2),"red");
	draw(wm->field->getRegion(CField::TC2012Bottom3),"red");
	draw(wm->field->getRegion(CField::TC2012Bottom4),"red");
	draw(wm->field->getRegion(CField::TC2012Top1),"red");
	draw(wm->field->getRegion(CField::TC2012Top2),"red");
	draw(wm->field->getRegion(CField::TC2012Top3),"red");
	draw(wm->field->getRegion(CField::TC2012Top4),"red");

	for( int i=0 ; i<4 ; i++ ){
		gp[i]->setFinalDir(Vector2D(0,0));
		gp[i]->setLookAt(Vector2D(0,0));
		gotoPoint[i] = true;
	}

	knowledge->updateGameState();

	if( ballPosHist.length() > 25 )
		ballPosHist.pop_front();
	ballPosHist.push_back(wm->ball->pos);

	if( knowledge->getGameState() == CKnowledge::Stop ){
		resetEverything();
		draw("STOP" , Vector2D(-3,2) , "black");

		if( Circle2D(tars[0] , 0.1).contains(wm->ball->pos) == false )
			position(0,0);
		else
			position(0,9);
		position(1,1);
		position(2,2);
		position(3,3);
	}
	else if( knowledge->getGameState() == CKnowledge::Start){
		if( 0 >= forceMode && (wm->field->getRegion(wm->field->TC2012Bottom1).contains(wm->ball->pos) || mode == 0) ){
			draw("Mode 0" , Vector2D(-3,2) , "black");
			forceMode = 0;

			if( canSendPass(0,1) ){
				sendPass(0,0,1);
			}
			else if( Circle2D(tars[0] , 0.1).contains(wm->ball->pos) == false )
				position(0,0);
			else
				position(0,9);
			recievePass(1,1,true);
			position(2,2);
			position(3,3);
		}
		else if( 1 >= forceMode && (wm->field->getRegion(wm->field->TC2012Top1).contains(wm->ball->pos) || mode == 1) ){
			draw("Mode 1" , Vector2D(-3,2) , "black");
			forceMode = 1;

			position(0,4);
			if( canSendPass(1,2) ){
				sendPass(1,1,2);
			}
			else{
				recievePass(1,1,false);
			}
			recievePass(2,2,true);
			position(3,3);
		}
		else if( 2 >= forceMode && (wm->field->getRegion(wm->field->TC2012Bottom2).contains(wm->ball->pos) || mode == 2) ){
			draw("Mode 2" , Vector2D(-3,2) , "black");
			forceMode = 2;

			position(0,4);
			position(1,5);
			if( canSendPass(2,3) ){
				sendPass(2,2,3);
			}
			else{
				recievePass(2,2,false);
			}
			recievePass(3,3,true);
		}
		else if( 3 >= forceMode && (wm->field->getRegion(wm->field->TC2012Top2).contains(wm->ball->pos) || mode == 3) ){
			draw("Mode 3" , Vector2D(-3,2) , "black");
			forceMode = 3;

			recievePass(0,4,true);
			position(1,5);
			position(2,6);
			if( canSendPass(3,4) )
				sendPass(3,3,4);
			else
				recievePass(3,3,false);
		}
		else if( 4 >= forceMode && (wm->field->getRegion(wm->field->TC2012Bottom3).contains(wm->ball->pos) || mode == 4) ){
			draw("Mode 4" , Vector2D(-3,2) , "black");
			forceMode = 4;

			if( canSendPass(4,5) )
				sendPass(0,4,5);
			else
				recievePass(0,4,false);
			recievePass(1,5,true);
			position(2,6);
			position(3,7);
		}
		else if( 5 >= forceMode && (wm->field->getRegion(wm->field->TC2012Top3).contains(wm->ball->pos) || mode == 5) ){
			draw("Mode 5" , Vector2D(-3,2) , "black");
			forceMode = 5;

			position(0,4);
			if( canSendPass(5,6) )
				sendPass(1,5,6);
			else
				recievePass(1,5,false);
			recievePass(2,6,true);
			position(3,7);
		}
		else if( 6 >= forceMode && (wm->field->getRegion(wm->field->TC2012Bottom4).contains(wm->ball->pos) || mode == 6) ){
			draw("Mode 6" , Vector2D(-3,2) , "black");
			forceMode = 6;

			position(0,4);
			position(1,5);
			if( canSendPass(6,7) )
				sendPass(2,6,7);
			else
				recievePass(2,6,false);
			recievePass(3,7,true);
		}
		else if( 7 >= forceMode && (wm->field->getRegion(wm->field->TC2012Top4).contains(wm->ball->pos) || mode == 7) ){
			draw("Mode 7" , Vector2D(-3,2) , "black");
			forceMode = 7;

			position(0,4);
			position(1,5);
			position(2,6);
			if( canSendPass(7,8) )
				sendPass(3,7,8);
			else{
//				sendPass(3,7,8);
				recievePass(3,7,false);
			}
		}
		else{
			draw("Nothing!!" , Vector2D(-3,2) , "black");
			for( int i=0 ; i<4 ; i++ )
				gotoPoint[i] = false;
		}
	}
        else{
            for( int id=0 ; id<4 ; id++ )
                gotoPoint[id] = false;
        }

	for( int i=0 ; i<4 ; i++ ){
		if( gotoPoint[i] ){
			gp[i]->execute();
			draw(QString("gp[%1]!!").arg(i) , Vector2D(-3,2.4-i*0.1) , "black");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

CPassChallengeDef::CPassChallengeDef( CSoccer *_soccer ){
	soccer = _soccer;
	robotSpeed = 1;
	if( false ){
		for( int i=0 ; i<wm->our.activeAgentsCount() && i<4 ; i++ )
			CID[i] = wm->our.active(i)->id;
	}
	else{
		CID[0] = _FIRST;
		CID[1] = _SECOND;
		CID[2] = _THIRD;
		CID[3] = _FOURTH;
	}

	for( int i=0 ; i<4 ; i++ ){
		gp[i] = new CSkillGotoPoint(soccer->agents[CID[i]]);
		flags[i] = 0;
	}

	QList <Vector2D> a;
	a << Vector2D(2.3 , 0.65) << Vector2D(0 , 0.65) << 	\
		 Vector2D(-0.3 , 0.65) << Vector2D(-2.6 , 0.65) << \
		 Vector2D(0.3 , -0.65) << Vector2D(2.6 , -0.65) << \
		 Vector2D(-2.3 , -0.65) << Vector2D(0 , -0.65);
	for( int i=0 ; i<8 ; i+=2 ){
		tars[i/2][0] = a.at(i);
		tars[i/2][1] = a.at(i+1);
	}

	start = false;
	num = 0;
	cnt = 0;
}

CPassChallengeDef::~CPassChallengeDef(){
	for( int i=3 ; i>=0 ; i-- )
		delete gp[i];
}

void CPassChallengeDef::exectue(){

	for( int i=0 ; i<4 ; i++ ){
		gp[i]->setFinalDir(Vector2D(0,0));
		gp[i]->setLookAt(Vector2D(0,0));
	}

//	if( cnt == 500 )
//		cnt = 0 , start = false , num = 0;

	knowledge->updateGameState();

	if( start && knowledge->getGameState() == CKnowledge::Start ){
		for( int i=0 ; i<4 ; i++ ){
			if( soccer->agents[CID[i]]->pos().dist(tars[i][flags[i]]) < 0.05 )
				flags[i] = 1-flags[i];
			gp[i]->init(tars[i][flags[i]] , Vector2D(0,0));
			gp[i]->setLookAt(Vector2D(3 , tars[i][0].y));
			draw(Circle2D(Vector2D(3 , tars[i][0].y) , 0.05) , "brown" , true);
			gp[i]->setMaxVelocity(robotSpeed);
		}
	}
	else{
		if( false ){
			for( int i=0 ; i<wm->our.activeAgentsCount() && i<4 ; i++ )
				CID[i] = wm->our.active(i)->id;
		}
		else{
			CID[0] = _FIRST;
			CID[1] = _SECOND;
			CID[2] = _THIRD;
			CID[3] = _FOURTH;
		}

		for( int i=0 ; i<4 ; i++ )
			gp[i]->setAgent(soccer->agents[CID[i]]);
		for( int i=0 ; i<4 ; i++ ){
			gp[i]->init(tars[i][0] , Vector2D(0,0));
			gp[i]->setLookAt(Vector2D(3 , tars[i][0].y));
			draw(Circle2D(Vector2D(3 , tars[i][0].y) , 0.05) , "brown" , true);
			gp[i]->setMaxVelocity(robotSpeed);
			if( soccer->agents[CID[i]]->pos().dist(tars[i][0]) < 0.05 )
				num |= 1<<i;
		}
		if( num == 15 )
			start = true;
	}

	for( int i=0 ; i<4 ; i++ ){
		gp[i]->execute();
	}
	cnt++;
}

//////////////////////////////////////////////////////////////////////////////////////////


CKhersChallengeAtt::CKhersChallengeAtt( CSoccer *_soccer ){
	soccer = _soccer;
	for( int i=0 ; i<4 ; i++ ){
        gp[i] = new CSkillGotoPoint(soccer->agents[i]);
	}

	QList <Vector2D> a;
	a << Vector2D(0.5 , 1.5) << Vector2D(2.5 , 1.5) << \
		 Vector2D(0.5 , -1.5) << Vector2D(2.5 , -1.5);
	for( int i=0 ; i<4 ; i++ )
		tars[i] = a.at(i);

	rects << wm->field->getRegion(CField::TC2012Rect1) << wm->field->getRegion(CField::TC2012Rect2) << \
			 wm->field->getRegion(CField::TC2012Rect3) << wm->field->getRegion(CField::TC2012Rect4);
}

CKhersChallengeAtt::~CKhersChallengeAtt(){
}

int CKhersChallengeAtt::determinePassSender(){
    for( int i=0 ; i<4 ; i++ )
        if( rects.at(i).contains(wm->ball->pos) ){
            return i;
        }
}

int CKhersChallengeAtt::determinePassReceiver(){
    bool flag;
    for( int i=0 ; i<4 ; i++ ){
        flag = true;
        for( int j=0 ; j<wm->opp.activeAgentsCount() ; j++ )
            if( Circle2D(soccer->agents[i]->pos() , 0.5).contains(wm->opp.active(j)->pos) ){
                flag = false;
                break;
            }
        if( flag )
            return i;
    }
}

void CKhersChallengeAtt::sendPass( int from , int to ){

}

void CKhersChallengeAtt::execute(){

	draw(wm->field->getRegion(CField::TC2012Rect1),"red");
	draw(wm->field->getRegion(CField::TC2012Rect2),"red");
	draw(wm->field->getRegion(CField::TC2012Rect3),"red");
	draw(wm->field->getRegion(CField::TC2012Rect4),"red");

	for( int i=0 ; i<4 ; i++ ){
		gp[i]->setFinalDir(Vector2D(0,0));
		gp[i]->setLookAt(Vector2D(0,0));
		gotoPoint[i] = true;
	}

	knowledge->updateGameState();

	if( knowledge->getGameState() == CKnowledge::Stop ){
		for( int i=0 ; i<4 ; i++ ){
			if( Circle2D(tars[i] , 0.1).contains(wm->ball->pos) == false )
                gp[i]->init(tars[i] , Vector2D( 0 , (i<2 ? -1 : 1)));
			else
                gp[i]->init(Vector2D(tars[i].x , tars[i].y + sign(tars[i].y)*0.15) , Vector2D( 0 , (i<2 ? -1 : 1)));
		}
	}
	else if( knowledge->getGameState() == CKnowledge::Start ){
		if( wm->ball->vel.length() < 0.25 ){
            passSender = determinePassSender();
			gotoPoint[passSender] = false;
            passReceiver = determinePassReceiver();
            sendPass(passSender , passReceiver);
			for( int i=0 ; i<4 ; i++ ){
				if( i == passSender )
					continue;
                gp[i]->init(tars[i] , Vector2D( 0 , (i<2 ? -1 : 1)));
			}
		}
		else{

		}

	}

	for( int i=0 ; i<4 ; i++ )
		if( gotoPoint[i] )
			gp[i]->execute();
}

///////////////////////////////////////////////////////////////////////////////////////


CKhersChallengeDef::CKhersChallengeDef( CSoccer *_soccer ){
	soccer = _soccer;
}

CKhersChallengeDef::~CKhersChallengeDef(){
}

void CKhersChallengeDef::execute(){

}


