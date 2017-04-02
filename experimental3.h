#ifndef EXPERIMENTAL3_H
#define EXPERIMENTAL3_H
#include "mainapplication.h"
#include <geom/voronoi_diagram_original.h>
#include <gamelogger.h>

//#define NAVIGATION_CHALLENGE











// -*-c++-*-

/*!
  \file edit_data.h
  \brief formation editor data class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifndef FEDIT2_EDIT_DATA_H
#define FEDIT2_EDIT_DATA_H

#include <QString>

#include <formation/sample_data.h>
#include <formation/formation.h>
//#include <rcsc/geom/cdt/triangulation.h>
#include <geom/triangulation.h>
#include <geom/vector_2d.h>

#include <boost/shared_ptr.hpp>

#include <vector>
#include <utility>
#include <iostream>


class EditData {
public:

	static const double MAX_X;
	static const double MAX_Y;

	enum SelectType {
		SELECT_BALL,
		SELECT_PLAYER,
		SELECT_SAMPLE,
		NO_SELECT,
	};

private:

	QString M_filepath;
	bool M_conf_changed;

	rcsc::formation::SampleData M_state; //!< current state on the edit canvas.

	rcsc::Formation::Ptr M_formation;
	rcsc::formation::SampleDataSet::Ptr M_samples; //!< training data
	rcsc::Triangulation M_triangulation;

	rcsc::Formation::Ptr M_background_formation;
	rcsc::Triangulation M_background_triangulation;

	int M_current_index;

	SelectType M_select_type;
	size_t M_select_index;

	size_t M_constraint_origin_index;
	size_t M_constraint_terminal_index;
	rcsc::Vector2D M_constraint_terminal;

	// not used
	EditData( const EditData & );
	EditData & operator=( const EditData & );
public:

	EditData();
	~EditData();

	void createFormation( const QString & type_name );
private:

	void init();
	void backup( const QString & filepath );

public:

	const
	QString & filePath() const
	  {
		  return M_filepath;
	  }

	bool isConfChanged() const
	  {
		  return M_conf_changed;
	  }

	const
	rcsc::formation::SampleData & state() const
	  {
		  return M_state;
	  }

	rcsc::Formation::ConstPtr formation() const
	  {
		  return M_formation;
	  }
	rcsc::formation::SampleDataSet::Ptr samples() const
	  {
		  return M_samples;
	  }
	const
	rcsc::Triangulation & triangulation() const
	  {
		  return M_triangulation;
	  }

	rcsc::Formation::ConstPtr backgroundFormation() const
	  {
		  return M_background_formation;
	  }
	const
	rcsc::Triangulation & backgroundTriangulation() const
	  {
		  return M_background_triangulation;
	  }

	int currentIndex() const
	  {
		  return M_current_index;
	  }

	SelectType selectType() const
	  {
		  return M_select_type;
	  }
	size_t selectIndex() const
	  {
		  return M_select_index;
	  }

	size_t constraintOriginIndex() const
	  {
		  return M_constraint_origin_index;
	  }

	size_t constraintTerminalIndex() const
	  {
		  return M_constraint_terminal_index;
	  }

	const
	rcsc::Vector2D & constraintTerminal() const
	  {
		  return M_constraint_terminal;
	  }

	bool openConf( const QString & filepath );
	bool saveConf();
	bool saveConfAs( const QString & filepath );

	bool openData( const QString & filepath );

	bool openBackgroundConf( const QString & filepath );

private:
	void updatePlayerPosition();
	void updateTriangulation();

public:

	void updateRoleData( const int unum,
						 const int symmetry_unum,
						 const std::string & role_name );

	void moveBallTo( const double & x,
					 const double & y );
	void movePlayerTo( const int unum,
					   const double & x,
					   const double & y );
	void setConstraintTerminal( const double & x,
								const double & y );
	void setConstraintIndex( const int origin_idx,
							 const int terminal_idx );
	bool moveSelectObjectTo( const double & x,
							 const double & y );

	bool selectObject( const double & x,
					   const double & y );
	bool releaseObject();

	rcsc::formation::SampleDataSet::ErrorType addData();
	rcsc::formation::SampleDataSet::ErrorType insertData( const int idx );
	rcsc::formation::SampleDataSet::ErrorType replaceData( const int idx );
	rcsc::formation::SampleDataSet::ErrorType replaceBall( const int idx,
														   const double & x,
														   const double & y );
	rcsc::formation::SampleDataSet::ErrorType replacePlayer( const int idx,
															 const int unum,
															 const double & x,
															 const double & y );
	rcsc::formation::SampleDataSet::ErrorType deleteData( const int idx );
	rcsc::formation::SampleDataSet::ErrorType changeDataIndex( const int old_idx,
															   const int new_idx );

	rcsc::formation::SampleDataSet::ErrorType addConstraint( const int origin_idx,
															 const int terminal_idx );
	rcsc::formation::SampleDataSet::ErrorType replaceConstraint( const int idx,
																 const int origin_idx,
																 const int terminal_idx );
	rcsc::formation::SampleDataSet::ErrorType deleteConstraint( const int origin_idx,
																const int terminal_idx );


	bool setCurrentIndex( const int idx );
	void reverseY();
	void train();
};

#endif




// -*-c++-*-

/*!
  \file feditor_data.cpp
  \brief formation editor data class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QDateTime>
#include <QFile>

#include "formation/edit_data.h"


void CMainApplication::Experimental3()
{
//    long i;
//    debug("mahmood",D_MAHMOOD);
//    debug("MHMMD",D_MHMMD);
//    debug("atousa",D_ATOUSA);
//    debug("fatemeh",D_FATEMEH);
//    debug("mahi",D_MAHI);
//    debug("amin",D_AMIN);
//    debug("amiR",D_AMIR);
//    debug("AHZ",D_AHZ);
//    debug("hamed",D_HAMED);
//    knowledge->profiler->load(JSON);
//    QVector< QVector<double> > *KickCoeff = new QVector< QVector<double> >();

//    CPolynomialRegression ProRes;

//    for(int q=0; q<16; q++)
//        KickCoeff->append(ProRes.PolynomialRegression(knowledge->profiler->robotsProfile[q].finalKickMap.values() , knowledge->profiler->robotsProfile[q].finalKickMap.keys(),2));

//    //    ProfilerResult[robotID][0:kick , 1:chip , 2:SpinKick , 3:SpinChip][10*distance(0-80)] ---> contains needed voltage for this distance

//    for(int q=0; q<16; q++)
//        for(double dis=0; dis<=8; dis+=0.1){
//            if(KickCoeff->at(q).count()>0)
//                knowledge->ProfilerResult[q][0][(int)(dis*10)] =(double)
//                        KickCoeff->at(q).at(0)+KickCoeff->at(q).at(1)*dis+KickCoeff->at(q).at(2)*dis*dis;
//        }

    analyze("hello",13,false);
    analyze("nadia",1,false);
    analyze("nadia",3,false);
    analyze("hello",15,false);
    analyze("nadia",4,false);
    analyze("hello",3,false);
    analyze("robot",10,false);
return;
//    Segment2D segToOppGoali[5];
//    Vector2D tempMousePos = tPoint;
//    int tCnt = 0;
//    int intersectCnt = 0;
//    for(double y = wm->field->oppGoalR().y; y <= wm->field->oppGoalL().y; y += _GOAL_WIDTH/4.01)
//    {
//        segToOppGoali[tCnt].assign(tempMousePos, Vector2D(wm->field->oppGoalR().x, y));
//        if(kkCheckIntersectWithAgents(segToOppGoali[tCnt]))
//            intersectCnt++;
//        draw(segToOppGoali[tCnt], QColor(Qt::green));
//        tCnt++;
//    }
//    if(intersectCnt < 5)
//        debug("OK",D_KK);
//    else
//        debug("NOTOK",D_KK);







//    return;
    CSkillGotoPointAvoid *GPA = new CSkillGotoPointAvoid(soccer->agents[4]);
    Vector2D pos;
    Vector2D ang;
    ang = -soccer->agents[4]->pos() + wm->ball->pos;
    pos = wm->ball->pos + (-wm->ball->pos + soccer->agents[4]->pos()).norm()*0.12;

//        if(wm->ball->pos.dist(soccer->agents[4]->pos()) < 0.11) {
//            GPA->init(pos,ang);
//            GPA->execute();
//        }
//        else if(wm->ball->pos.dist(soccer->agents[4]->pos()) > 0.13) {
//            GPA->init(pos,ang);
//            GPA->execute();
//        }
//        else {
    soccer->agents[4]->setRobotVel(0,-.2,(ang.th().radian() - soccer->agents[4]->dir().th().radian())*5);
//        }
    return;
    static CskillNewGotoPoint abc(soccer->agents[4]);
    abc.init(knowledge->getMousePos(),Vector2D(0,0));
    abc.execute();;
    return;
////////////////////////one touch skill by don Shirazi
  static Vector2D place(0,0),finalPlace(0,0);

  AngleDeg finalDir=0;
  bool exeFlag = false;
  finalDir = (wm->field->ourGoal() - place).th();
  debug(QString("dir: %1").arg(finalDir.degree()),D_HOSSEIN);
  draw(wm->field->ourGoal(),D_HOSSEIN,"red");

  CskillNewGotoPoint *gpoint = new CskillNewGotoPoint(soccer->agents[6]);

  gpoint->execute();
  debug(QString("ball speed :%1").arg(wm->ball->vel.length()),D_HOSSEIN);

  Circle2D canMove;
  Circle2D stayReg;

  canMove.assign(soccer->agents[6]->pos(), 0.5);
  Segment2D ballPath;
  ballPath.assign(wm->ball->pos,wm->ball->pos+(5*wm->ball->vel.norm()));
  draw(ballPath,"red");


  Vector2D *_pos1 = new Vector2D(0,0);
  Vector2D *_pos2 = new Vector2D(0,0);


  if(soccer->agents[6]->pos().dist(place) <= 0.5 && exeFlag == false)
  {
    exeFlag = true;
  }


  if(soccer->agents[6]->pos().dist(wm->ball->pos) < 0.3)
    soccer->agents[6]->setKick(1000);
    else
    soccer->agents[6]->setKick(0);

  if(exeFlag == true)
  {
    if(wm->ball->vel.length()>1)
    {
       if(canMove.intersection(ballPath,_pos1,_pos2) > 1)
       {
         finalPlace = ballPath.nearestPoint(place)- Vector2D(0.09*cos(finalDir.radian()), 0.09*sin(finalDir.radian()));

       }
    }
  }
  else
  {
    place = knowledge->getMousePos();
    finalPlace = place;
  }

  draw(place,D_HOSSEIN,"blue");


///////////////////////////////////////////////////////////////////

















return;
	//Technical Challenge Navigation for Robocup 2013

    int rob0 = 2;
    int rob1 = 3;
    int rob2 = 5;

	static CSkillGotoPointAvoid* gp0 = new CSkillGotoPointAvoid( soccer->agents[rob0]);
	static CSkillGotoPointAvoid* gp1 = new CSkillGotoPointAvoid( soccer->agents[rob1]);
	static CSkillGotoPointAvoid* gp2 = new CSkillGotoPointAvoid( soccer->agents[rob2]);

	Vector2D pos0;
	Vector2D pos1;
	Vector2D pos2;

	static bool flag0 = false;
	static bool flag1 = false;
	static bool flag2 = false;

	static bool wait0 = true;
	static bool wait1 = false;
	static bool wait2 = true;

	static int mode = 0;

	knowledge->updateGameState();
    gp0->setAvoidPenaltyArea(false);
    gp1->setAvoidPenaltyArea(false);
    gp2->setAvoidPenaltyArea(false);

	if( knowledge->getGameState() == CKnowledge::Start)
		mode = 1;
	else if( knowledge->getGameState() == CKnowledge::Stop)
    mode = 0;
	else if( knowledge->getGameState() == CKnowledge::Halt)
		mode = -1;

//	debug(QString("mode : %1").arg(mode),D_SEPEHR);
	if( mode == -1)
	{
		haltAllRobots();
		return;
	}
	else if ( mode == 0)
	{
        pos0 = Vector2D(wm->field->ourCornerL().x+0.05, wm->field->ourCornerL().y-0.05);
		pos1 = wm->field->ourPenalty();
        pos2 = Vector2D(wm->field->ourCornerR().x+0.05, wm->field->ourCornerR().y+0.05);
		gp0->setTarget(pos0,Vector2D(0,0));
		gp1->setTarget(pos1,Vector2D(0,0));
		gp2->setTarget(pos2,Vector2D(0,0));
		flag0 = false;
		flag1 = false;
		flag2 = false;
		wait0 = true;
		wait2 = true;
	}
	else
	{
		if( !flag0)
		{
            if( soccer->agents[rob0]->pos().dist(Vector2D(0,0)) < 0.15)
				flag0 = true;
			if( wait0)
                gp0->setTarget( wm->field->ourCornerL().norm()*1.2,Vector2D(0,0));
			else
				gp0->setTarget( Vector2D(0,0),Vector2D(0,0));
		}
		else
		{
			wait2 = false;
            gp0->setTarget( Vector2D(wm->field->oppCornerR().x - 0.05, wm->field->oppCornerR().y + 0.05),Vector2D(0,0));
		}

		if( !flag1)
		{
            if ( soccer->agents[rob1]->pos().dist(Vector2D(0,0)) < 0.15)
				flag1 = true;
			if( wait1)
				gp1->setTarget( soccer->agents[rob1]->pos(),Vector2D(0,0));
			else
				gp1->setTarget( Vector2D(0,0),Vector2D(0,0));
		}
		else
		{
			wait0 = false;
			gp1->setTarget( wm->field->oppPenalty(),Vector2D(0,0));
		}

		if( !flag2)
		{
            if( soccer->agents[rob2]->pos().dist(Vector2D(0,0)) < 0.15)
				flag2 = true;
			if( wait2)
                gp2->setTarget( wm->field->ourCornerR().norm()*1.2,Vector2D(0,0));
			else
				gp2->setTarget( Vector2D(0,0),Vector2D(0,0));
		}
		else
		{
            gp2->setTarget( Vector2D( wm->field->oppCornerL().x - 0.05, wm->field->oppCornerL().y - 0.05),Vector2D(0,0));
		}

	}

	gp0->setAgent(soccer->agents[rob0]);
	gp1->setAgent(soccer->agents[rob1]);
	gp2->setAgent(soccer->agents[rob2]);

	gp0->setAvoidPenaltyArea(false);
	gp1->setAvoidPenaltyArea(false);
	gp2->setAvoidPenaltyArea(false);

	gp0->execute();
	gp1->execute();
	gp2->execute();

	return;




//	return;

	soccer->agents[1]->setRobotVel(0,0,90);
	//    static CSkillTurn* turn = new CSkillTurn(soccer->agents[5]);
//    turn->setAgent(soccer->agents[5]);
//    turn->setTurnMode(CSkillTurn::Fast);
//    turn->setDirection(soccer->agents[5]->dir().rotatedVector(170));
//    turn->execute();
     return;

    Vector2D kickTar;
    double kickW;
    QList<int> ourRelId,oppRelId;
    oppRelId.clear();
    ourRelId.clear();
    kickTar = knowledge->getEmptyPosOnGoal( wm->ball->pos, kickW, true, ourRelId, oppRelId, 0.9);

    qDebug() << "awf";
    debug(QString("hola : %1").arg(kickW),D_SEPEHR);
return;


	//positioning test
//        Position p;
//        p.rects.append(wm->field->getRegion("oppfield"));
//	p.ready = true;
//	p.isInited = true;
//	p.initialPoint = mousePos;
//	p.mode = Position::Offense;
//	Vector2D p0 = mousePos;
//	double d=0, passProb=0;

	return;
	//positioningThread->nearestBestPos(wm, p);
	//draw(p.point, 1, QColor(0,0,255));


#ifndef GAME_MODE
    //    draw(wm->field->getRegion("field1stquarter"), "pink");
    //    static Vector2D lastMousePos;
    //    QList<int> empty;
    //    double w;
    //    knowledge->getEmptyPosOnGoal(mousePos, w, true, empty, empty, 1);
    //    debug(QString("goal %1").arg(w), D_EXPERIMENT);
    //    static bool move = false;
    //    static Vector2D lastMousePos = mousePos;
    //    if (lastMousePos!=mousePos)
    //    {
    //        bool ss = (wm->field->isInOurPArea(mousePos));
    //        if (ss) debug("in", D_EXPERIMENT);
    //        else debug("out", D_EXPERIMENT);
    //        move = !move;
    //    }
    //    if (move)
    //    {
    //        soccer->agents[4]->setRobotVel(0.5,0.0,0);
    //        debug(QString("vel=%1").arg(soccer->agents[4]->vel().length()), D_ALI);
    //    }
        //    lastMousePos = mousePos;

#ifdef NAVIGATION_CHALLENGE // navigation :

    static CSkillGotoPointAvoid* gp0 = new CSkillGotoPointAvoid(soccer->agents[0]);
    static CSkillGotoPointAvoid* gp1 = new CSkillGotoPointAvoid(soccer->agents[1]);
    static CSkillGotoPointAvoid* gp2 = new CSkillGotoPointAvoid(soccer->agents[2]);
    static CSkillGotoPointAvoid* gp3 = new CSkillGotoPointAvoid(soccer->agents[3]);
    static CSkillGotoPointAvoid* gp4 = new CSkillGotoPointAvoid(soccer->agents[4]);

    static int tn0=0;
    static int tn1=0;
    static int tn2=0;
    static int tn3=0;
    static int tn4=0;
    static int tn0M=2;
    static int tn1M=2;
    static int tn2M=2;
    static int tn3M=2;
    static int tn4M=6;

    soccer->agents[0]->setRoller(2);
    soccer->agents[1]->setRoller(2);
    soccer->agents[2]->setRoller(2);
    soccer->agents[3]->setRoller(2);
    soccer->agents[4]->setRoller(2);


	static Vector2D tar0[2]={Vector2D( 1.000,-2.025),Vector2D( 1.000, 2.025)};
	static Vector2D tar1[2]={Vector2D( 0.500, 2.025),Vector2D( 0.500,-2.025)};
	static Vector2D tar2[2]={Vector2D(-0.500,-2.025),Vector2D(-0.500, 2.025)};
	static Vector2D tar3[2]={Vector2D(-1.000, 2.025),Vector2D(-1.000,-2.025)};
    static Vector2D tar4[6]={Vector2D( 2.5, 0.5),Vector2D(-2.5, 0.5),Vector2D(-2.7,0),Vector2D(-2.5,-0.5),Vector2D(2.5,-0.5),Vector2D(2.7,0)};

    static int cnt0 = 0;
    static int cnt1 = 0;
    static int cnt2 = 0;
    static int cnt3 = 0;
    static int cnt4 = 0;

//    qDebug()<<"new";


    if ( soccer->agents[0]->pos().dist( tar0[tn0]) < 0.05 && cnt0 > 5)
    {
        cnt0 = 0;
        tn0++;
        tn0%=tn0M;
    }
    if ( soccer->agents[1]->pos().dist( tar1[tn1]) < 0.05 && cnt1 > 5)
    {
        cnt1 = 0;
        tn1++;
        tn1%=tn1M;
    }
    if ( soccer->agents[2]->pos().dist( tar2[tn2]) < 0.05 && cnt2 > 5)
    {
        cnt2 = 0;
        tn2++;
        tn2%=tn2M;
    }
    if ( soccer->agents[3]->pos().dist( tar3[tn3]) < 0.05 && cnt3 > 5)
    {
        cnt3 = 0;
        tn3++;
        tn3%=tn3M;
    }
    if ( soccer->agents[4]->pos().dist( tar4[tn4]) < 0.05 && cnt4 > 1)
    {
        cnt4 = 0;
        tn4++;
        tn4%=tn4M;
    }

    cnt0++;
    cnt1++;
    cnt2++;
    cnt3++;
    cnt4++;

    gp0->setNoAvoid(true);
    gp1->setNoAvoid(true);
    gp2->setNoAvoid(true);
    gp3->setNoAvoid(true);
    //    gp4->setNoAvoid(true);

    gp0->setTarget(tar0[tn0],Vector2D(0,1));
    gp1->setTarget(tar1[tn1],Vector2D(0,1));
    gp2->setTarget(tar2[tn2],Vector2D(0,1));
    gp3->setTarget(tar3[tn3],Vector2D(0,1));
    gp4->setTarget(tar4[tn4],Vector2D(1,0));
    gp0->execute();
    gp1->execute();
    gp2->execute();
    gp3->execute();
//    gp4->execute();
#else
//    static int passer = 0;
//    static int oner = 1;
//    static CSkillKick* kick = new CSkillKick(soccer->agents[passer]);
//    static CSkillKickOneTouch* onetouch = new CSkillKickOneTouch(soccer->agents[oner]);
//    static int active = -1;
//    static int cntr = 50;

//    if ( knowledge->joystick->getButton6() && cntr > 50)
//    {
//        active *= -1;
//        cntr = 0;
//    }
//    cntr ++;

//    kick->setTarget( soccer->agents[oner]->self()->pos);
//    kick->setKickSpeed(4);
//    kick->setThroughMode(false);
//    kick->setSlow(true);
//    kick->setInterceptMode(false);
//    kick->setParallelMode(false);
//    kick->setWaitFrames( 0);
//    kick->setTolerance( 0.03);
//    kick->setDontKick(false);
//    kick->setChip(true);
//    if ( active == 1 &&  soccer->agents[passer]->self()->pos.dist(wm->ball->pos) < 0.5)
//        kick->execute();
//    else
//        kick->getAgent()->waitHere();

//    onetouch->setAgent(soccer->agents[oner]);
//    onetouch->setTarget( wm->field->oppGoal());
//    onetouch->setKickSpeed(15);
//    onetouch->setChip(false);
//    if ( active == 1)
//        onetouch->execute();
//    else
//        onetouch->getAgent()->waitHere();

//	int id = 5;
//	static int mode = 0;
//	static CSkillGotoPointAvoid* gp = new CSkillGotoPointAvoid( soccer->agents[id]);
//	static CSkillTurn* turn = new CSkillTurn( soccer->agents[2]);
//	static Vector2D dir(1,0);
//	dir.rotate( 3);
////	soccer->agents[id]->setRoller(7);
//	qDebug( ) << dir.x << dir.y;
//	gp->setTargetLook(wm->ball->pos+(dir.norm()*(CRobot::robot_radius_new+CBall::radius+0.09)), wm->ball->pos)	;
//	if ( mode )
//		gp->execute();
//	else
//		soccer->agents[id]->setRobotVel(0,0,0);

//	if ( knowledge->joystick->getButton6())
//		mode = 1;
//	else if ( knowledge->joystick->getButton8())
//		mode = 0;

//	double goal_p;
//	QList<int> ourrelaxed;
//	QList<int> opprelaxed;
//	Vector2D v(0,0);
//	v = knowledge->getEmptyPosOnGoal(wm->ball->pos(), goal_p, true, ourrelaxed, opprelaxed, 1.0, 1.0); ///calculate empty angle     //p*empty_width + (1-p)*goalwidth
//	static double vx = 0;
//	static double z = 0;
//	static double vy = 0.0;
//	z+=0.3;

//	vx = sin(z/20.00)*1;
//	vy = cos(z/20.00)*1;
//	if( z > 7200)
//	{
//			z = 0;
//	}

//	draw(QString("%1").arg(soccer->agents[0]->canOneTouch()),Vector2D(0,0));
//	soccer->agents[5]->setGyroZero();

//	soccer->agents[4]->setRobotVel(0,0,6.28);
//	turn->setDirection(Vector2D(0,1));
//	turn->execute();


//	knowledge->plotWidgetCustom[0] = vy;
//	knowledge->plotWidgetCustom[1] = wm->our[2]->vel.y;
//	knowledge->plotWidgetCustom[2] = wm->our[2]->acc.y;

//	knowledge->plotWidgetCustom[3] = wm->ball->vel.y;
//	knowledge->plotWidgetCustom[4] = wm->ball->acc.y;

//	knowledge->plotWidgetCustom[6] = wm->our[2]->pos.x;
//	knowledge->plotWidgetCustom[7] = wm->our[2]->pos.y;
#endif
#endif
}

#endif // EXPERIMENTAL3_H
