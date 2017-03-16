#include "markplan.h"
#include <cassert>
#include <WMatching.h>
#include <robot.h>
#include <algorithm>

using namespace std;

CMarkPlan::CMarkPlan()
{
    fastestTimeSum = 0;
    fastest = -1;
    ballInOurField = -1;
    distance = -1;
    lastFastest = -1;
    lastFastestFoundCycle = 0;
    fastestTime = 0;
    ourBallOwner = -1;
    supporter = NULL;
    oppBallOwner = -1;
    oppFastest = -1;
    ourFastest = -1;
    supporter = NULL;
    lastFrameInSupportMode = -1;


    /// --- HMD---///
    segmentper = .5;
    markRadius = 1.5;
    markRadiusStrict = 1.43;


    for(int i = 0; i< 6 ; i++)
    {
        markGPA[i] = new CSkillGotoPointAvoid(NULL);
    }
    /////////////////////

    for( int i=0 ; i<_MAX_NUM_PLAYERS ; i++ )
        oldMark[i] = new CRoleMark(NULL);
}

CMarkPlan::~CMarkPlan()
{
    for (int i=_MAX_NUM_PLAYERS-1 ; i>=0 ; i--)
        delete oldMark[i];
}

#define Ball_Ours_History 25
#define Our_Fastest_History 25
#define Opp_Fastest_History 25

