#ifndef EXPERIMENTAL5_H
#define EXPERIMENTAL5_H

#include "mainapplication.h"

class CPassChallenge{
public:
	CPassChallenge(CSoccer *);
	~CPassChallenge();
	void resetEverything();
	bool isBallInsideThisRect(Rect2D);
	bool canSendPass(int , int);
	void sendPass(int , int , int);
	void recievePass(int , int );
	void position(int , int);
	void exectue();

	CSkillGotoPointAvoid* gp[4];
	CSkillKick* kick[4];
	Vector2D tars[10];
	QList <Rect2D> rects;
	int lastPos[4];
	int kickSpeed;
	int mode;
	bool gotoPoint[4];
	bool moves[4];
	CSoccer *soccer;
};

CPassChallenge::CPassChallenge( CSoccer *_soccer ){

	soccer = _soccer;
	QList <Vector2D> a;
	a << Vector2D(-2.4 , -1.75) << Vector2D(-2.4 , 1.75) << Vector2D(-1.2 , -1.75) << Vector2D(-1.2 , 1.75) << Vector2D(0 , -1.75) \
	  << Vector2D(0 , 1.75) << Vector2D(1.2 , -1.75) << Vector2D(1.2 , 1.75) << wm->field->oppGoal() << Vector2D(-2.4 , -1.95);
	for( int i=0 ; i<10 ; i++ )
		tars[i] = a[i];

//	for( int i=0 ; i<4 ; i++ ){
//		gp[i] = new CSkillGotoPointAvoid(soccer->agents[i]);
//		kick[i] = new CSkillKick(soccer->agents[i]);
//	}

	gp[0] = new CSkillGotoPointAvoid(soccer->agents[2]);
	kick[0] = new CSkillKick(soccer->agents[2]);
	gp[1] = new CSkillGotoPointAvoid(soccer->agents[3]);
	kick[1] = new CSkillKick(soccer->agents[3]);
	gp[2] = new CSkillGotoPointAvoid(soccer->agents[4]);
	kick[2] = new CSkillKick(soccer->agents[4]);
	gp[3] = new CSkillGotoPointAvoid(soccer->agents[6]);
	kick[3] = new CSkillKick(soccer->agents[6]);

	rects << wm->field->getRegion("tc2012bottom1") << wm->field->getRegion("tc2012top1") \
		  << wm->field->getRegion("tc2012bottom2") << wm->field->getRegion("tc2012top2") \
		  << wm->field->getRegion("tc2012bottom3") << wm->field->getRegion("tc2012top3") \
		  << wm->field->getRegion("tc2012bottom4") << wm->field->getRegion("tc2012top4");

	resetEverything();
}

CPassChallenge::~CPassChallenge(){
	for( int i=3 ; i>=0 ; i-- ){
		delete kick[i];
		delete gp[i];
	}
}

void CPassChallenge::resetEverything(){
	for( int i=0 ; i<4 ; i++ ){
		lastPos[i] = i;
		moves[i] = true;
	}

	kickSpeed = 8;
	mode = 0;
}

bool CPassChallenge::isBallInsideThisRect( Rect2D r ){
	return wm->ball->pos.x < r.maxX() && wm->ball->pos.x > r.minX() && wm->ball->pos.y < r.maxY() && wm->ball->pos.y > r.minY();
}