//////////////////////////////// AHZ //////////////////////////////////////////
Line2D CMarkPlan::getBisectorLine(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint){
    Line2D bisectorLine (originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    return bisectorLine;
}
Segment2D CMarkPlan::getBisectorSegment(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint){
    Line2D bisectorLine (originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    Segment2D bisectorSegment(originPoint , Segment2D (thirdPoint , firstPoint).intersection(bisectorLine));
    return bisectorSegment;
}
Vector2D CMarkPlan::getPointInDirection(Vector2D firstPoint , Vector2D secondPoint , double proportion){
    firstPoint = firstPoint + (secondPoint - firstPoint).norm() * proportion * (Segment2D(secondPoint , firstPoint).length());
    return firstPoint;
}
void CMarkPlan::manToManMarkInPlayOnBlockPass(QList<Vector2D> opponentAgentsToBeMarkePossition , int ourMarkAgentsSize , double proportionOfDistance){
    ////////////////////////// Variables of this function //////////////////////
    int i;
    int j = 0;
    Vector2D opponentPlayMakerPossition;
    Vector2D ourCenterOfGoalPossition = wm->field->ourGoal();
    Vector2D opponentCenterOfGoalPossition = wm->field->oppGoal();
    Vector2D ourLeftOfGoalPossition = wm->field->ourGoalL();
    Vector2D ourRightOfGoalPossition = wm->field->ourGoalR();
    Vector2D sol1 , sol2;
    Vector2D sol3 , sol4;
    Vector2D sol5 , sol6;
    Vector2D ballPossition;
    Vector2D ballVelocity;
    Vector2D secondPos;
    Segment2D tempSegment;
    double ballCircleR = 0.09;
    double opponentAgentsCircleR = 0.2;
    QList<Circle2D> opponentAgentsToBeMarkedCircle;
    QList<Circle2D> tempOpponentAgentsToBeMarkedCircle;
    QList<Vector2D> tempMarkPoses;
    QList<Segment2D> opponentAgents2OpponentPlayMaker;
    QList<QPair<Vector2D , double> > tempSortDangerAgentsToBeBlockPassPlayOn;
    QList<Vector2D> ourMarkAgentsPossition;
    QList<QPair<Vector2D,double> > sortDangerAgentsToBeBlockPassPlayOn;
    Circle2D opponentPlayMakerCircle;
    Circle2D ourGoalCircle(ourCenterOfGoalPossition , 1.43);
    Circle2D opponentgoalCircle(opponentCenterOfGoalPossition , 1);
    Segment2D opponentPlayMaker2OurGoal;
    //////////////////// Clear QLists for change the states ////////////////////
    ourMarkAgentsPossition.clear();
    markPoses.clear();
    markAngs.clear();
    ///////// Make Cirlcles around opponent agents /////////////////////////////
    for(i = 0 ; i < opponentAgentsToBeMarkePossition.size() ; i++){
        opponentAgentsToBeMarkedCircle.append(Circle2D(opponentAgentsToBeMarkePossition.at(i) , opponentAgentsCircleR));
        draw(opponentAgentsToBeMarkedCircle.at(i),QColor(Qt::cyan));
    }
    if(opponentAgentsToBeMarkePossition.size() == 0){// opponentAgentsToBeMarkePossition.size() == 1){
        debug("0 ta darim" , D_SEPEHR);
        for(i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
            ourMarkAgentsPossition.append(wm->opp.active(i)->pos);
        }
        for(i = 0 ; i < opponentAgentsToBeMarkePossition.size() ; i++){
            for(j = 0 ; j < ourMarkAgentsPossition.size() ; j++){
                if(opponentAgentsToBeMarkePossition.at(i) == ourMarkAgentsPossition.at(j)){
                    ourMarkAgentsPossition.removeAt(j);
                }
            }
        }
        tempSortDangerAgentsToBeBlockPassPlayOn = sortdangerpassplayoff(ourMarkAgentsPossition);
        for(i = 0 ; i < tempSortDangerAgentsToBeBlockPassPlayOn.size() ; i++){
            tempOpponentAgentsToBeMarkedCircle.append(Circle2D(tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first , opponentAgentsCircleR));
            draw(tempOpponentAgentsToBeMarkedCircle.at(i),QColor(Qt::cyan));
        }

        for(i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++){
            tempOpponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first , wm->ball->pos) , &sol1 , &sol2);
            Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first , wm->ball->pos) , &sol3 , &sol4);
            tempMarkPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                     Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4,
                                                     proportionOfDistance));
            markAngs.append(wm->ball->pos - tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first);
            if(wm->field->isInOurPenaltyArea(tempMarkPoses.at(i))){
                debug("AHZ" , D_SEPEHR);
                if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first)){
                    ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                         Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);
                    tempMarkPoses.removeAt(i);
                    tempMarkPoses.append(getPointInDirection(tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first
                                                             ,Segment2D(sol5 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol6 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol5 : sol6
                                                                                                                                                                                                                                , proportionOfDistance));
                }
                else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first)){
                    ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                         Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);
                    tempMarkPoses.removeAt(i);
                    tempMarkPoses.append(getPointInDirection(wm->ball->pos
                                                             ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                    , proportionOfDistance));
                }
                else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first)){
                    ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                         Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                    tempMarkPoses.removeAt(i);
                    tempMarkPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                }
                else{
                    ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                         Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);

                    //                            if(isValidPoint(sol5) && isValidPoint(sol6)){
                    //                                if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                    //                                        Segment2D(Segment2D(sol5 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol6 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol5:sol6 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length()){
                    //                                    tempMarkPoses.removeAt(i);
                    //                                    draw(Segment2D(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                    //                                    tempMarkPoses.append(getPointInDirection(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                    //                                    markAngs.append(wm->ball->pos - tempMarkPoses.at(i));
                    //                                }

                    //                                else{
                    //                                    tempMarkPoses.removeAt(i);
                    //                                    tempMarkPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol6 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol5:sol6 , proportionOfDistance));
                    //                                    markAngs.append(wm->ball->pos - tempMarkPoses.at(i));
                    //                                }
                    //                            }
                    //                            else{
                    //                                tempMarkPoses.removeAt(i);
                    //                                draw(Segment2D(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                    //                                tempMarkPoses.append(getPointInDirection(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                    //                                markAngs.append(wm->ball->pos - tempMarkPoses.at(i));
                    //                            }
                }
            }


        }
        markPoses.append(tempMarkPoses);
        debug(QString("s :%1").arg(markPoses.size()) , D_SEPEHR);
        debug(QString("coach :%1").arg(ourMarkAgentsSize) , D_SEPEHR);


    }
    else{
        ////////////////////// Drawing the lines between opponent agents to the both side of our goal ////////////////////////////
        if(ourMarkAgentsSize == 1){
            if(opponentAgentsToBeMarkePossition.size() == ourMarkAgentsSize){
                for(i = 0 ; i < ourMarkAgentsSize ; i++){
                    opponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(opponentAgentsToBeMarkePossition.at(i) , wm->ball->pos) , &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(opponentAgentsToBeMarkePossition.at(i) , wm->ball->pos) , &sol3 , &sol4);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                         Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4,
                                                         proportionOfDistance));
                    //////////////////// check if the mark agents possition is in ourPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOurPenaltyArea(markPoses.at(i))){
                        if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                                 ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                          , proportionOfDistance));
                        }
                        else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(wm->ball->pos
                                                                 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                        , proportionOfDistance));
                        }
                        else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            markPoses.removeAt(i);
                            markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                            //                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            //                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                            //                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                            //                                markPoses.removeAt(i);
                            //                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                            else{
                            //                                markPoses.removeAt(i);
                            //                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                        }
                            //                        else{
                            //                            markPoses.removeAt(i);
                            //                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                            markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                        }
                        }
                    }


                    //////////////////// check if the mark agents possition is in opponentPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOppPenaltyArea(markPoses.at(i))){
                        if(wm->field->isInOppPenaltyArea(markPoses.at(i)) && !wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                                 ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                          , proportionOfDistance));
                        }
                        else if(!wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(wm->ball->pos
                                                                 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                        , proportionOfDistance));
                        }
                        else if(wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            markPoses.removeAt(i);
                            markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                            //                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            //                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                            //                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                            //                                markPoses.removeAt(i);
                            //                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                            else{
                            //                                markPoses.removeAt(i);
                            //                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                        }
                            //                        else{
                            //                            markPoses.removeAt(i);
                            //                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                            markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                        }
                        }
                    }
                    markAngs.append(wm->ball->pos - markPoses.at(i));
                    draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                   Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , QColor(Qt::red));
                    draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                    draw(ourGoalCircle,QColor(Qt::black));
                    debug(QString("Man To Man Mark In PlayOff Mode / BlockPass / our = opp") , D_SEPEHR);
                }
            }

            else if (opponentAgentsToBeMarkePossition.size() > ourMarkAgentsSize){
                sortDangerAgentsToBeBlockPassPlayOn = sortdangerpassplayon(opponentAgentsToBeMarkePossition);
                for(i = 0 ; i < ourMarkAgentsSize ; i++){
                    Circle2D(sortDangerAgentsToBeBlockPassPlayOn.at(i).first , opponentAgentsCircleR)
                            .intersection(Segment2D(sortDangerAgentsToBeBlockPassPlayOn.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR)
                            .intersection(Segment2D(sortDangerAgentsToBeBlockPassPlayOn.at(i).first , wm->ball->pos) , &sol3 , &sol4);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                         Segment2D(sol3 , sortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4,
                                                         proportionOfDistance));
                    //////////////////// check if the mark agents possition is in ourPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOurPenaltyArea(markPoses.at(i))){
                        debug(QString("oomad") , D_SEPEHR);
                        if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                                 ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                          , proportionOfDistance));
                            debug(QString("gaeedam") , D_SEPEHR);
                        }
                        else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(wm->ball->pos
                                                                 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                        , proportionOfDistance));
                            debug(QString("gaeedam") , D_SEPEHR);
                        }
                        else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            markPoses.removeAt(i);
                            debug(QString("gaeedam") , D_SEPEHR);
                            markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                            //                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            //                            debug(QString("hastan") , D_SEPEHR);
                            //                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                            //                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                            //                                markPoses.removeAt(i);
                            //                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                                debug(QString("boo miad") , D_SEPEHR);
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                            else{
                            //                                markPoses.removeAt(i);
                            //                                debug(QString("sag zade") , D_SEPEHR);
                            //                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                        }
                            //                        else{
                            //                            markPoses.removeAt(i);
                            //                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                            debug(QString("boo miad") , D_SEPEHR);
                            //                            markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                        }
                        }
                    }
                    //////////////////// check if the mark agents possition is in opponentPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOppPenaltyArea(markPoses.at(i))){
                        if(wm->field->isInOppPenaltyArea(markPoses.at(i)) && !wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                                 ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                          , proportionOfDistance));
                        }
                        else if(!wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(wm->ball->pos
                                                                 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                        , proportionOfDistance));
                        }
                        else if(wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            markPoses.removeAt(i);
                            markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                            //                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            //                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                            //                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                            //                                markPoses.removeAt(i);
                            //                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                            else{
                            //                                markPoses.removeAt(i);
                            //                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                        }
                            //                        else{
                            //                            markPoses.removeAt(i);
                            //                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                            markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                        }
                        }
                    }

                    markAngs.append(wm->ball->pos - markPoses.at(i));

                    draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                   Segment2D(sol3 , sortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4)
                         , QColor(Qt::red));
                    draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                    debug(QString("MostDanger = %1").arg(sortDangerAgentsToBeBlockPassPlayOn.at(0).second) ,D_SEPEHR , QColor("red"));
                    debug(QString("opponentAgentsToBeMarkePossitionSize = %1").arg(opponentAgentsToBeMarkePossition.size()) ,D_SEPEHR , QColor("blue"));
                    debug(QString("MarkAngsSize = %1").arg(markAngs.size()) ,D_SEPEHR , QColor("red"));
                    debug(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp > our") , D_SEPEHR);
                }

            }

            else{
                ourGoalCircle.intersection(Segment2D(ourGoalCircle.center() , wm->ball->pos) , &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(ourGoalCircle.center() , wm->ball->pos) , &sol3 , &sol4);
                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2
                                                                                                                                           ,Segment2D(sol3 , ourGoalCircle.center()).length() < Segment2D(sol4 , ourGoalCircle.center()).length() ? sol3 : sol4
                                                                                                                                                                                                                                                    ,proportionOfDistance));
                markAngs.append(wm->ball->pos - markPoses.last());
                debug(QString("Man To Man Mark In PlayOn / BlockPass / our = 1 / opp < our") , D_SEPEHR);
            }
        }
        if(ourMarkAgentsSize == 2){

            if(opponentAgentsToBeMarkePossition.size() == ourMarkAgentsSize){
                for(i = 0 ; i < ourMarkAgentsSize ; i++){
                    opponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(opponentAgentsToBeMarkePossition.at(i) , wm->ball->pos) , &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(opponentAgentsToBeMarkePossition.at(i) , wm->ball->pos) , &sol3 , &sol4);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                         Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4,
                                                         proportionOfDistance));
                    //////////////////// check if the mark agents possition is in ourPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOurPenaltyArea(markPoses.at(i))){
                        if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                                 ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                          , proportionOfDistance));
                        }
                        else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(wm->ball->pos
                                                                 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                        , proportionOfDistance));
                        }
                        else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            markPoses.removeAt(i);
                            markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                            //                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            //                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                            //                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                            //                                markPoses.removeAt(i);
                            //                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                            else{
                            //                                markPoses.removeAt(i);
                            //                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                        }
                            //                        else{
                            //                            markPoses.removeAt(i);
                            //                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                            markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                        }
                        }
                    }
                    //////////////////// check if the mark agents possition is in opponentPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOppPenaltyArea(markPoses.at(i))){
                        if(wm->field->isInOppPenaltyArea(markPoses.at(i)) && !wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                                 ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                          , proportionOfDistance));
                        }
                        else if(!wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(wm->ball->pos
                                                                 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                        , proportionOfDistance));
                        }
                        else if(wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            markPoses.removeAt(i);
                            markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                            //                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            //                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                            //                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                            //                                markPoses.removeAt(i);
                            //                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                            else{
                            //                                markPoses.removeAt(i);
                            //                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                        }
                            //                        else{
                            //                            markPoses.removeAt(i);
                            //                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                            markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                        }
                        }
                    }
                    markAngs.append(wm->ball->pos - markPoses.at(i));
                    draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                   Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , QColor(Qt::red));
                    draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                    draw(ourGoalCircle,QColor(Qt::black));
                    debug(QString("Man To Man Mark In PlayOff Mode / BlockPass / our = opp") , D_SEPEHR);
                }
            }

            else if(opponentAgentsToBeMarkePossition.size() > ourMarkAgentsSize){
                sortDangerAgentsToBeBlockPassPlayOn = sortdangerpassplayon(opponentAgentsToBeMarkePossition);
                for(i = 0 ; i < ourMarkAgentsSize ; i++){
                    Circle2D(sortDangerAgentsToBeBlockPassPlayOn.at(i).first , opponentAgentsCircleR)
                            .intersection(Segment2D(sortDangerAgentsToBeBlockPassPlayOn.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR)
                            .intersection(Segment2D(sortDangerAgentsToBeBlockPassPlayOn.at(i).first , wm->ball->pos) , &sol3 , &sol4);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                         Segment2D(sol3 , sortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4,
                                                         proportionOfDistance));
                    //////////////////// check if the mark agents possition is in ourPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOurPenaltyArea(markPoses.at(i))){
                        debug(QString("oomad") , D_SEPEHR);
                        if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                                 ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                          , proportionOfDistance));
                            debug(QString("gaeedam") , D_SEPEHR);
                        }
                        else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(wm->ball->pos
                                                                 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                        , proportionOfDistance));
                            debug(QString("gaeedam") , D_SEPEHR);
                        }
                        else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            markPoses.removeAt(i);
                            debug(QString("gaeedam") , D_SEPEHR);
                            markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                 Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                        }
                    }
                    //////////////////// check if the mark agents possition is in opponentPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOppPenaltyArea(markPoses.at(i))){
                        if(wm->field->isInOppPenaltyArea(markPoses.at(i)) && !wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                                 ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                          , proportionOfDistance));
                        }
                        else if(!wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(wm->ball->pos
                                                                 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                        , proportionOfDistance));
                        }
                        else if(wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            markPoses.removeAt(i);
                            markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                            //                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            //                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                            //                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                            //                                markPoses.removeAt(i);
                            //                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                            else{
                            //                                markPoses.removeAt(i);
                            //                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                        }
                            //                        else{
                            //                            markPoses.removeAt(i);
                            //                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                            markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                        }
                        }
                    }
                    markAngs.append(wm->ball->pos - markPoses.at(i));

                    draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                   Segment2D(sol3 , sortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeBlockPassPlayOn.at(i).first).length() ? sol3 : sol4)
                         , QColor(Qt::red));
                    draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                    debug(QString("MostDanger = %1").arg(sortDangerAgentsToBeBlockPassPlayOn.at(0).second) ,D_SEPEHR , QColor("red"));
                    debug(QString("opponentAgentsToBeMarkePossitionSize = %1").arg(opponentAgentsToBeMarkePossition.size()) ,D_SEPEHR , QColor("blue"));
                    debug(QString("MarkAngsSize = %1").arg(markAngs.size()) ,D_SEPEHR , QColor("red"));
                    debug(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp > our") , D_SEPEHR);
                }
            }

            else{
                for(i = 0 ; i < ourMarkAgentsSize ; i++){
                    opponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkePossition.at(i))
                                                                      , &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkePossition.at(i))
                                                                       , &sol3 , &sol4);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2
                                                                                                                                               ,Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4
                                                                                                                                                                                                                                                                                        , proportionOfDistance));
                    ////////////////////// check if the mark agents possition is in ourPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOurPenaltyArea(markPoses.at(i))){
                        ourGoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                             Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.removeAt(i);
                        markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                             ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                      , proportionOfDistance));
                    }
                    //////////////////// check if the mark agents possition is in opponentPenaltyArea or not ////////////////////////////////////////////////////////////
                    if(wm->field->isInOppPenaltyArea(markPoses.at(i))){
                        if(wm->field->isInOppPenaltyArea(markPoses.at(i)) && !wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                                 ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                          , proportionOfDistance));
                        }
                        else if(!wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                            markPoses.removeAt(i);
                            markPoses.append(getPointInDirection(wm->ball->pos
                                                                 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                        , proportionOfDistance));
                        }
                        else if(wm->field->isInOppPenaltyArea(wm->ball->pos) && wm->field->isInOppPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            markPoses.removeAt(i);
                            markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            opponentgoalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                                      Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                            //                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            //                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                            //                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                            //                                markPoses.removeAt(i);
                            //                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                            else{
                            //                                markPoses.removeAt(i);
                            //                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                            //                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                            }
                            //                        }
                            //                        else{
                            //                            markPoses.removeAt(i);
                            //                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            //                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            //                            markAngs.append(wm->ball->pos - markPoses.at(i));
                            //                        }
                        }
                    }
                    markAngs.append(wm->ball->pos - markPoses.at(i));
                    draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2
                                                                                                                         ,Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4)
                         , QColor(Qt::red));
                    draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                    debug(QString("Man To Man Mark In PlayOn / BlockPass / our = 2 / opp < our") , D_SEPEHR);
                }
                ourGoalCircle.intersection(Segment2D(ourGoalCircle.center() , wm->ball->pos) , &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(ourGoalCircle.center() , wm->ball->pos) , &sol3 , &sol4);
                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2
                                                                                                                                           ,Segment2D(sol3 , ourGoalCircle.center()).length() < Segment2D(sol4 , ourGoalCircle.center()).length() ? sol3 : sol4
                                                                                                                                                                                                                                                    ,proportionOfDistance));
                markAngs.append(wm->ball->pos - markPoses.last());
            }
        }
    }
    //////////////// Draw Possition of Mark Agents //////////////////////////
    debug(QString("agentsCount : %1").arg(agents.size()) , D_SEPEHR);
    debug(QString("markPosesSize : %1").arg(markPoses.size()) , D_SEPEHR);
    for(i = 0 ; i < markPoses.size() ; i++){
        draw(markPoses.at(i),1,QColor(Qt::red));
    }
}
//////////////////////////////// end of AHZ ///////////////////////////////////
void CMarkPlan::findBallOwnership()
{
    if (ballInOurField == -1)
    {
        if (wm->ball->pos.x < -1) ballInOurField = 1;
        else ballInOurField = 0;
    }
    else {
        if (wm->ball->pos.x < -1.2) ballInOurField = 1;
        if (wm->ball->pos.x >  -0.8) ballInOurField = 0;
    }


    // if (count() == 0) return;
    oppOffenders.clear();
    ourOffenders.clear();
    ourOffenders = wm->our.data->activeAgents;
    /*for (int i=0;i<count();i++)
        {
                ourOffenders.append(agent(i)->id());
        }*/
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        if ((wm->opp.active(i)->pos - wm->field->oppGoal()).length() > 1.4)
        {
            oppOffenders.append(wm->opp.active(i)->id);
        }
    }
    QString s1, s2;
    for (int i=0;i<ourOffenders.count();i++)
    {
        s1 = s1 + QString(" , %1").arg(ourOffenders[i]);
    }

    for (int i=0;i<oppOffenders.count();i++)
    {
        s2 = s2 + QString(" , %1").arg(oppOffenders[i]);
    }
    // draw(QString("our: %1 , opp: %2").arg(s1).arg(s2), Vector2D(-2, -2.4), "blue");

    NewFastestToBall f = knowledge->newFastestToBall(0.016, wm->our.data->activeAgents, wm->opp.data->activeAgents);//ourOffenders, oppOffenders);
    //NewFastestToBall f = knowledge->findFastest(0.016, ourOffenders, oppOffenders);
    fastestTime = f.ourFastestTime() - f.oppFastestTime();


    double factor = 0.3; //integration factor
    if (fastestTime > 0)
    {
        fastestTimeSum += 1.5*fastestTime*factor;
    }
    else {
        fastestTimeSum += fastestTime*factor;
    }
    //fastestTimeSum += fastestTime;

    if (fastestTimeSum > 1)
    {
        fastestTimeSum = 1;
    }
    else if (fastestTimeSum < -1){
        fastestTimeSum = -1;
    }

    if (fastestTimeSum > 0.5)
        ballOurs = false;
    if (fastestTimeSum < -0.5)
        ballOurs = true;


    if (wm->ball->vel.length() > 0.5)
    {
        for (int i=0;i<oppOffenders.count();i++)
        {
            if  (fabs(AngleDeg::normalize_angle((wm->opp[oppOffenders[i]]->pos - wm->ball->pos).th().degree() - wm->ball->vel.th().degree()))
                 < 15.0)
            {
                bool flag = false;
                for (int j=0;j<ourOffenders.count();j++)
                {
                    Vector2D sol1, sol2;
                    if (wm->our[ourOffenders[j]]->getCirle().intersection(Segment2D(wm->ball->pos, wm->opp[oppOffenders[i]]->pos), &sol1, &sol2) > 0)
                    {
                        flag = true;
                        break;
                    }
                }
                if (!flag) //We dont have any blockers, sadly we're gonna lose the ball
                {
                    ballOurs = false;
                    //                        draw("Ball going to opponent :(", Vector2D(-2, 1.3), "blue");
                    break;
                }
            }
        }
    }

    //        draw(QString("fastest time:%1 , sum:%2 , our count:%3, opp count:%4").arg(fastestTime).arg(fastestTimeSum).arg(ourOffenders.count()).arg(oppOffenders.count()), Vector2D(-2, -2), "blue");
    /*
        if (fastestTime < 0.20) {
                ballOurs = true;
        }
        if (fastestTime > -0.20) {
                ballOurs = false;
        }
*/
    bool ballInFrontOfOpp = false;

    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        double d = (wm->opp.active(i)->pos - wm->ball->pos).length();
        if (d < 0.3) //check if there is an opponent in front of ball
        {
            if (fabs(wm->opp.active(i)->dir.th().degree() - (wm->ball->pos - wm->opp.active(i)->pos).th().degree() ) < 20.0)
            {
                ballInFrontOfOpp = true;
            }
        }
    }

    bool ballInFrontOfOur = false;

    for (int i=0;i<wm->our.activeAgentsCount();i++)
    {
        double d = (wm->our.active(i)->pos - wm->ball->pos).length();
        if (d < 0.3) //check if there is an opponent in front of ball
        {
            if (fabs(wm->our.active(i)->dir.th().degree() - (wm->ball->pos - wm->our.active(i)->pos).th().degree() ) < 20.0)
            {
                ballInFrontOfOur = true;
                //                    draw("Coming to opponent", Vector2D(-2, -0.5), "blue");
            }
        }
    }

    if (ballInFrontOfOpp) ballOurs = false;
    //else if (ballInFrontOfOur) ballOurs = true;

    if (oppOffenders.count()==0) ballOurs = true;
    if (ballOursQueue.length() < Ball_Ours_History)
    {
        ballOursQueue.append(ballOurs);
    }
    else {
        ballOursQueue.removeFirst();
        ballOursQueue.append(ballOurs);
    }

    //        if (ballOurs) draw("We gotta own ball", Vector2D(-2, 1.9), "blue");

    int recentFramesBallWasOurs = 0;
    for (int i=0;i<ballOursQueue.size();i++)
        if (ballOursQueue[i]) recentFramesBallWasOurs++;
    if (recentFramesBallWasOurs >= ballOursQueue.size()/2)
    {
        weOwnBall = true;
    }
    else weOwnBall = false;

    if (knowledge->frameCount - knowledge->lastFrameTheirNonPlayKick < 120)
    {
        draw("Forcing Mark , Feeling Sexy", Vector2D(0.0, -1.5), "red");
        weOwnBall = false;
    }



    if (ballInOurField == 1)
    {
        weOwnBall = false;
        //            draw("Ball in our field", Vector2D(-2, 1.6), "blue");
    }
    if (ourOffenders.size() > 1)
    {
        if (ourFastest!=f.ourFastest())
        {
            if (ourFastest != -1)
            {
                NewFastestToBall f2 = knowledge->newFastestToBall(0.016,QList<int>() << ourFastest);
                if (fabs(f2.ourFastestTime() - f.ourFastestTime()) > 0.4)
                {
                    ourFastest = f.ourFastest();
                }
            }
            else ourFastest = f.ourFastest();
        }
    }
    else ourFastest = f.ourFastest();
    if (ourFastestQueue.length() < Our_Fastest_History)
    {
        ourFastestQueue.append(ourFastest);
    }
    else {
        ourFastestQueue.removeFirst();
        ourFastestQueue.append(ourFastest);
    }
    int fastestHistogram[_MAX_NUM_PLAYERS];
    for (int i=0;i<_MAX_NUM_PLAYERS;i++)
        fastestHistogram[i] = 0;
    for (int i=0;i<ourFastestQueue.size();i++)
    {
        if (ourFastestQueue[i] != -1 && ourOffenders.contains(ourFastestQueue[i]))
            fastestHistogram[ourFastestQueue[i]]++;
    }
    int maxFastestHistogram = -1;
    int newFastest = -1;
    for (int i=0;i<_MAX_NUM_PLAYERS;i++)
    {
        if (fastestHistogram [i] > maxFastestHistogram)
        {
            maxFastestHistogram = fastestHistogram [i];
            newFastest = i;
        }
    }
    if (newFastest != -1)
    {
        ourBallOwner = newFastest;
    }
    if (ourBallOwner == -1)
    {
        ourBallOwner = f.ourFastest();
        ourFastestTime = f.ourFastestTime();
    }
    if (ourBallOwner == -1)
        ourBallOwner = agent(0)->id();

    /////Find Opponents Fastest Agent



    if (oppOffenders.size() > 1)
    {
        if (oppFastest!=f.oppFastest())
        {
            if (oppFastest != -1)
            {
                FastestToBall f2 = knowledge->findFastestToBall(QList<int>(), QList<int>() << oppFastest);
                if (fabs(f2.oppFastestTime - f.oppFastestTime()) > 0.2)
                {
                    oppFastest = f.oppFastest();
                    qDebug() << "ff:"<< fabs(f2.oppFastestTime - f.oppFastestTime());
                    //                                    draw(QString("%1").arg(fabs(f2.oppFastestTime - f.oppFastestTime())), Vector2D(-2, 1.8));
                }
            }
            else oppFastest = f.oppFastest();
        }
    }
    else oppFastest = f.oppFastest();
    if (oppFastestQueue.length() < Opp_Fastest_History)
    {
        oppFastestQueue.append(oppFastest);
    }
    else {
        oppFastestQueue.removeFirst();
        oppFastestQueue.append(oppFastest);
    }
    for (int i=0;i<_MAX_NUM_PLAYERS;i++)
        fastestHistogram[i] = 0;
    for (int i=0;i<oppFastestQueue.size();i++)
    {
        if (oppFastestQueue[i] != -1 && oppOffenders.contains(oppFastestQueue[i]))
            fastestHistogram[oppFastestQueue[i]]++;
    }
    maxFastestHistogram = -1;
    newFastest = -1;
    for (int i=0;i<_MAX_NUM_PLAYERS;i++)
    {
        if (fastestHistogram [i] > maxFastestHistogram)
        {
            maxFastestHistogram = fastestHistogram [i];
            newFastest = i;
        }
    }
    if (newFastest != -1)
    {
        oppBallOwner = newFastest;
    }
    if (oppBallOwner == -1)
    {
        oppBallOwner = f.oppFastest();
        oppFastestTime = f.oppFastestTime();
    }
    if (oppOffenders.size() > 0)
    {
        if (oppBallOwner == -1)
            oppBallOwner = oppOffenders[0];
    }
}

void CMarkPlan::extractGameSituation(){
    findBallOwnership();

    if (weOwnBall)
    {
        draw ("We Own The Ball", Vector2D(-2,1.5), "white");
    }
    else {
        draw ("We Lost It!", Vector2D(-2,1.5), "white");
    }

    NewFastestToBall result = knowledge->newFastestToBall(0.016 , wm->our.data->activeAgents , wm->opp.data->activeAgents);
    double ourTim = -1 , oppTim = -1;
    if( result.ourF.size() )
        ourTim = result.ourF.at(0).first;
    if( result.oppF.size() )
        oppTim = result.oppF.at(0).first;

    if( oppTim == -1 ){
        ballCatch = 1;
    }
    else if( ourTim == -1 ){
        ballCatch = 0;
    }
    else{
        if( ourTim >= oppTim )
            ballCatch = 0;
        else if( 2*ourTim <= oppTim )
            ballCatch = 1;
        else{
            ballCatch = sqrt(fabs(oppTim - ourTim)*2.0/oppTim);
        }
    }

    oppsInOurOneThirth = 0;
    for( int i=0 ; i<knowledge->toBeMopps.size() ; i++ )
        if( knowledge->toBeMopps[i]->pos.x < -1 )
            oppsInOurOneThirth++;

    ballCatcherDanger = false;
    if( result.isFastestOurs == false && result.oppF.size() ){
        vector<int> all;
        for( int i=0 ; i<knowledge->toBeMopps.size() ; i++ )
            all.push_back(knowledge->toBeMopps[i]->id );
        if( find(all.begin() , all.end() , result.oppF.at(0).second ) != all.end() )
            ballCatcherDanger = true;
    }

    numberOfMarkers = 0;

    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        double d = (wm->opp.active(i)->pos - wm->ball->pos).length();
        if (d < 0.3) //check if there is an opponent in front of ball
        {
            if (fabs(wm->opp.active(i)->dir.th().degree() - (wm->ball->pos - wm->opp.active(i)->pos).th().degree() ) < 20.0)
            {
                numberOfMarkers++;
            }
        }
    }



    if( fabs(ballCatch) < 1e-6 ){
        numberOfMarkers = knowledge->toBeMopps.size();
    }
    else if( fabs(ballCatch - 1) < 0.1 && oppsInOurOneThirth >= 2 ){
        numberOfMarkers = 1;
    }
    else if( fabs(ballCatch - 1) < 0.1 ){
        numberOfMarkers = 0;
    }
    else if( fabs(ballCatch - 1) < 0.3 && oppsInOurOneThirth >= 1 ){
        numberOfMarkers = 1;
    }
    else if( fabs(ballCatch - 1) < 0.3 ){
        numberOfMarkers = 0;
    }
    else if( fabs(ballCatch - 1) < 0.5 && oppsInOurOneThirth >= 2 ){
        numberOfMarkers = 2;
    }
    else if( fabs(ballCatch - 1) < 0.5 ){
        numberOfMarkers = 1;
    }
    else if( fabs(ballCatch - 1) < 0.7 && oppsInOurOneThirth >= 3 ){
        numberOfMarkers = 3;
    }
    else if( fabs(ballCatch - 1) < 0.7 ){
        numberOfMarkers = 2;
    }
    else{
        numberOfMarkers = knowledge->toBeMopps.size();
    }
}