bool CPassChallenge::canSendPass( int from , int to ){
	Vector2D ballPos , robotPos;
	double t;
	if( wm->ball->vel.length() < 0.05 && rects.at(from).contains(wm->ball->pos) ){
		for( int j=0 ; j<1000 ; j++ ){
			t = 2.0*j/1000.0;
			ballPos = (-0.5)*(wm->ball->getBallAcc() * (tars[to]-tars[from]).norm())*(t*t) + (kickSpeed*(tars[to]-tars[from]).norm())*t + wm->ball->pos;
			for( int i=0 ; i<wm->opp.activeAgentsCount() ; i++ ){
				robotPos = wm->opp.active(i)->vel*t + wm->opp.active(i)->pos;
				if( ballPos.dist(robotPos) < 0.15 ){
					draw("Cant't Send Pass!!" , Vector2D(-0.2,2.4) , "black");
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

void CPassChallenge::sendPass( int id , int from , int to ){
	kick[id]->setTarget(tars[to]);
	kick[id]->setChip(false);
	kick[id]->setKickSpeed(kickSpeed);
	kick[id]->setSpin(false);
	kick[id]->setSlow(true);
	kick[id]->setTolerance(0.05);
	kick[id]->setThroughMode(false);
	kick[id]->execute();
	lastPos[id] = from;
	gotoPoint[id] = false;
	mode = to;
}

void CPassChallenge::recievePass(int id , int region ){
	Vector2D pos;
	if( wm->ball->vel.length() > 0.2 ){
		Line2D ballLine(wm->ball->pos, wm->ball->vel.norm().th());
		for( int i=0 ; i<8 ; i++ ){
			if( id % 2 )
				pos = ballLine.intersection(Line2D(Vector2D(-3 , 1.8+(-0.1*i)) , Vector2D(-1.8,1.8+(-0.1*i))));
			else
				pos = ballLine.intersection(Line2D(Vector2D(-3 , -1.8+(0.1*i)) , Vector2D(-1.8,-1.8+(0.1*i))));
			if( rects.at(region).contains(pos) )
				break;
		}
		if( rects.at(region).contains(pos) ){
			Vector2D direction = (wm->ball->pos - pos).norm();
			if( soccer->agents[id]->pos().dist(pos) > 0.03 || true ){
				gp[id]->setTarget(pos , direction);
				gp[id]->setMaxVelocity(1);
				draw(Circle2D(pos , 0.05) , "black" , true);
			}
			else{
				double distToCatchPoint = 0.1;
				Vector2D catchPoint = pos - direction*distToCatchPoint;
				Vector2D robotPos = catchPoint - direction*((CRobot::center_from_kicker_new+CBall::radius)/1.0);
				double D = wm->ball->pos.dist(catchPoint);
				double d = pos.dist(robotPos);
				double vel , t;
				if( wm->ball->vel.r2() > 2.0*(wm->ball->getBallAcc())*D ){
					vel = sqrt(wm->ball->vel.r2() - 2.0*(wm->ball->getBallAcc())*D);
					t = (wm->ball->vel.r() - vel) / wm->ball->getBallAcc();
//                    soccer->agents[id]->setRobotVel(-vel , 0 , 0);
					if( soccer->agents[id]->pos().dist(robotPos) > 0.02 && soccer->agents[id]->pos().dist(wm->ball->pos) > 0.02 )
						soccer->agents[id]->setRobotVel(-1*(vel-2*(vel-d/t)) , 0 , 0);
					else
						soccer->agents[id]->setRobotVel(-1*(soccer->agents[id]->vel().r()*0.99) , 0 , 0);

					gotoPoint[id] = false;
					draw("Recieve Pass" , Vector2D(-3,2.5) , "black");
				}
				else{
					gp[id]->setTarget(soccer->agents[id]->pos() , Vector2D(0,(id%2 ? -1 : 1)));
					gp[id]->setMaxVelocity(1);
					lastPos[id] = region;
					draw(QString("%1 Wait Here!!").arg(id) , Vector2D(-3.3	,2) , "black");
				}
			}
			lastPos[id] = region;
		}
		else{
			gp[id]->setTarget(soccer->agents[id]->pos() , Vector2D(0,(id%2 ? -1 : 1)));
			gp[id]->setMaxVelocity(1);
			lastPos[id] = region;
			draw(QString("%1 Wait Here!!").arg(id) , Vector2D(-3.3	,2) , "black");
		}
	}
	else{
		pos = wm->ball->pos + ((wm->ball->pos - tars[region+1]).norm()*0.1);
		gp[id]->setTarget(pos , tars[region+1] - wm->ball->pos);
		gp[id]->setMaxVelocity(1);
		lastPos[id] = region;
		draw(Circle2D(pos , 0.05) , "black" , true);
	}
}

void CPassChallenge::position( int id , int target ){
	if( id != target && moves[id] ){
		gp[id]->setTarget(Vector2D(tars[target-2].x , sign(tars[target-2].y)*0.6 + tars[target-2].y) , Vector2D(0,0));
		gp[id]->setMaxVelocity(1);
		draw(Circle2D(Vector2D(tars[target-2].x , sign(tars[target-2].y)*0.6 + tars[target-2].y) , 0.05) , "brown" , true);
		lastPos[id] = target;
		if( soccer->agents[id]->pos().dist(Vector2D(tars[target-2].x , sign(tars[target-2].y)*0.6 + tars[target-2].y)) < 0.2 )
			moves[id] = false;
	}
	else{
		gp[id]->setTarget(tars[target] , Vector2D(0,(id%2 ? -1 : 1)));
		gp[id]->setMaxVelocity(1);
		draw(Circle2D(tars[target] , 0.05) , "brown" , true);
		lastPos[id] = target;
	}
}

void CPassChallenge::exectue(){
	for( int i=0 ; i<4 ; i++ ){
		gp[i]->setFinalDir(Vector2D(0,0));
		gp[i]->setLookAt(Vector2D(0,0));
		gotoPoint[i] = true;
	}

	knowledge->updateGameState();

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
	else if( knowledge->getGameState() == CKnowledge::Start ){
		if( isBallInsideThisRect(wm->field->getRegion(wm->field->TC2012Bottom1)) || mode == 0 ){
			draw("Mode 0" , Vector2D(-3,2) , "black");

			if( canSendPass(0,1) ){
				sendPass(0,0,1);
			}
			else if( Circle2D(tars[0] , 0.1).contains(wm->ball->pos) == false )
				position(0,0);
			else
				position(0,10);
			position(1,1);
			position(2,2);
			position(3,3);
		}
		else if( isBallInsideThisRect(wm->field->getRegion(wm->field->TC2012Top1)) || mode == 1 ){
			draw("Mode 1" , Vector2D(-3,2) , "black");

			position(0,4);
			if( canSendPass(1,2) ){
				sendPass(1,1,2);
			}
			else{
				recievePass(1,1);
			}
			position(2,2);
			position(3,3);
		}
		else if( isBallInsideThisRect(wm->field->getRegion(wm->field->TC2012Bottom2)) || mode == 2 ){
			draw("Mode 2" , Vector2D(-3,2) , "black");

			position(0,4);
			position(1,5);
			if( canSendPass(2,3) ){
				sendPass(2,2,3);
			}
			else{
				recievePass(2,2);
			}
			position(3,3);
		}
		else if( isBallInsideThisRect(wm->field->getRegion(wm->field->TC2012Top2)) || mode == 3 ){
			draw("Mode 3" , Vector2D(-3,2) , "black");

			position(0,4);
			position(1,5);
			position(2,6);
			if( canSendPass(3,4) )
				sendPass(3,3,4);
			else
				recievePass(3,3);
		}
		else if( isBallInsideThisRect(wm->field->getRegion(wm->field->TC2012Bottom3)) || mode == 4 ){
			draw("Mode 4" , Vector2D(-3,2) , "black");

			if( canSendPass(4,5) )
				sendPass(0,4,5);
			else
				recievePass(0,4);
			position(1,5);
			position(2,6);
			position(3,7);
		}
		else if( isBallInsideThisRect(wm->field->getRegion(wm->field->TC2012Top3)) || mode == 5 ){
			draw("Mode 5" , Vector2D(-3,2) , "black");

			position(0,4);
			if( canSendPass(5,6) )
				sendPass(1,5,6);
			else
				recievePass(1,5);
			position(2,6);
			position(3,7);
		}
		else if( isBallInsideThisRect(wm->field->getRegion(wm->field->TC2012Bottom4)) || mode == 6 ){
			draw("Mode 6" , Vector2D(-3,2) , "black");

			position(0,4);
			position(1,5);
			if( canSendPass(6,7) )
				sendPass(2,6,7);
			else
				recievePass(2,6);
			position(3,7);
		}
		else if( isBallInsideThisRect(wm->field->getRegion(wm->field->TC2012Top4)) || mode == 7 ){
			draw("Mode 7" , Vector2D(-3,2) , "black");

			position(0,4);
			position(1,5);
			position(2,6);
			if( canSendPass(7,8) )
				sendPass(3,7,8);
			else
				recievePass(3,7);
		}
		else{
			draw("Nothing!!" , Vector2D(-3,2) , "black");
			for( int i=0 ; i<4 ; i++ )
				gotoPoint[i] = false;
		}
	}


	if( gotoPoint[0] ){
		gp[0]->execute();
		draw("gp[0]!!" , Vector2D(-3,2.4) , "black");
	}
	if( gotoPoint[1] ){
		gp[1]->execute();
		draw("gp[1]!!" , Vector2D(-3,2.3) , "black");
	}
	if( gotoPoint[2] ){
		gp[2]->execute();
		draw("gp[2]!!" , Vector2D(-3,2.2) , "black");
	}
	if( gotoPoint[3] ){
		gp[3]->execute();
		draw("gp[3]!!" , Vector2D(-3,2.1) , "black");
	}
}

void CMainApplication::Experimental5()
{

	static CPassChallenge tc(soccer);

	tc.exectue();

//#ifndef GAME_MODE
////    draw(wm->field->getRegion(CField::OppCornerTop), "red");
//    Vector2D t0 , t1 , t2, t3;// = knowledge->findGetOpenPointForAgent(0, wm->field->getRegion(CField::OppCornerTop), 0.5);
//    static CSkillGotoPointAvoid* gp[0] = new CSkillGotoPointAvoid(soccer->agents[2]);
//    static CSkillGotoPointAvoid* gp[1] = new CSkillGotoPointAvoid(soccer->agents[3]);
//    static CSkillGotoPointAvoid* gp[2] = new CSkillGotoPointAvoid(soccer->agents[5]);
//    static CSkillGotoPointAvoid* gp[3] = new CSkillGotoPointAvoid(soccer->agents[4]);
//    static int p = 1;
//    static int cntr = 50;
//    if ( knowledge->getMousePos().x > 0)
//        p = 1;
//    else
//        p = -1;
//    if ( knowledge->joystick->getButton8() && cntr > 50)
//    {
//        p *=-1;
//        cntr = 0;
//    }
////    if ( knowledge->joystick->getButton1() && cntr > 50)
////    {
////        target.assign( 2.5, 1.0);
////    }
////    if ( knowledge->joystick->getButton2() && cntr > 50)
////    {
////        target.assign( 2.5, -1.0);
////    }
////    if ( knowledge->joystick->getButton3() && cntr > 50)
////    {
////        target.assign( -2.5, 1.0);
////    }
////    if ( knowledge->joystick->getButton4() && cntr > 50)
////    {
////        target.assign( -2.5, -1.0);
////    }
//    cntr ++;
//    if ( p == 1 )
//    {
//        t0.assign( 0.6, 0.0);
//        t1.assign( 1.0, 0.0);
//        t2.assign( 1.4, 0.0);
//        t3.assign( 2.0, -0.9);
//    }
//    else
//    {
//        t0.assign(  -1.8, 0.0);
//        t1.assign(  -1.4, 0.0);
//        t2.assign(  -1.0, 0.0);
//        t3.assign( -2.0, -0.9);
//    }
////    gp[0]->setFastW(false);
////    gp[1]->setFastW(false);
////    gp[2]->setFastW(false);
////    gp[3]->setFastW(false);
//    gp[0]->setTarget(t0, Vector2D( 0.0 , 0.0));
//    gp[1]->setTarget(t1, Vector2D( 0.0 , 0.0));
//    gp[2]->setTarget(t2, Vector2D( 0.0 , 0.0));
//    gp[3]->setTarget(t3, Vector2D( 0.0 , 0.0));
//    gp[0]->execute();
//    gp[1]->execute();
//    gp[2]->execute();
//    gp[3]->execute();
//#endif
}

#endif // EXPERIMENTAL5_H