//-------//mhsh - HMD//----------//


QList<Vector2D> CMarkPlan::ZoneMark(QList<QPair<Vector2D, double> > Posdanger , int marknum)
{
    //input is sorted Dangerious Weigth
    segmentper = policy()->Mark_ShootRatioBlock();
    double zoneradius = .2;
    double limitradus = 1.5;
    double manTomanRadius = 1.0;
    Vector2D sol1,sol2,sol;
    Line2D tempLine(Vector2D(0,0), Vector2D(1,1));
    Segment2D tempseg;
    Circle2D circlemilde;
    Circle2D circlelimit;
    Circle2D manTomanArea(wm->ball->pos, manTomanRadius);
    Vector2D midle;
    QList<Vector2D> output;
    output.clear();
    segmentper = policy()->Mark_ShootRatioBlock()/100;
    if (marknum == 1)
    {
        //  Vector2D::angleOf(Posdanger[0].first, wm->field->ourGoal(), posdanger[1]).degree();
        if(Posdanger.size() == 1)
        {
            //Shoot block
            tempseg.assign(Posdanger[0].first, wm->field->ourGoal());
            markPoses.append(Posdanger[0].first + (wm->field->ourGoal() - Posdanger[0].first) * segmentper);
            markAngs.append(Posdanger[0].first - wm->field->ourGoal());

        }
        if(Posdanger.size() > 1)
        {
            /* real Zone

            tempseg.assign(Posdanger[0].first, Posdanger[1].first);

            draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {

                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }


                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


*/


            // man to man
            markPoses.append(ShootBlockRatio(segmentper, Posdanger[0].first).first());
            markAngs.append(ShootBlockRatio(segmentper, Posdanger[0].first).last());

        }
    }

    else if(marknum == 2)
    {
        if(Posdanger.size() == 1)
        {
            //Shoot block
            markPoses.append(ShootBlockRatio(segmentper, Posdanger[0].first).first());
            markAngs.append(ShootBlockRatio(segmentper, Posdanger[0].first).last());

            //block pass
            tempseg.assign(wm->ball->pos, Posdanger[0].first);
            markPoses.append(tempseg.intersection(tempseg.perpendicularBisector()));
            markAngs.append(Posdanger[0].first - markPoses.last());




        }
        if(Posdanger.size() == 2)
        {

            //shoot block
            markPoses.append(ShootBlockRatio(segmentper, Posdanger[0].first).first());
            markAngs.append(ShootBlockRatio(segmentper, Posdanger[0].first).last());

            markPoses.append(ShootBlockRatio(segmentper, Posdanger[1].first).first());
            markAngs.append(ShootBlockRatio(segmentper, Posdanger[1].first).last());



        }
        if(Posdanger.size() > 2)
        {
            /* zone mark
            ////fisrt one
            tempseg.assign(Posdanger[0].first, Posdanger[1].first);

            draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                debug(QString("Going to penalty Area"), D_MAHI);
                debug(QString("sol1:x:%1y:%2 sol2:x:%3y:%4").arg(sol1.x).arg(sol1.y).arg(sol2.x).arg(sol2.y),D_MAHI);
                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }



            ///second one

            tempseg.assign(Posdanger[1].first, Posdanger[2].first);

            draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                debug(QString("Going to penalty Area"), D_MAHI);
                debug(QString("sol1:x:%1y:%2 sol2:x:%3y:%4").arg(sol1.x).arg(sol1.y).arg(sol2.x).arg(sol2.y),D_MAHI);

                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


*/
            // man to man

            //shoot block
            markPoses.append(ShootBlockRatio(segmentper, Posdanger[0].first).first());
            markAngs.append(ShootBlockRatio(segmentper, Posdanger[0].first).last());

            markPoses.append(ShootBlockRatio(segmentper, Posdanger[1].first).first());
            markAngs.append(ShootBlockRatio(segmentper, Posdanger[1].first).last());


        }
    }
    else if(marknum == 3)
    {
        if(Posdanger.size() > 3)
        {
            tempseg.assign(Posdanger[0].first, Posdanger[1].first);

            draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


            ///second one

            tempseg.assign(Posdanger[1].first, Posdanger[2].first);

            draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


            //third one


            tempseg.assign(Posdanger[2].first, Posdanger[3].first);

            draw(tempseg,QColor(Qt::green));
            midle = tempseg.intersection(tempseg.perpendicularBisector());
            tempseg.assign(wm->field->ourGoal(), midle);
            tempLine.assign(wm->field->ourGoal(),midle);
            draw(tempseg, QColor(Qt::green));

            circlemilde.assign(midle, zoneradius);
            circlemilde.intersection(tempLine, &sol1, &sol2);
            circlelimit.assign(wm->field->ourGoal(), limitradus);


            //cheking which on is nearest
            if((wm->field->ourGoal() - sol1).length() < (wm->field->ourGoal() - sol2).length())
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            if(!circlelimit.contains(sol))
            {
                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }
            else
            {
                circlelimit.intersection(tempLine, &sol1, &sol2);
                if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
                {
                    sol = sol1;
                }
                else
                {
                    sol = sol2;
                }



                markPoses.append(sol);
                markAngs.append(Vector2D(midle - wm->field->ourGoal()));
                output.append(markPoses.last());
            }


        }

    }
    return output;


}

QList<QPair<Vector2D, double> > CMarkPlan::sortdangershoot(double _radius, double treshold) {

    Vector2D sol1,sol2,sol3;
    Vector2D _pos1 = wm->ball->pos;
    Vector2D _pos2 = wm->ball->pos + 5.0  * wm->ball->vel;
    Line2D _path(_pos1,_pos2);
    Polygon2D _poly;
    Circle2D(_pos2,_radius + treshold).
            intersection(_path.perpendicular(_pos2),&sol1,&sol2);


    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(_pos1,CRobot::robot_radius_new + treshold).
            intersection(_path.perpendicular(_pos1),&sol1,&sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    draw(_poly,"blue");


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    output.clear();


    double KA = 1; //Angle parameter
    double KDB = 1; //distancetoball
    double KDI = 1; //distancetointersect

    double Angle;
    double distanceToBallProjection;
    double distanceToIntersect;
    double danger;

    double RangeofAngle = 90;
    double RangeofdistanceToBallProjection = Segment2D(Vector2D(-1.0 * _FIELD_WIDTH / 2, -1.0 * _FIELD_HEIGHT /2 ), Vector2D(_FIELD_WIDTH / 2 , _FIELD_HEIGHT / 2)).length();
    double RangeofdistanceToIntersect =  _radius;


    for(int i = 0;i < oppAgentsToMarkPos.count();i++) {
        if(_poly.contains(oppAgentsToMarkPos[i]))
        {
            Angle = Vector2D::angleOf( oppAgentsToMarkPos[i], wm->ball->pos, _path.projection(oppAgentsToMarkPos[i]) ).degree();
            temp.first = oppAgentsToMarkPos[i];


            distanceToIntersect = _path.dist(oppAgentsToMarkPos[i]);
            distanceToBallProjection = _path.projection(oppAgentsToMarkPos[i]).length();
            danger = KA * ( 1 - Angle/RangeofAngle) + KDB * (1 - distanceToBallProjection/RangeofdistanceToBallProjection ) + KDI * (1 - distanceToIntersect / RangeofdistanceToIntersect);
            temp.second = danger;
            output.append(temp);
        }

    }
    if(!output.isEmpty())
    {

        Segment2D tempsegment;
        tempsegment.assign(output.first().first, wm->field->ourGoal());

        draw(tempsegment, QColor(Qt::blue));
    }





    ///////---------sorting------------//////
    for(int i=0; i<output.count(); i++)
    {

        for(int j=0; j<output.count() - 1; j++)
        {
            if(output[j].second < output[j+1].second)
                output.swap(j,j+1);
        }

    }
    // this is for test
    output.clear();

    return output;
}


QList<CRobot*> CMarkPlan::sortdanger(const QList<CRobot*> oppagent)
{
    QList<CRobot*> sorted = oppagent;
    for(int i=0; i<sorted.count(); i++)
    {

        for(int k = 0; k<sorted.count() - 1; k++)
        {

            if(sorted[k]->danger < sorted[k+1]->danger)
            {
                sorted.swap(k, k+1);
            }

        }

    }
    return sorted;

}

QList<QPair<Vector2D, double> > CMarkPlan::sortdangerpassplayoff(QList<Vector2D> oppposdanger) {



    ///////////////
    double KA=1; //Angle Coefficient
    double KDB=1;  //Distance To Ball
    double KDG=1;  //Distnce To Goal
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(),Vector2D(-1.0 * (_FIELD_WIDTH / 2 - _GOAL_RAD), 0), wm->field->ourGoalL()).degree();
    //draw(Vector2D(-1.0 * (_FIELD_WIDTH - _GOAL_WIDTH), 0), QColor(Qt::red));
    // double RangeofAngle2 = Vector2D::angleOf(wm->field->ou,Vector2D(0, -1.0 * (_FIELD_WIDTH - _GOAL_WIDTH)), wm->field->ourGoalL()).degree();

    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), Vector2D(-1.0 * _FIELD_WIDTH/2,-1.0 * _FIELD_HEIGHT /2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), wm->field->ourGoal()).length());

    //double RangeofTempDis = 2;
    double angle, distancetoball, distancetogoal,danger;


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    for(int i = 0; i<oppposdanger.count(); i++) {
        temp.first = oppposdanger[i];


        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL() ).degree();
        distancetoball =  (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal =  (oppposdanger[i] - wm->field->ourGoal()).length();
        danger = (KA * fabs(angle) / RangeofAngle) + ( KDB * 1 - (distancetoball / RangeofDistancetoBall) ) + (KDG * 1 -(distancetogoal / RangeofDistancetoGoal));
        temp.second = danger;
        output.append(temp);
        // finding nearest to intersect
        Segment2D tempsegment;
        tempsegment.assign(oppposdanger[i],wm->field->ourGoal());

        double mintempdis = 0.0;
        if(wm->our.activeAgentsCount() != 0)
            mintempdis = tempsegment.dist(wm->our.active(0)->pos);

        for(int j=0; j<wm->our.activeAgentsCount(); j++)
        {
            if(tempsegment.dist(wm->our.active(j)->pos) < mintempdis)
            {
                mintempdis = tempsegment.dist(wm->our.active(j)->pos);
            }

        }
    }

    ///sorting the Qlist
    for(int i = 0; i< output.count(); i++)
    {
        for(int j = 0; j< output.count() - 1; j++ )
        {
            if(output[j].second < output[j + 1].second)
                output.swap(j, j+1);
        }
    }

    return output;
}

Vector2D CMarkPlan::findcenterdefence()
{

    Segment2D tempsegment;

    if(knowledge->defenseAgents.count() == 0)
    {
        return Vector2D(-2, 0);

    }

    else if (knowledge->defenseAgents.count() == 1)
    {
        return knowledge->defenseAgents[0]->pos();

    }



    else if(knowledge->defenseAgents.count() == 2)
    {
        tempsegment.assign(knowledge->defenseAgents[0]->pos(),knowledge->defenseAgents[1]->pos());
        return tempsegment.intersection(tempsegment.perpendicularBisector());
    }

    else
    {
        return Vector2D(2,0);
    }

}


QList<QPair<Vector2D, double> > CMarkPlan::sortdangerpassplayon(QList<Vector2D> oppposdanger) {

    double KA=1; //Angle Coefficient
    double KDB=0;  //Distance To Ball
    double KDG=1;  //Distnce To Goal
    double KAD=2;  // angle of difense
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(),Vector2D(-1.0 * (_FIELD_WIDTH / 2 - _GOAL_RAD), 0), wm->field->ourGoalL()).degree();
    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), Vector2D(-1.0 * _FIELD_WIDTH/2,-1.0 * _FIELD_HEIGHT /2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), wm->field->ourGoal()).length());

    double RangeofTempDis = 2;


    double RangeofAngleDef = 180;

    double angle, angleDef, distancetoball, distancetogoal, danger;

    ///finding center of defense


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    for(int i = 0; i<oppposdanger.count(); i++) {
        temp.first = oppposdanger[i];


        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL() ).degree();
        distancetoball =  (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal =  (oppposdanger[i] - wm->field->ourGoal()).length();

        angleDef = Vector2D::angleOf(oppposdanger[i], wm->field->ourGoal(), findcenterdefence()).degree();
        danger = (KA * fabs(angle) / RangeofAngle) + ( KDB * (1 - (distancetoball / RangeofDistancetoBall)) ) + (KDG * (1 -(distancetogoal / RangeofDistancetoGoal))) + (KAD * angleDef / RangeofAngleDef);
        temp.second = danger;
        output.append(temp);
        // finding nearest to intersect
        Segment2D tempsegment;
        tempsegment.assign(oppposdanger[i],wm->field->ourGoal());

        double mintempdis = 0.0;
        if(wm->our.activeAgentsCount() != 0)
            mintempdis = tempsegment.dist(wm->our.active(0)->pos);

        for(int j=0; j<wm->our.activeAgentsCount(); j++)
        {
            if(tempsegment.dist(wm->our.active(j)->pos) < mintempdis)
            {
                mintempdis = tempsegment.dist(wm->our.active(j)->pos);
            }

        }

    }

    ///sorting the Qlist
    for(int i = 0; i< output.count(); i++)
    {
        for(int j = 0; j< output.count() - 1; j++ )
        {
            if(output[j].second < output[j + 1].second)
                output.swap(j, j+1);
        }
    }

    return output;
}


bool CMarkPlan::sortBy(const Vector2D &robot1,const Vector2D &robot2)
{
    double dist2BallW=-1;
    double dist2GoalW=1;
    double thetaW=2;
    AngleDeg thetaMin=15;

    double ballW1 = (dist2BallW/(robot1.dist(wm->ball->pos)));
    double ballW2 = (dist2BallW/(robot2.dist(wm->ball->pos)));
    double goalW1 = (dist2GoalW/(robot1.dist(wm->field->ourGoal())));
    double goalW2 = (dist2GoalW/(robot2.dist(wm->field->ourGoal())));
    double theta1 = (robot1 - wm->field->ourGoal()).th().degree();
    if(fabs(theta1) < thetaMin.degree())
        theta1 = thetaMin.degree();
    double theta2 = (robot2 - wm->field->ourGoal()).th().degree();
    if(fabs(theta2) < thetaMin.degree())
        theta2 = thetaMin.degree();


    if( ballW1  + goalW1 + (thetaW/theta1) > ballW2  + goalW2 + (thetaW/theta2) )
        return true;
    else
        return false;
}

Vector2D CMarkPlan::posvel(CRobot* opp){
    if(opp->vel.length() > 0.5 && opp->vel.x < 0)
        return opp->pos + 0.5 * opp->vel;
    else
        return opp->pos;
}
void CMarkPlan::findOppAgentsToMark()
{
    oppAgentsToMark.clear();
    oppAgentsMarkedByDef.clear();
    ignoredOppAgents.clear();

    for(int i=0; i<wm->opp.activeAgentsCount();i++){
        oppAgentsToMark.append(wm->opp.active(i));
        if(wm->field->isInOppPenaltyArea(oppAgentsToMark.last()->pos))
            oppAgentsToMark.removeOne(oppAgentsToMark.last());   // detection of the goali
    }

    oppAgentsToMarkPos.clear();
    Segment2D tempMarkSeg;
    Vector2D sol1,sol2;

    if(knowledge->getGameState() == CKnowledge::Start)
    {
        for(int i = 0; i < oppAgentsToMark.count(); i++)
        {
            if(oppAgentsToMark[i]->pos.x > policy()->Mark_OppOmitLimitPlayon()){
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                i--;
            }
        }
    }
    else if(knowledge->getGameState() == CKnowledge::TheirKickOff)
    {
        for(int i = 0; i < oppAgentsToMark.count(); i++)
        {
            if(posvel(oppAgentsToMark[i]).x > policy()->Mark_OppOmitLimitKickOff()){
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                i--;
            }
        }

    }

    if(policy()->Mark_OmmitNearestToBallPlayon())
    {
        debug(QString("Ommit nearest to ball"),D_MAHI);
        int nearestToBall = -1;
        double nearestToBallDist = 100000;

        for(int i = 0 ; i < oppAgentsToMark.count() ; i++)
        {
            if((oppAgentsToMark[i]->pos/*+ oppAgentsToMark[i]->vel*/).dist(wm->ball->pos /*+  wm->ball->vel*/) < nearestToBallDist)
            {
                nearestToBall = i;
                nearestToBallDist = oppAgentsToMark[i]->pos.dist(wm->ball->pos);
                debug(QString("the nearest id is:%1").arg(oppAgentsToMark[i]->id),D_MAHI);
                draw(oppAgentsToMark[i]->pos + oppAgentsToMark[i]->vel);
            }
        }
        if(nearestToBall != -1)
            oppAgentsToMark.removeOne(oppAgentsToMark[nearestToBall]);
    }



    for(int i=0; i < oppAgentsToMark.count(); i++)
    {
        oppAgentsToMarkPos.append(oppAgentsToMark[i]->pos);
        draw(Circle2D(oppAgentsToMarkPos.last(),.1),Qt::yellow);
    }


}

void CMarkPlan::markPosesRefinePlayon()
{
    Vector2D sol1, sol2, sol;
    Line2D tempLine(Vector2D(0,0), Vector2D(1,0));
    Circle2D _markAreaRestricted1(wm->field->ourGoal() + Vector2D(-1.0 * 0.2, 0), markRadiusStrict);
    Circle2D _markAreaRestricted2(wm->field->oppGoal() + Vector2D(0.2, 0), markRadiusStrict);
    for(int i = 0; i < markPoses.count(); i++)
    {
        tempLine.assign(wm->field->ourGoal(), markPoses[i]);
        if((wm->field->ourGoal() - markPoses[i]).length() < markRadiusStrict)
        {
            _markAreaRestricted1.intersection(tempLine, &sol1, &sol2);

            if(sol1.x > -1.0 * _FIELD_WIDTH / 2)
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            markPoses[i] = sol;
        }

        tempLine.assign( markPoses[i], wm->field->oppGoal());

        if((markPoses[i] - wm->field->oppGoal()).length() < markRadiusStrict)
        {
            _markAreaRestricted2.intersection(tempLine, &sol1, &sol2);

            if(sol1.x < 1.0 * _FIELD_WIDTH / 2)
            {
                sol = sol1;
            }
            else
            {
                sol = sol2;
            }

            markPoses[i] = sol;
        }
    }

    ///refinning kick off
    double xKickoff  = -.08;
    Circle2D indirect;
    //Vector2D sol1,sol2,sol;
    indirect.assign(Vector2D(0,0),.5);
    Segment2D tempMarkSeg;
    if(knowledge->getGameState() == CKnowledge::TheirKickOff )
    {
        // Not positioning the Center circle
        for(int i=0; i<markPoses.count();i++)
        {

            if(indirect.contains(markPoses[i]))
            {
                tempMarkSeg.assign(markPoses[i], wm->field->ourGoal());
                indirect.intersection(tempMarkSeg,&sol1,&sol2);
                if(sol1.x<sol2.x)
                    sol = sol1;
                else
                    sol = sol2;

                markPoses[i] = sol;
            }
        }
        // not invading the opponent field
        for(int i=0; i<markPoses.count();i++)
        {
            if(markPoses[i].x > xKickoff)
                markPoses[i].x = xKickoff;
        }
    }
}

QList<Vector2D> CMarkPlan::ShootBlockRatio(double ratio, Vector2D opp){
    Segment2D tempMarkSeg;
    QList<Vector2D> tempQlist;
    Circle2D indirect;
    Vector2D sol1,sol2,sol;
    Segment2D sag;
    sag.assign(Vector2D(0,-3) , Vector2D(0,3));
    indirect.assign(Vector2D(0,0),.7);
    tempQlist.clear();
    tempMarkSeg.assign(opp, wm->field->ourGoal());

    if(knowledge->getGameState() == CKnowledge::TheirKickOff ){
        if((opp + (wm->field->ourGoal() - opp) * ratio).x < 0 )
        {
            if(indirect.contains(opp + (wm->field->ourGoal() - opp) * ratio))
            {
                indirect.intersection(tempMarkSeg,&sol1,&sol2);
                if(sol1.x < sol2.x)
                    sol = sol1;
                else
                    sol = sol2;

                tempQlist.append(sol);
                tempQlist.append(opp - wm->field->ourGoal());
            }
            else{
                tempQlist.append(opp + (wm->field->ourGoal() - opp) * ratio);
                tempQlist.append(opp - wm->field->ourGoal());

            }
        }

        else{
            if(indirect.contains(opp + (wm->field->ourGoal() - opp) * ratio))
            {
                indirect.intersection(tempMarkSeg,&sol1,&sol2);
                if(sol1.x<sol2.x)
                    sol = sol1;
                else
                    sol = sol2;

                tempQlist.append(sol);
                tempQlist.append(opp - wm->field->ourGoal());
            }
            else{
                tempQlist.append(tempMarkSeg.intersection(sag));
                tempQlist.append(opp - wm->field->ourGoal());

            }

        }
    }
    else{
        tempQlist.append(opp + (wm->field->ourGoal() - opp) * ratio);
        tempQlist.append(opp - wm->field->ourGoal());
    }

    return tempQlist;
}


void CMarkPlan::execute()
{
    debug(QString(" Mark play on Execute is runninng "), D_MAHI);
    findOppAgentsToMark();
    //sortdangerpass(oppAgentsToMarkPos);

    Circle2D MarkArea(wm->field->ourGoal(),markRadius);
    Circle2D MarkAreaStrict(wm->field->ourGoal(), markRadiusStrict);
    Vector2D sol1,sol2;
    Segment2D tempMarkSeg;
    QList<CAgent*> markers;
    oppmarkedpos.clear();
    QList<CRobot*> temprobot;
    QList<QPair<Vector2D, double> >tempQlistQpair;
    markPoses.clear();
    draw(MarkArea,QColor(Qt::blue));
    markAngs.clear();


    Segment2D temp;         //distance
    int count;
    double mindistance = 9;
    Vector2D nearest;
    double temppos;
    QList<int> matchPoints;
    Circle2D IndirectAvoid(wm->ball->pos, 0.6);

    //////----------HMD Play on Mark-------------////


    if( knowledge->getGameState() == CKnowledge::Start )
    {
        if(policy()->Mark_PlayOnManToMan()){
            manToManMarkInPlayOnBlockPass(oppAgentsToMarkPos , agents.count() , policy()->Mark_PassRatioBlock() / 100);
        }
        else{
            //debug(QString("Play on Marking"), D_MAHI);
            //debug(QString("Agents.cout%1").arg(agents.count()),D_MAHI);
            markPoses.clear();
            markAngs.clear();
            //oppAgentsToMarkPos.clear();
            QList<QPair<Vector2D, double> > OopPosDanger;
            //Circle2D TheirArea(wm->field->oppGoal(), _GOAL_RAD);
            OopPosDanger.clear();

            OopPosDanger = sortdangerpassplayon(oppAgentsToMarkPos);
            // debug(QString("Oppposdamnger %1").arg(agents.count()),D_MAHI);
            //debug(QString("Number of OopPosDanger%1 and %2").arg(oppAgentsToMarkPos.first().x).arg(oppAgentsToMarkPos.last().x),D_MAHI);

            for(int i=0; i<oppAgentsToMarkPos.count(); i++)
            {
                draw(Circle2D(oppAgentsToMarkPos[i],.2),QColor("cyan"));
            }

            QList<QPair<Vector2D, double> > oppPosDangerShoot;
            oppPosDangerShoot.clear();

            oppPosDangerShoot = sortdangershoot(1,0.2);
            Segment2D temp;
            QList<QPair<Vector2D, double>  > temp2; // Qlist of First:MarkAgents     Second:Distance
            QPair<Vector2D, double> temp3;

            for(int i=0; i < oppPosDangerShoot.count(); i++)
            {
                //draw(QString("this is contain%1"),oppPosDangerShoot[i].first + Vector2D(0,.2),QColor(Qt::blue));
                //draw(oppPosDangerShoot[i].first);
            }

            if(agents.count() == 1)
            {
                if(!oppPosDangerShoot.isEmpty())
                {

                    temp.assign(oppPosDangerShoot[0].first,wm->field->ourGoal());


                    //draw(temp, QColor(Qt::blue));

                    for(int j=0; j<agents.count(); j++)
                    {
                        temp3.first = agents[j]->pos();
                        temp3.second = temp.dist(agents[j]->pos());
                        temp2.append(temp3);
                    }


                    /////---- sorting our Robot To find Best one to Go to The Mark Point


                    for(int j=0; j<temp2.count(); j++)
                    {
                        for(int k=0; k<temp2.count() - 1 ; k++)
                        {
                            if(temp2[k].first.length() < temp2[k+1].first.length())
                                temp2.swap(k, k+1);

                        }
                    }




                    markPoses.append(temp.nearestPoint(temp2.last().first));
                    markAngs.append(Vector2D(1,0));
                    draw(QString("Danger"), oppPosDangerShoot[0].first, QColor(Qt::black));
                    draw(QString("Here"), temp.nearestPoint(temp2.last().first));
                }



                else
                {

                    ZoneMark(OopPosDanger, 1);

                }
            }
            else if(agents.count() == 2)
            {
                markPoses.clear();
                markAngs.clear();
                if(!oppPosDangerShoot.isEmpty())
                {

                    temp.assign(oppPosDangerShoot[0].first,wm->field->ourGoal());





                    //draw(temp, QColor(Qt::blue));

                    for(int j=0; j < agents.count(); j++)
                    {
                        temp3.first = agents[j]->pos();
                        temp3.second = temp.dist(agents[j]->pos());
                        temp2.append(temp3);
                    }


                    /////---- sorting our Robot To find Best one to Go to The Mark Point


                    for(int j=0; j < temp2.count(); j++)
                    {
                        for(int k=0; k < temp2.count() - 1 ; k++)
                        {
                            if(temp2[k].first.length() < temp2[k+1].first.length())
                                temp2.swap(k, k+1);

                        }
                    }




                    markPoses.append(temp.nearestPoint(temp2.last().first));
                    markAngs.append(Vector2D(1,0));
                    draw(QString("Danger"), oppPosDangerShoot[0].first, QColor(Qt::black));
                    draw(QString("Here"), temp.nearestPoint(temp2.last().first));
                    ZoneMark(OopPosDanger,1);
                }


                else
                {
                    ZoneMark(OopPosDanger, 2);
                }
            }
            //   draw(QString("temp2.last.first pos is %1").arg(temp2.last().first.absY()), Vector2D(0,0), QColor(Qt::blue));




            for(int i=0; i<agents.count(); i++)
            {
                draw(QString("I am Mark"), agents[i]->pos(),QColor(Qt::black));
            }


            draw(QString("Number of Mark %1").arg(agents.count()), Vector2D(-3,3));
        }


        //Checking Not Going To Penalty Area
        markPosesRefinePlayon();
        matchPoints.clear();
        matchPoints.append(0);
        matchPoints.append(1);
        matchPoints.append(2);
        matchPoints.append(3);

        knowledge->Matching(agents,markPoses,matchPoints);
        if(agents.count() == markPoses.count())
        {
            for(int i =0; i<markPoses.count(); i++)
            {
                if(i < matchPoints.size()) {
                    markGPA[i]->setAgent(agents[i]);
                    markGPA[i]->init(markPoses[matchPoints[i]],markAngs[matchPoints[i]]);
                    markGPA[i]->setAvoidPenaltyArea(1);
                    markGPA[i]->execute();
                }
            }

        }




    }
    else if(knowledge->getGameState() == CKnowledge::TheirKickOff)
    {
        segmentper = policy()->Mark_ShootRatioBlock() / 100;
        markPoses.clear();
        markAngs.clear();
        debug(QString("Its TheirKickoff"),D_MAHI);
        findOppAgentsToMark();
        Segment2D tempsegLine, tempsegLine2, tempsegopp;
        tempsegLine.assign(Vector2D(-2, _FIELD_HEIGHT / 2), Vector2D(-2, -1.0 * _FIELD_HEIGHT / 2 ));
        tempsegLine2.assign(Vector2D(-2, _FIELD_HEIGHT / 2), Vector2D(-2, -1.0 * _FIELD_HEIGHT / 2 ));

        if(agents.count() == oppAgentsToMarkPos.count())
        {
            for(int i = 0; i<oppAgentsToMarkPos.count(); i++)
            {
                markPoses.append(ShootBlockRatio(segmentper, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentper, oppAgentsToMarkPos[i]).last());
            }
        }

        else if(agents.count() < oppAgentsToMarkPos.count())
        {
            QList<QPair<Vector2D, double> >tempQlistQpair = sortdangerpassplayoff(oppAgentsToMarkPos);
            for(int i=0; i<agents.count(); i++)
            {
                markPoses.append(ShootBlockRatio(segmentper, tempQlistQpair[i].first).first());
                markAngs.append(ShootBlockRatio(segmentper, tempQlistQpair[i].first).last());
            }


        }

        else if(agents.count() > oppAgentsToMarkPos.count())
        {

            for(int i=0; i<oppAgentsToMarkPos.count(); i++)
            {
                markPoses.append(ShootBlockRatio(segmentper, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentper, oppAgentsToMarkPos[i]).last());
            }
            for(int i=0; i< agents.count() - oppAgentsToMarkPos.count(); i++)
            {
                markPoses.append(Vector2D(-1,i));
                markAngs.append(Vector2D(-1,0));
                draw(Circle2D(markPoses.last(), 0.2),QColor(Qt::blue) );
            }

        }

        markPosesRefinePlayon();
        wm->markposes = markPoses;

        matchPoints.clear();
        matchPoints.append(0);
        matchPoints.append(1);
        matchPoints.append(2);
        matchPoints.append(3);

        knowledge->Matching(agents, markPoses, matchPoints);
        if(agents.count() == markPoses.count())
        {
            for(int i =0; i<markPoses.count(); i++)
            {
                if(i < matchPoints.size()) {
                    markGPA[i]->setAgent(agents[i]);
                    markGPA[i]->init(markPoses[matchPoints[i]],markAngs[matchPoints[i]]);
                    markGPA[i]->setAvoidPenaltyArea(1);
                    markGPA[i]->setAvoidCenterCircle(1);
                    markGPA[i]->execute();
                }
            }

        }


    }


    return;
}