#include "defenseplan.h"
#include <cmath>
#include <sstream>

using namespace std;

#define LOG(key, value) debug(QString("%1:: %2").arg(key).arg(value), D_MOHAMMED);

#define CHIP_POWER 200
#define LONG_CHIP_POWER 200
#define KICK_POWER 200

float getDegree(Vector2D pos1, Vector2D pos2, Vector2D pos3)
{
    Vector2D v1 = pos1 - pos2, v2 = pos3 - pos2;
    return (v1.th() - v2.th()).degree();
}
///////////////////////////////// AHZ ////////////////////////////////////////
Line2D DefensePlan::getBisectorLine(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint){
    Line2D bisectorLine (originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    return bisectorLine;
}
Segment2D DefensePlan::getBisectorSegment(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint){
    Line2D bisectorLine (originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    Segment2D bisectorSegment(originPoint , Segment2D (thirdPoint , firstPoint).intersection(bisectorLine));
    return bisectorSegment;
}
void DefensePlan::manToManMarkInPlayOffBlockPass(QList<Vector2D> opponentAgentsToBeMarkePossition , int ourMarkAgentsSize , double proportionOfDistance){
    ////////////////////////// Variables of this function //////////////////////
    int i;
    int j;
    Vector2D opponentPlayMakerPossition;
    Vector2D ourCenterOfGoalPossition = wm->field->ourGoal();
    Vector2D ourLeftOfGoalPossition = wm->field->ourGoalL();
    Vector2D ourRightOfGoalPossition = wm->field->ourGoalR();
    Vector2D sol1 , sol2;
    Vector2D sol3 , sol4;
    Vector2D sol5 , sol6;
    Vector2D ballPossition;
    Vector2D ballVelocity;
    Vector2D secondPos;
    Segment2D tempSegment;
    double ballCircleR = 0.5;
    double opponentAgentsCircleR = 0.2;
    QList<Circle2D> opponentAgentsToBeMarkedCircle;
    QList<Circle2D> tempOpponentAgentsToBeMarkedCircle;
    QList<Segment2D> opponentAgents2OpponentPlayMaker;
    QList<Vector2D> tempMarkPoses;
    QList<Vector2D> ourMarkAgentsPossition;
    QList<QPair<Vector2D,double> > sortDangerAgentsToBeBlockPassPlayOff;
    QList<QPair<Vector2D,double> > sortDangerAgentsToBeBlockShotPlayOff;
    QList<QPair<Vector2D,double> > tempSortDangerAgentsToBeBlockPassPlayOff;
    QList<QPair<Vector2D,double> > sortDangerAgentsToBeBlockPassPlayOn;
    Circle2D opponentPlayMakerCircle;
    Circle2D goalCircle(ourCenterOfGoalPossition , 1.43);
    Segment2D opponentPlayMaker2OurGoal;
    //////////////////// Clear QLists for update the states ////////////////////
    ourMarkAgentsPossition.clear();
    markPoses.clear();
    markAngs.clear();
    ///////// Make Cirlcles around opponent agents /////////////////////////////
    for(i = 0 ; i < opponentAgentsToBeMarkePossition.size() ; i++){
        opponentAgentsToBeMarkedCircle.append(Circle2D(opponentAgentsToBeMarkePossition.at(i) , opponentAgentsCircleR));
        draw(opponentAgentsToBeMarkedCircle.at(i),QColor(Qt::cyan));
    }
    debug(QString("Mark Agents Count : %1").arg(ourMarkAgentsSize) , D_SEPEHR , QColor(Qt::red));
    debug(QString("QList : %1").arg(opponentAgentsToBeMarkePossition.size()) , D_SEPEHR , QColor(Qt::red));
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
        tempSortDangerAgentsToBeBlockPassPlayOff = sortdangerpassplayoff(ourMarkAgentsPossition);
        for(i = 0 ; i < tempSortDangerAgentsToBeBlockPassPlayOff.size() ; i++){
            tempOpponentAgentsToBeMarkedCircle.append(Circle2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , opponentAgentsCircleR));
            draw(tempOpponentAgentsToBeMarkedCircle.at(i),QColor(Qt::cyan));
        }

        for(i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++){
            tempOpponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
            Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol3 , &sol4);
            tempMarkPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                     Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4,
                                                     proportionOfDistance));
            markAngs.append(wm->ball->pos - tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first);
            if(wm->field->isInOurPenaltyArea(tempMarkPoses.at(i))){
                debug("AHZ" , D_SEPEHR);
                if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first)){
                    goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                      Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);
                    tempMarkPoses.removeAt(i);
                    tempMarkPoses.append(getPointInDirection(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first
                                                             ,Segment2D(sol5 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol6 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol5 : sol6
                                                                                                                                                                                                                                  , proportionOfDistance));
                }
                else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first)){
                    goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                      Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);
                    tempMarkPoses.removeAt(i);
                    tempMarkPoses.append(getPointInDirection(wm->ball->pos
                                                             ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                    , proportionOfDistance));
                }
                else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first)){
                    goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                      Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);


                    tempMarkPoses.removeAt(i);
                    tempMarkPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                }
                else{
                    goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                      Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);

                    if(isValidPoint(sol5) && isValidPoint(sol6)){
                        if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                                Segment2D(Segment2D(sol5 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol6 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol5:sol6 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length()){
                            tempMarkPoses.removeAt(i);
                            draw(Segment2D(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            tempMarkPoses.append(getPointInDirection(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            markAngs.append(wm->ball->pos - tempMarkPoses.at(i));
                        }

                        else{
                            tempMarkPoses.removeAt(i);
                            tempMarkPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol6 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol5:sol6 , proportionOfDistance));
                            markAngs.append(wm->ball->pos - tempMarkPoses.at(i));
                        }
                    }
                    else{
                        tempMarkPoses.removeAt(i);
                        draw(Segment2D(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                        tempMarkPoses.append(getPointInDirection(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                        markAngs.append(wm->ball->pos - tempMarkPoses.at(i));
                    }
                }
            }


        }
        markPoses.append(tempMarkPoses);
        debug(QString("s :%1").arg(markPoses.size()) , D_SEPEHR);
        debug(QString("coach :%1").arg(ourMarkAgentsSize) , D_SEPEHR);


    }

    else{

        if(opponentAgentsToBeMarkePossition.size() == ourMarkAgentsSize){
            for(i = 0 ; i < opponentAgentsToBeMarkePossition.size() ; i++){
                opponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(opponentAgentsToBeMarkePossition.at(i) , wm->ball->pos) , &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(opponentAgentsToBeMarkePossition.at(i) , wm->ball->pos) , &sol3 , &sol4);
                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                     Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4,
                                                     proportionOfDistance));
                //////////////////// check if the mark agents possition is in ourPenaltyArea or not ////////////////////////////////////////////////////////////
                if(wm->field->isInOurPenaltyArea(markPoses.at(i))){
                    if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.removeAt(i);
                        markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                             ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                      , proportionOfDistance));
                    }
                    else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.removeAt(i);
                        markPoses.append(getPointInDirection(wm->ball->pos
                                                             ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                    , proportionOfDistance));
                    }
                    else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                        markPoses.removeAt(i);
                        markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                    }
                    else{
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                                markPoses.removeAt(i);
                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            }
                            else{
                                markPoses.removeAt(i);
                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            }
                        }
                        else{
                            markPoses.removeAt(i);
                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            markAngs.append(wm->ball->pos - markPoses.at(i));
                        }
                    }
                }
                markAngs.append(wm->ball->pos - markPoses.at(i));
                draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                               Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , QColor(Qt::red));
                draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                draw(goalCircle,QColor(Qt::black));
                debug(QString("Man To Man Mark In PlayOff Mode / BlockPass / our = opp") , D_SEPEHR);
            }
        }


        else if(opponentAgentsToBeMarkePossition.size() < ourMarkAgentsSize){
            debug(QString("Mark Agents Count : %1").arg(opponentAgentsToBeMarkePossition.size()) , D_SEPEHR , QColor(Qt::green));
            for(i = 0 ; i < opponentAgentsToBeMarkePossition.size() ; i++){
                opponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkePossition.at(i))
                                                                  , &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkePossition.at(i))
                                                                   , &sol3 , &sol4);
                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2
                                                                                                                                           ,Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4
                                                                                                                                                                                                                                                                                    , proportionOfDistance));
                //////////////////// check if the mark agents possition is in ourPenaltyArea or not ////////////////////////////////////////////////////////////
                if(wm->field->isInOurPenaltyArea(markPoses.at(i))){
                    if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.removeAt(i);
                        markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                             ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                      , proportionOfDistance));
                    }
                    else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.removeAt(i);
                        markPoses.append(getPointInDirection(wm->ball->pos
                                                             ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                    , proportionOfDistance));
                    }
                    else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                        markPoses.removeAt(i);
                        markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                    }
                    else{
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                                markPoses.removeAt(i);
                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            }
                            else{
                                markPoses.removeAt(i);
                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            }
                        }
                        else{
                            markPoses.removeAt(i);
                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            debug(QString("boo miad") , D_SEPEHR);
                            markAngs.append(wm->ball->pos - markPoses.at(i));
                        }
                    }
                }
                markAngs.append(wm->ball->pos - markPoses.at(i));
                draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2
                                                                                                                     ,Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4)
                     , QColor(Qt::red));
                draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                debug(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp < our") , D_SEPEHR);
            }
            goalCircle.intersection(Segment2D(goalCircle.center() , wm->ball->pos) , &sol1 , &sol2);
            Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(goalCircle.center() , wm->ball->pos) , &sol3 , &sol4);
            markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2
                                                                                                                                       ,Segment2D(sol3 , goalCircle.center()).length() < Segment2D(sol4 , goalCircle.center()).length() ? sol3 : sol4                                                                                                                                                                                                                                          ,proportionOfDistance));
            markAngs.append(wm->ball->pos - markPoses.last());




            if(ourMarkAgentsSize > markPoses.size()){
                debug("sag zede" , D_SEPEHR);
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
                tempSortDangerAgentsToBeBlockPassPlayOff = sortdangerpassplayoff(ourMarkAgentsPossition);
                for(i = 0 ; i < tempSortDangerAgentsToBeBlockPassPlayOff.size() ; i++){
                    tempOpponentAgentsToBeMarkedCircle.append(Circle2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , opponentAgentsCircleR));
                    draw(tempOpponentAgentsToBeMarkedCircle.at(i),QColor(Qt::cyan));
                }

                for(i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++){
                    tempOpponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol3 , &sol4);
                    tempMarkPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                             Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4,
                                                             proportionOfDistance));
                    markAngs.append(wm->ball->pos - tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first);

                    if(wm->field->isInOurPenaltyArea(tempMarkPoses.at(i))){
                        debug("AHZ" , D_SEPEHR);
                        if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first)){
                            goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                              Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);
                            tempMarkPoses.removeAt(i);
                            tempMarkPoses.append(getPointInDirection(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first
                                                                     ,Segment2D(sol5 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol6 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol5 : sol6
                                                                                                                                                                                                                                          , proportionOfDistance));
                        }
                        else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first)){
                            goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                              Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);
                            tempMarkPoses.removeAt(i);
                            tempMarkPoses.append(getPointInDirection(wm->ball->pos
                                                                     ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                            , proportionOfDistance));
                        }
                        else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first)){
                            goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                              Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                            tempMarkPoses.removeAt(i);
                            tempMarkPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                        }
                        else{
                            goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                              Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);

                            if(isValidPoint(sol5) && isValidPoint(sol6)){
                                if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                                        Segment2D(Segment2D(sol5 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol6 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol5:sol6 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length()){
                                    tempMarkPoses.removeAt(i);
                                    draw(Segment2D(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                                    tempMarkPoses.append(getPointInDirection(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                                    markAngs.append(wm->ball->pos - tempMarkPoses.at(i));
                                }

                                else{
                                    tempMarkPoses.removeAt(i);
                                    tempMarkPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol6 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol5:sol6 , proportionOfDistance));
                                    markAngs.append(wm->ball->pos - tempMarkPoses.at(i));
                                }
                            }
                            else{
                                tempMarkPoses.removeAt(i);
                                draw(Segment2D(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                                tempMarkPoses.append(getPointInDirection(Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                                markAngs.append(wm->ball->pos - tempMarkPoses.at(i));
                            }
                        }
                    }


                }
                markPoses.append(tempMarkPoses);

                debug(QString("s :%1").arg(markPoses.size()) , D_SEPEHR);
                debug(QString("coach :%1").arg(ourMarkAgentsSize) , D_SEPEHR);
            }



        }


        else if(opponentAgentsToBeMarkePossition.size() > ourMarkAgentsSize){
            sortDangerAgentsToBeBlockPassPlayOff = sortdangerpassplayoff(opponentAgentsToBeMarkePossition);
            sortDangerAgentsToBeBlockPassPlayOff.removeLast();
            for(i = 0 ; i < sortDangerAgentsToBeBlockPassPlayOff.size() ; i++){
                Circle2D(sortDangerAgentsToBeBlockPassPlayOff.at(i).first , opponentAgentsCircleR)
                        .intersection(Segment2D(sortDangerAgentsToBeBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR)
                        .intersection(Segment2D(sortDangerAgentsToBeBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol3 , &sol4);
                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                     Segment2D(sol3 , sortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4,
                                                     proportionOfDistance));
                //////////////////// check if the mark agents possition is in ourPenaltyArea or not ////////////////////////////////////////////////////////////
                if(wm->field->isInOurPenaltyArea(markPoses.at(i))){

                    if(wm->field->isInOurPenaltyArea(wm->ball->pos) && !wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.removeAt(i);
                        markPoses.append(getPointInDirection(opponentAgentsToBeMarkePossition.at(i)
                                                             ,Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5 : sol6
                                                                                                                                                                                                      , proportionOfDistance));

                    }
                    else if(!wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.removeAt(i);
                        markPoses.append(getPointInDirection(wm->ball->pos
                                                             ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 : sol6
                                                                                                                                                    , proportionOfDistance));

                    }
                    else if(wm->field->isInOurPenaltyArea(wm->ball->pos) && wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkePossition.at(i))){
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4).perpendicularBisector() , &sol5 , &sol6);

                        markPoses.removeAt(i);
                        debug(QString("gaeedam") , D_SEPEHR);
                        markPoses.append(wm->field->isInField(sol5) ? sol5 : sol6);
                    }
                    else{
                        goalCircle.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                                                          Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);

                        if(isValidPoint(sol5) && isValidPoint(sol6)){
                            debug(QString("hastan") , D_SEPEHR);
                            if(Segment2D(Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , wm->ball->pos).length() >=
                                    Segment2D(Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , opponentAgentsToBeMarkePossition.at(i)).length()){
                                markPoses.removeAt(i);
                                draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                                markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                                debug(QString("boo miad") , D_SEPEHR);
                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            }
                            else{
                                markPoses.removeAt(i);
                                debug(QString("sag zade") , D_SEPEHR);
                                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 , Segment2D(sol5 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol5:sol6 , proportionOfDistance));
                                markAngs.append(wm->ball->pos - markPoses.at(i));
                            }
                        }
                        else{
                            markPoses.removeAt(i);
                            draw(Segment2D(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6),QColor(Qt::green));
                            markPoses.append(getPointInDirection(Segment2D(sol3 , opponentAgentsToBeMarkePossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkePossition.at(i)).length() ? sol3 : sol4 , Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5:sol6 , proportionOfDistance));
                            debug(QString("boo miad") , D_SEPEHR);
                            markAngs.append(wm->ball->pos - markPoses.at(i));
                        }
                    }
                }
                markAngs.append(wm->ball->pos - markPoses.at(i));

                draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                               Segment2D(sol3 , sortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3 : sol4)
                     , QColor(Qt::red));
                draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                debug(QString("MostDanger = %1").arg(sortDangerAgentsToBeBlockPassPlayOff.at(0).second) ,D_SEPEHR , QColor("red"));
                debug(QString("opponentAgentsToBeMarkePossitionSize = %1").arg(opponentAgentsToBeMarkePossition.size()) ,D_SEPEHR , QColor("blue"));
                debug(QString("MarkAngsSize = %1").arg(markAngs.size()) ,D_SEPEHR , QColor("red"));
                debug(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp > our") , D_SEPEHR);
            }

        }
    }
    //////////////// Draw Possition of Mark Agents //////////////////////////
    for(i = 0 ; i < markPoses.size() ; i++){
        if(markPoses.at(i).x > 100){
            markPoses.removeAt(i);
            markPoses.append(Vector2D(0,0));
        }
    }

    for(i = 0 ; i < markPoses.size() ; i++){
        draw(markPoses.at(i),1,QColor(Qt::red));
    }
}

Vector2D DefensePlan::getPointInDirection(Vector2D firstPoint , Vector2D secondPoint , double proportion){
    firstPoint = firstPoint + (secondPoint - firstPoint).norm() * proportion * (Segment2D(secondPoint , firstPoint).length());
    return firstPoint;
}
/////////////////////////////// end of AHZ /////////////////////////////////////
void DefensePlan::tempFindPos(int _markAgentSize){
    bool playOn = knowledge->getGameMode() == CKnowledge::Start;
    bool playOff = ((knowledge->getGameState() == CKnowledge::TheirDirectKick)
                    || (knowledge->getGameState() == CKnowledge::TheirKickOff)
                    || (knowledge->getGameState() == CKnowledge::TheirIndirectKick)
                    );

    Circle2D MarkArea(wm->field->ourGoal(),markRadius);
    Circle2D MarkAreaStrict(wm->field->ourGoal(), markRadiusStrict);
    Vector2D sol1,sol2;
    Segment2D tempMarkSeg;
    oppmarkedpos.clear();
    markPoses.clear();
    draw(MarkArea,QColor(Qt::blue));
    markAngs.clear();

    Segment2D temp;         //distance
    int count;
    double mindistance = 9;
    Vector2D nearest;
    Circle2D IndirectAvoid(wm->ball->pos, 0.6);

    debug(QString(" This is play off"), D_MAHI);


    if(_markAgentSize == oppAgentsToMarkPos.count())
    {

        draw(QString("first if"), Vector2D(0,0),QColor(Qt::red));
        ///shoot block
        for(int i=0; i < oppAgentsToMarkPos.count(); i++)
        {


            if(!checkIndirectAreaShoot(oppAgentsToMarkPos[i]))
            {
                markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
            }
            else
            {
                markPoses.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).first());
                markAngs.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).last());
            }


        }

    }

    else if(_markAgentSize > oppAgentsToMarkPos.count() && (_markAgentSize - oppAgentsToMarkPos.count()) <= oppAgentsToMarkPos.count())
    {
        draw(QString("second if"), Vector2D(0,0),QColor(Qt::red));

        for(int i=0; i < _markAgentSize - oppAgentsToMarkPos.count(); i++)
        {


            if(!checkIndirectAreaPass(oppAgentsToMarkPos[i]))
            {
                markPoses.append(PassBlockRatio(segmentperpass, oppAgentsToMarkPos[i]).first());
                markAngs.append(PassBlockRatio(segmentperpass, oppAgentsToMarkPos[i]).last());
            }
            else
            {
                markPoses.append(indirectAvoidPass(oppAgentsToMarkPos[i]).first());
                markAngs.append(indirectAvoidPass(oppAgentsToMarkPos[i]).last());
            }


        }
        // Blocking direct shoot

        for(int i =0;i < oppAgentsToMarkPos.count();i++)
        {

            if(!checkIndirectAreaShoot(oppAgentsToMarkPos[i]))
            {
                markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
            }
            else
            {
                markPoses.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).first());
                markAngs.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).last());
            }
        }
        // Blocking pass which is marked by defence
        for(int i=0;i<oppAgentsMarkedByDef.count();i++)
        {

            if(!checkIndirectAreaPass(oppAgentsToMarkPos[i]))
            {
                markPoses.append(PassBlockRatio(segmentperpass, oppAgentsToMarkPos[i]).first());
                markAngs.append(PassBlockRatio(segmentperpass, oppAgentsToMarkPos[i]).last());
            }
            else
            {
                markPoses.append(indirectAvoidPass(oppAgentsToMarkPos[i]).first());
                markAngs.append(indirectAvoidPass(oppAgentsToMarkPos[i]).last());
            }
        }

        //extra robot we have

        if(_markAgentSize > markPoses.count())
        {

            for(int i=markPoses.count() ; i<_markAgentSize; i++)
            {

                if(markPoses.count() < _markAgentSize)
                {

                    markPoses.append(Vector2D(0,  count ));
                    markAngs.append(Vector2D(1,0));
                }
                count++;
            }

        }

    }
    else if((_markAgentSize > oppAgentsToMarkPos.count()) &&  ((_markAgentSize - oppAgentsToMarkPos.count()) > oppAgentsToMarkPos.count()))
    {

        draw(QString("third if"), Vector2D(0,0),QColor(Qt::red));


        //Blocking Shoot
        for(int i =0; i < oppAgentsToMarkPos.count();i++)
        {


            if(!checkIndirectAreaShoot(oppAgentsToMarkPos[i]))
            {
                markPoses.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).first());
                markAngs.append(ShootBlockRatio(segmentpershoot, oppAgentsToMarkPos[i]).last());
            }
            else
            {
                markPoses.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).first());
                markAngs.append(indirectAvoidShoot(oppAgentsToMarkPos[i]).last());
            }


        }


        //blocking pass

        for(int i = 0; i< oppAgentsToMark.count();i++)
        {

            if(!checkIndirectAreaPass(oppAgentsToMarkPos[i]))
            {
                markPoses.append(PassBlockRatio(segmentperpass, oppAgentsToMarkPos[i]).first());
                markAngs.append(PassBlockRatio(segmentperpass, oppAgentsToMarkPos[i]).last());
            }
            else
            {
                markPoses.append(indirectAvoidPass(oppAgentsToMarkPos[i]).first());
                markAngs.append(indirectAvoidPass(oppAgentsToMarkPos[i]).last());
            }
        }


        //  blocking pass to which marked by def


        if(oppAgentsMarkedByDef.count() > 0 && _markAgentSize > markPoses.count())
        {



            for(int i = 0; i< oppAgentsMarkedByDef.count();i++)
            {
                if(!checkIndirectAreaPass(oppAgentsToMarkPos[i]))
                {
                    markPoses.append(PassBlockRatio(segmentperpass, oppAgentsToMarkPos[i]).first());
                    markAngs.append(PassBlockRatio(segmentperpass, oppAgentsToMarkPos[i]).last());
                }
                else
                {
                    markPoses.append(indirectAvoidPass(oppAgentsToMarkPos[i]).first());
                    markAngs.append(indirectAvoidPass(oppAgentsToMarkPos[i]).last());
                }

            }
        }


        //now here we have extra robots


        if(_markAgentSize > markPoses.count())
        {

            debug(QString("the number of our agent is %1").arg(_markAgentSize),D_MAHI);

            count = 0;
            for(int i=markPoses.count() ; i < _markAgentSize; i++)
            {
                markPoses.append(Vector2D(0, count));
                markAngs.append(Vector2D(1,0));
                count++;
            }
        }
    }


    else if(_markAgentSize < oppAgentsToMarkPos.count())
    {


        draw(QString("Last Condition"));
        if(playOff || knowledge->transientFlag)
        {
            QList<QPair<Vector2D, double> > tempsorted = sortdangerpassplayoff(oppAgentsToMarkPos);

            for(int i = 0; i<_markAgentSize; i++)
            {

                if(!checkIndirectAreaShoot(tempsorted[i].first))
                {
                    markPoses.append(ShootBlockRatio(segmentpershoot, tempsorted[i].first).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, tempsorted[i].first).last());
                }
                else
                {
                    markPoses.append(indirectAvoidShoot(tempsorted[i].first).first());
                    markAngs.append(indirectAvoidShoot(tempsorted[i].first).last());
                }

            }
        }
        else if (playOn)
        {
            QList<QPair<Vector2D, double> > tempsorted = sortdangerpassplayon(oppAgentsToMarkPos);
            for(int i = 0; i<_markAgentSize; i++)
            {

                if(!checkIndirectAreaShoot(tempsorted[i].first))
                {
                    markPoses.append(ShootBlockRatio(segmentpershoot, tempsorted[i].first).first());
                    markAngs.append(ShootBlockRatio(segmentpershoot, tempsorted[i].first).last());
                }
                else
                {
                    markPoses.append(indirectAvoidShoot(tempsorted[i].first).first());
                    markAngs.append(indirectAvoidShoot(tempsorted[i].first).last());
                }

            }

        }

    }

    //markPosRefinePlayoff();
    markExecute(_markAgentSize);
}

void DefensePlan::markExecute(int _markAgentSize)
{

    QList<int> matchPoints;

    matchPoints.clear();
    matchPoints.append(0);
    matchPoints.append(1);
    matchPoints.append(2);
    matchPoints.append(3);
    debug(QString("Number of agents %1").arg(_markAgentSize), D_MAHI);
    //    knowledge->Matching(agents,markPoses,matchPoints);
    if(_markAgentSize == markPoses.count())
    {
        for(int i =0;i<markPoses.count(); i++)
        {
            if(i < matchPoints.size()) {
                draw(Circle2D(markPoses[i], 0.1), QColor(Qt::white), true);
                debug(QString("%1 : x : %2, y : %3").arg(i).arg(markPoses[i].x).arg(markPoses[i].y),D_MAHI);
            }
        }
    }
}

void DefensePlan::checkGoalieState()
{
    bool playOn = knowledge->getGameMode() == CKnowledge::Start;
    Rect2D fieldRect(Vector2D(- _FIELD_WIDTH/2.0 , - _FIELD_HEIGHT/2.0) - Vector2D(-0.02,-0.02),Vector2D(_FIELD_WIDTH/2.0 , _FIELD_HEIGHT/2.0)-Vector2D(+0.02,+0.02));

    ///////////////////////////Check if goalie is in filed or nor////// else this func dont work//////////
    if(knowledge->goalie != NULL) {
        ////////////////basic parameters for goalie//////////////////////////
        Vector2D BallPos = wm->ball->pos;
        ////////Check if ball is in field or not/////////////////////else this func return center of goal//////////
        if(fieldRect.contains(BallPos)) {
            Vector2D BallVel = wm->ball->vel;
            Vector2D goaliePos = knowledge->goalie->pos();
            Vector2D Solutions[2];
            Circle2D penaltyArea(wm->field->ourGoal() - Vector2D(0.2,0) , 1.33);
            Segment2D ballLine(BallPos, BallPos + BallVel.norm()*10);
            Segment2D goalLine(wm->field->ourGoal() + Vector2D(0 , 0.7) , wm->field->ourGoal() - Vector2D(0 , 0.7));
            penaltyArea.intersection(ballLine, &Solutions[0], &Solutions[1]);
            QList<Circle2D> defs;
            double AZBisecOpenAngle=0,AZBigestOpenAngle=0,AZDangerPercent=0;
            for (int g=0;g<defenseAgents.count();g++)
                defs.append(Circle2D(defenseAgents[g]->pos(), CRobot::robot_radius_new));
            knowledge->getEmptyAngle(BallPos, wm->field->ourGoalL(), wm->field->ourGoalR(), defs, AZDangerPercent, AZBisecOpenAngle, AZBigestOpenAngle,false);
            Segment2D AZTopOfOpenSeg(BallPos , BallPos + Vector2D(cos(_PI*(AZBisecOpenAngle + (AZBigestOpenAngle/2))/180),sin(_PI*(AZBisecOpenAngle + (AZBigestOpenAngle/2))/180)).norm()*12);
            Segment2D AZBottomOfOpenSeg(BallPos , BallPos + Vector2D(cos(_PI*(AZBisecOpenAngle - (AZBigestOpenAngle/2))/180),sin(_PI*(AZBisecOpenAngle - (AZBigestOpenAngle/2))/180)).norm()*12);
            Vector2D openAngGoalIntersectionTop(AZTopOfOpenSeg.intersection(goalLine));
            Vector2D openAngGoalIntersectionBottom(AZBottomOfOpenSeg.intersection(goalLine));

            ///////////////////////////////////////////////
            ///////Check if ball is behind goalie or not////////////else we should check other states
            if(goaliePos.x + behindBallThr > BallPos.x && (penaltyArea.contains(BallPos) || !penaltyArea.contains(goaliePos))) {
                ballBehindGoalie = true ;
                goalieOneTouch = false, goalieInPenaltyAreaPrediction = false, goalieClearMode = false, goalieStrictFollow = false, goalieFollow = false, ballIsOutOfField = false;
                behindBallThr = 0.08;
                strictfollowThr = 0;
                return;
            }
            else if(playOn && ((knowledge->getRealBallVel() > 0.25 || goalieOneTouch)) && (goalLine.intersection(ballLine).valid() || oneTouchCnt < 5)) {
                ballBehindGoalie = false, goalieOneTouch = true, goalieInPenaltyAreaPrediction = false, goalieClearMode = false, goalieStrictFollow = false, goalieFollow = false, ballIsOutOfField = false;
                behindBallThr = 0;
                strictfollowThr = 0;
                if(!goalLine.intersection(ballLine).valid()) {
                    oneTouchCnt++;
                    return;
                }
                oneTouchCnt = 0;
                return;
            }
            else if(playOn && ((knowledge->getRealBallVel() > 0.6 && wm->field->isInField(Solutions[0]) && wm->field->isInField(Solutions[1])) || (goalieInPenaltyAreaPrediction && (wm->field->isInField(Solutions[0]) || wm->field->isInField(Solutions[1]))))){
                ballBehindGoalie = false, goalieOneTouch = false, goalieInPenaltyAreaPrediction = true, goalieClearMode = false, goalieStrictFollow = false, goalieFollow = false, ballIsOutOfField = false;
                behindBallThr = 0;
                strictfollowThr = 0;
                return;
            }
            else if(playOn && ((( knowledge->getRealBallVel() < 0.5) || goalieClearMode ) && penaltyArea.contains(BallPos))) {
                ballBehindGoalie = false, goalieOneTouch = false, goalieInPenaltyAreaPrediction = false, goalieClearMode = true, goalieStrictFollow = false, goalieFollow = false, ballIsOutOfField = false;
                behindBallThr = 0;
                strictfollowThr = 0;
                return;
            }

            else if (openAngGoalIntersectionTop.dist(openAngGoalIntersectionBottom) > 0.6 - strictfollowThr || defenseAgents.size() == 1) {
                ballBehindGoalie = false, goalieOneTouch = false, goalieInPenaltyAreaPrediction = false, goalieClearMode = false, goalieStrictFollow = true, goalieFollow = false, ballIsOutOfField = false;
                strictfollowThr = 0.15;
                behindBallThr = 0;
                return;
            }
            else {
                ballBehindGoalie = false, goalieOneTouch = false, goalieInPenaltyAreaPrediction = false, goalieClearMode = false, goalieStrictFollow = false, goalieFollow = true, ballIsOutOfField = false;
                strictfollowThr = 0;
                behindBallThr = 0;
                return;
            }
        }
        else {
            ballBehindGoalie = false, goalieOneTouch = false, goalieInPenaltyAreaPrediction = false, goalieClearMode = false, goalieStrictFollow = false, goalieFollow = false, ballIsOutOfField = true;
            strictfollowThr = 0;
            behindBallThr = 0;
            return;
        }
    }
}

void DefensePlan::runGoalie()
{
    ///////////////////////////// Variables of this function ////////////////////////////////
    Vector2D ballPos;
    Vector2D ballVel;
    Vector2D predictedBall;
    Vector2D goaliePos;
    Vector2D Solutions[2];
    bool playOnMode = knowledge->getGameMode() == CKnowledge::Start;
    bool stopMode = knowledge->getGameMode() == CKnowledge::Stop;
    ///////////////////////////////////////////////////////////////////////////////////////
    if(knowledge->goalie != NULL){
        Rect2D ourLeftPole(wm->field->ourGoalL() + Vector2D(0.2 , 0.1) , wm->field->ourGoalL() - Vector2D(0 , 0.1));
        Rect2D ourRightPole(wm->field->ourGoalR() + Vector2D(0.2 , 0.1) , wm->field->ourGoalR() - Vector2D(0 , 0.1));
        ballPos = wm->ball->pos;
        ballVel = wm->ball->vel;
        predictedBall = ballPos + ballVel;
        goaliePos = knowledge->goalie->pos();
        Circle2D penaltyArea(wm->field->ourGoal() - Vector2D(0.3,0),1.28);
        Segment2D ballLine(ballPos, ballPos + ballVel.norm()*10);
        draw(ballLine);
        Line2D robotPrGoalLine(goaliePos, Vector2D(goaliePos.x,(goaliePos.y + 0.01)));
        besidePoleFlag = false;
        ///////////////oneTouchCoef is coeficiant of the dist between goaliepos and onetouch target that results to fast movement
        double oneTouchCoef = 0.5;
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if(stopMode){
            debug(QString("stopMode"),D_SEPEHR);
            //////////////////////////// AHZ ////////////////////////
            ballPos = wm->ball->pos;
            if(Vector2D::angleOf(ballPos,wm->field->ourGoal(),wm->field->ourGoalL()).degree() < 20 + angleDegreeThr || Vector2D::angleOf(ballPos,wm->field->ourGoal(),wm->field->ourGoalR()).degree() < 20+  angleDegreeThr){
                debug(QString("yess"),D_SEPEHR);
                angleDegreeThr = 5;
                goalieTarget = wm->field->ourGoal() + Vector2D(0.1,0);
            }
            else{
                angleDegreeThr = 0;
                goalieTarget = strictFollowBall(ballPos);
            }
            //////////////////////////////////////////////////////
            return;
        }
        if(ballIsOutOfField){
            dangerForGoalieClear = false;

            draw(QString("Ball Is Out Of Field"), Vector2D(0,1),"red");
            goalieTarget =wm->field->ourGoal() + Vector2D(0.1, 0);
            return;
        }
        else if (ballBehindGoalie){
            dangerForGoalieClear = false;

            draw(QString("Ball Is Behind The Goalie"), Vector2D(0,1),"red");
            return;
            ////////////Handle this state in executeGoalie() Func//////
        }
        else if (goalieOneTouch){
            dangerForGoalieClear = false;

            ////////////////////////////when nearest point to goalie position on ball line is
            ///////////////////////////far from ball position in comparison with horizental line beside goalie
            if(ballLine.nearestPoint(goaliePos).x < ballLine.intersection(robotPrGoalLine).x) {
                goalieTarget = ballLine.nearestPoint(goaliePos);
                goalieTarget = (oneTouchCoef+1)*goalieTarget - (oneTouchCoef)*goaliePos;
                draw(QString("OneTouch To Nearest Point"), Vector2D(0,1),"red");

            }

            else {
                goalieTarget = ballLine.intersection(robotPrGoalLine);
                goalieTarget = (oneTouchCoef+1)*goalieTarget - (oneTouchCoef)*goaliePos;
                draw(QString("OneTouch To Side Point"), Vector2D(0,1),"red");
            }
            return;
        }
        else if (goalieInPenaltyAreaPrediction && penaltyArea.contains(ballPos)) {
            dangerForGoalieClear = false;
            if (knowledge->getRealBallVel() > 2 && wm->opp.activeAgentsCount() > 0 && predictMostDangrousOppToBall() != 100){
                goalieTarget = followBall(wm->opp[predictMostDangrousOppToBall()]->pos);
                //goalieTarget = strictFollowBall(wm->opp[predictMostDangrousOppToBall()]->pos);
                draw(QString("Pass ball through penalty area using predict"), Vector2D(0,1),"red");

            }
            else {
                penaltyArea.intersection(ballLine,&Solutions[0],&Solutions[1]);
                goalieTarget = (Solutions[0].valid() && wm->field->isInField(Solutions[0])) ? followBall(Solutions[0]) : followBall(Solutions[1]);
                //goalieTarget = (Solutions[0].valid() && wm->field->isInField(Solutions[0])) ? strictFollowBall(Solutions[0]) : strictFollowBall(Solutions[1]);
                draw(QString("Pass ball through penalty area"), Vector2D(0,1),"red");
            }
            return;
        }
        else if (goalieClearMode && playOnMode ) {
            savedClearPos = findBestPointForChipTarget(savedClearDist,1);
            Segment2D ball2ClearTarget(savedClearPos , ballPos);
            Circle2D ballCircle(ballPos, 0.25);
            dangerForGoalieClear = false;
            debug(QString("play on"),D_SEPEHR);
            if(wm->our.activeAgentsCount() > 0){
                for (int i = 0; i < wm->our.activeAgentsCount(); i++) {
                    if(wm->our.active(i)->id != knowledge->goalie->id()) {
                        if (ballCircle.contains(wm->our.active(i)->pos) && ball2ClearTarget.dist(wm->our.active(i)->pos) < 0.15) {
                            dangerForGoalieClear = true;
                        }
                    }
                }
            }
            if(wm->opp.activeAgentsCount() > 0){
                for (int i = 0; i < wm->opp.activeAgentsCount(); i++){
                    if(ballCircle.contains(wm->opp.active(i)->pos) && ball2ClearTarget.dist(wm->opp.active(i)->pos) < 0.1) {
                        dangerForGoalieClear = true;
                    }
                }
            }
            if(dangerForGoalieClear){
                Ray2D goalie2Ball(goaliePos , ballPos);
                penaltyArea.intersection(goalie2Ball,&Solutions[0],&Solutions[1]);
                goalieTarget = (Solutions[0].valid() && wm->field->isInField(Solutions[0]) && Solutions[0].dist(ballPos) < Solutions[1].dist(ballPos) ) ? (Solutions[0]) : (Solutions[1]);
                goalieTarget = goalieTarget - (ballPos - goalieTarget).norm() * 0.09;
                draw(QString("Danger to Clear"), Vector2D(0,1),"red");
            }
            else if(ourLeftPole.contains(ballPos) || ourRightPole.contains(ballPos)){
                besidePoleFlag = true;
            }
            else {
                draw(QString("Clear"), Vector2D(0,1),"red");
            }
            return;
        }
        else {
            dangerForGoalieClear = false;
            draw(QString("strict follow"), Vector2D(0,1),"red");
            predictedBall = ballPrediction(true);
            if(predictedBall.x - 0.02 < goaliePos.x){
                Segment2D ball2PredictedBall(ballPos,predictedBall);
                if(ball2PredictedBall.intersection(robotPrGoalLine).valid()){
                    predictedBall = ball2PredictedBall.intersection(robotPrGoalLine);
                }

            }
            goalieTarget = strictFollowBall(predictedBall);

#ifdef STRICT_FOLLOW
            predictedBall = ballPrediction(true);
            if(predictedBall.x + 0.02 < goaliePos.x){
                Segment2D ball2PredictedBall(BallPos,predictedBall);
                if(ball2PredictedBall.intersection(robotPrGoalLine).valid()){
                    predictedBall = ball2PredictedBall.intersection(robotPrGoalLine);
                }

            }

            goalieTarget = strictFollowBall(predictedBall);
        }
#endif

    }

}
}





void DefensePlan::assignSkill(CAgent *_agent , CSkill *_skill){
    _agent->skill = _skill;
    _agent->skillName = _skill->getName();
    _skill->setAgent(_agent);

}



void DefensePlan::initGoalie(CAgent *_goalieAgent){
    goalieAgent = _goalieAgent;
    agents.clear();
    if( _goalieAgent )
        agents.append(_goalieAgent);

}



void DefensePlan::initDefense(const QList <CAgent*> &_defenseAgents){
    defenseAgents.clear();
    defenseAgents.append(_defenseAgents);
    agents.append(_defenseAgents);
}



void DefensePlan::setPointToKick()
{
    if( oneToucher < 0 || oneToucher >= defenseAgents.size() )
        oneToucher = 0;
    Vector2D ballPos = wm->ball->pos;
    Line2D ballLine(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm());
    if(defenseAgents.size() > 0){
        Line2D perpBallLine(defenseAgents.at(oneToucher)->self()->getKickerPos() , defenseAgents.at(oneToucher)->self()->getKickerPos()+wm->ball->vel.norm());
        perpBallLine = perpBallLine.perpendicular(defenseAgents.at(oneToucher)->self()->getKickerPos());
        Vector2D intersect = perpBallLine.intersection(ballLine);
        if( intersect.valid() ){
            if( isValidPoint(intersect) ){
                pointForKick = intersect;
            }
            else{
                //CHECK
                Vector2D* inter1 = getIntersectWithDefenseArea(ballLine, ballPos);
                if (inter1 != NULL && inter1->valid()){
                    pointForKick = *inter1;
                }
                else{
                    pointForKick.invalidate();
                    //				debug("SetPointToKick! ERROR" , Vector2D(-0.1,2.2) , "red");
                    return;
                }
            }
        }
        else{
            Vector2D *inter = getIntersectWithDefenseArea(Line2D(ballPos, midGoal), ballPos);
            if( inter == NULL || inter->valid() == false ){
                pointForKick.invalidate();
                return;
            }
            pointForKick = *inter;
            delete inter;
        }
        oneToucherDir = (Vector2D::unitVector(CSkillKickOneTouch::oneTouchAngle(pointForKick, Vector2D(0, 0), wm->ball->vel,
                                                                                (pointForKick - ballPos).norm(), wm->field->oppGoal(), conf()->SkillsParams_KickOneTouch_Landa(), conf()->SkillsParams_KickOneTouch_Gamma())));
        pointForKick -= oneToucherDir * (CRobot::center_from_kicker_new + CBall::radius);
    }
}



void DefensePlan::initVars(float goalCircleRad)
{
#ifndef OLD_FASTEST
    fastestToBall = knowledge->newFastestToBall(0.016 , wm->our.data->activeAgents , wm->opp.data->activeAgents);
    int ourFastest = fastestToBall.ourFastest();
#else
    fastestToBall = knowledge->findFastestToBall(wm->our.t->activeAgents, wm->opp.t->activeAgents);
    int ourFastest = fastestToBall.ourFastest;
#endif
    catch_time = fastestToBall.catch_time;
    isDefenseFastest = false;
    if( fastestToBall.isFastestOurs ){
        for (int i = 0 ; i < defenseAgents.size() ; i++)
        {
            if( defenseAgents[i]->id() == ourFastest )
            {
                isDefenseFastest = true;
                break;
            }
        }
    }

    ballPos = wm->ball->pos;
    ballVel = wm->ball->vel;
    topGoal = wm->field->ourGoalL();
    downGoal = wm->field->ourGoalR();
    midGoal = (topGoal + downGoal) / 2.0;
    ourAgentsRad = 0.09;
    ourGoalAreaCircleRad = goalCircleRad + ourAgentsRad * 1.5 + 0.3;

    knowledge->defenseClearer = -1;

    Vector2D upperGoalForCircle = topGoal;
    upperGoalForCircle.y = (_GOAL_WIDTH/2.0) / 2.f;
    Vector2D bottomGoalForCircle = downGoal;
    bottomGoalForCircle.y = -(_GOAL_WIDTH/2.0) / 2.f;

    goalieCircleRadius = 0.75 + 0.2;
    goalieCircleX = 0.5 + 0.3;

    defenseAreaBottomCircle = Circle2D(upperGoalForCircle, ourGoalAreaCircleRad);
    defenseAreaTopCircle = Circle2D(bottomGoalForCircle, ourGoalAreaCircleRad);
    defenseAreaLine = Segment2D(upperGoalForCircle + Vector2D(ourGoalAreaCircleRad, 0), bottomGoalForCircle + Vector2D(ourGoalAreaCircleRad, 0));

    Vector2D firstBallPos = wm->ball->pos;
    if (firstBallPos.x > wm->field->oppGoal().x - 0.01)
        firstBallPos.x = wm->field->oppGoal().x - 0.01;
    if (firstBallPos.y > wm->field->ourCornerL().y - 0.01)
        firstBallPos.y = wm->field->ourCornerL().y - 0.01;
    if (firstBallPos.y < wm->field->ourCornerR().y + 0.01)
        firstBallPos.y = wm->field->ourCornerR().y + 0.01;
    if (firstBallPos.x < topGoal.x + 0.05)
        firstBallPos.x  = topGoal.x + 0.05;

    ballPos = wm->ball->predict(catch_time);
    if (ballVel.length() > 0.3 /*&& fabs(Vector2D::angleBetween(Vector2D(-1, 0), wm->ball->vel).degree()) < 45*/)
        ballPos = firstBallPos;

    if (ballPos.x > wm->field->oppGoal().x || ballPos.y > wm->field->ourCornerL().y
            || ballPos.y < wm->field->ourCornerR().y
            || ballPos.x < topGoal.x + 0.05) {
        Segment2D segment(firstBallPos, ballPos);
        Vector2D vecs[] = { Segment2D(wm->field->ourCornerL() + Vector2D(0.05, 0), wm->field->oppCornerL()).intersection(segment)
                            , Segment2D(wm->field->ourCornerR() + Vector2D(0.05, 0), wm->field->oppCornerR()).intersection(segment)
                            , Segment2D(wm->field->ourCornerL() + Vector2D(0.05, 0), wm->field->ourCornerR() + Vector2D(0.05, 0)).intersection(segment)
                            , Segment2D(wm->field->oppCornerL(), wm->field->oppCornerR()).intersection(segment) };
        float minDist = 1000;
        for (int i = 0; i < 4; i++)
            if (vecs[i].valid())
            {
                float dist = firstBallPos.dist(vecs[i]);
                if (dist < minDist)
                {
                    ballPos = vecs[i];
                    minDist = dist;
                }
            }
    }
    clearDistanceForGoalie = ourGoalAreaCircleRad + 0.365;
    if (ballPos.dist(midGoal) < clearDistanceForGoalie)
        ballPos = midGoal + Vector2D().setPolar(clearDistanceForGoalie, (ballPos - midGoal).th().degree());

    draw(Circle2D(ballPos, 0.3), QColor("magenta"));

    goalieDegThreshold = 65.0;
    MinDefenseDistance = 0.1800;
}


void DefensePlan::preCalculate(){
    if ( goalieAgent )
        knowledge->goalie = goalieAgent;

    announceClearing(false);

    for (int i = 0; i < count() ; i++)
        agent(i)->idle = false;

    isItPossibleToClear = true;
    if ( policy()->Defense_NoClear() )
        isItPossibleToClear = false;

    for (int i = 0; i < count(); i++)
    {
        float dTheta = (wm->ball->pos - agent(i)->pos()).th().degree();
        if ( knowledge->isStart()
             && (wm->ball->pos.dist(agent(i)->pos()) < 1.0)
             && (fabs(dTheta) < goalieKickThreshold))
        {

            Vector2D normVec = (wm->field->oppGoal() - agent(i)->pos()).norm();
            agent(i)->setKick(agent(i)->kickSpeedValue(7.2 , false));
            bool crowded = knowledge->isCrowdedInFrontOfAgent(agent(i)->id(), 0.5);
            bool isClear = knowledge->isPointClear(normVec * 1.5 + agent(i)->pos(), normVec * 0.2 + agent(i)->pos(), CRobot::robot_radius_new, false, QList<int>(), QList<int>());
            if (goalieAgent != NULL && agent(i)->id() == goalieAgent->id())
                agent(i)->setChip(CHIP_POWER);
            else if ( !isClear) {
                agent(i)->setChip(LONG_CHIP_POWER);
            }
            else if (crowded && isClear) {
                agent(i)->setChip(CHIP_POWER);
            }
        }
        else
            agent(i)->setKick(0);
    }

    for (int i = 0 ; i < _MAX_NUM_PLAYERS ; i++)
    {
        if( knowledge->isStop() ){
            gpa[i]->setSlowMode(true);
            gpa[i]->setNoAvoid(false);
            gpa[i]->setAvoidPenaltyArea(false);
            gpa[i]->setAvoidCenterCircle(false);
        }
        else{
            gpa[i]->setSlowMode(false);
            gpa[i]->setNoAvoid(true);
        }
    }
}





bool DefensePlan::match(Vector2D pos1, Vector2D pos2, Vector2D target1, Vector2D target2)
{
    double angleDiff[2][2];
    angleDiff[0][0] = fabs(getDegree(pos1, midGoal, target1));
    angleDiff[0][1] = fabs(getDegree(pos1, midGoal, target2));
    angleDiff[1][0] = fabs(getDegree(pos2, midGoal, target1));
    angleDiff[1][1] = fabs(getDegree(pos2, midGoal, target2));

    float diffAngleMode1 = angleDiff[0][0] + angleDiff[1][1];
    float diffAngleMode2 = angleDiff[0][1] + angleDiff[1][0];
    if (fabs(diffAngleMode1 - diffAngleMode2) < 1) {
        float state1 = pos1.dist(target1) + pos2.dist(target2);
        float state2 = pos1.dist(target2) + pos2.dist(target1);
        if (fabs(state1 - state2) < 0.2)
            return (max(angleDiff[0][0], angleDiff[1][1]) < max(angleDiff[0][1], angleDiff[1][0]));
        return state1 < state2;
    }

    return (diffAngleMode1 < diffAngleMode2);
}


bool DefensePlan::isValidPoint(const Vector2D& point)
{
    float distThreshold = 0.10f;

    if (point.x < wm->field->fieldRect().left() - distThreshold)
        return false;
    if (point.x > wm->field->fieldRect().right() + distThreshold)
        return false;
    if (point.y < wm->field->fieldRect().bottom() - distThreshold)
        return false;
    if (point.y > wm->field->fieldRect().top() + distThreshold)
        return false;

    Rect2D rect(midGoal + Vector2D(0.0, defenseAreaLine.length() / 2.0), defenseAreaBottomCircle.radius(), defenseAreaLine.length());
    return  rect.contains(point) == false &&
            defenseAreaBottomCircle.contains(point) == false &&
            defenseAreaTopCircle.contains(point) == false;
}



DefensePlan::DefensePlan()
{
    //added by Arash.Z
    thr = 0;
    isOnetouch = 0;
    goalieClearFlag = 0;
    inPenaltyAreaFlag = 0;
    behindAgent = 0;
    noDefThr = 0;
    chipGKCounter = 0;

    savedClearPos.invalidate();
    clearCnt = 0;

    defenseCount = defenseAgents.size();

    defExeptions.active = false;
    defExeptions.exeptionMode = NoneExep;
    defExeptions.exepAgentId = -1;

    defClearThr = 0;
    defClearFlag = false;

    overDefThr = 0;

    ballBehindGoalie = false, goalieOneTouch = false, goalieInPenaltyAreaPrediction = false, goalieClearMode = false, goalieStrictFollow = false, goalieFollow = false, ballIsOutOfField = false;
    strictfollowThr = 0;
    behindBallThr = 0;

    besidePoleFlag = false;

    oneTouchCnt = 5;
    ///HMD
    markRadius = 1.6;
    markRadiusStrict = 1.39;
    segmentpershoot = policy()->Mark_ShootRatioBlock() / 100.0;
    segmentperpass = (100 - policy()->Mark_PassRatioBlock()) / 100.0;
    /////
    //added by KK
    predictThresh = 0;
    isInOneTouch = false;
    oneTouchCycleTest = 0;
    cycleCounter = 0;
    timeToReach = 0;
    doBlockPass = false;
    goalieAreaHis = 0;
    isBallGoingToOppAreaCnt = -1;
    pushBallHist = 0;
    failureAtempCnt = 0;
    goaliePassBlockCnt = -1;
    GOTThresh = 0.0;
    GOTCounter = 0;
    //end
    lastClearID = -1;
    lastTouchTheGoalie = -1;
    lastStateOffPlay = -1;
    oneToucher = 0;
    firstDefenseKickLine = -1;
    secondDefenseKickLine = 0;
    goalieKickThreshold = 70;
    for (int i = 0; i < _MAX_NUM_PLAYERS; i++)
    {
        lastMarker[i] = -1;

        gpa[i] = new CSkillGotoPointAvoid(NULL);
        gpa[i]->setNoAvoid(true);
        gpa[i]->setOneTouchMode(true);

        gps[i] = new CSkillGotoPoint(NULL);
        gps[i]->setBallMode(false);
        gps[i]->setSlowMode(false);
        gps[i]->setSlowShot(false);
        gps[i]->setTurningDist( 0.1);
    }
    kickSkill = new CSkillKick(NULL);
    upper_player = 0;
    tooFarDiffAngle = 30;
    isDefenseUpperThanGoalie = true;
    defenderForMark = false;
    doubleMarking = false;

}



Vector2D getIntersectPoint(Circle2D circle, Segment2D first, Segment2D second)
{
    Vector2D inter1, inter2;
    Vector2D intersection[2];
    int count = circle.intersection(first, intersection, intersection + 1);
    if (count == 0)
        return Vector2D(0, 0);
    if (count == 1)
        inter1 = intersection[0];
    else {
        if (intersection[0].dist(first.a()) < intersection[1].dist(first.b()))
            inter1 = intersection[0];
        else
            inter1 = intersection[1];
    }
    count = circle.intersection(second, intersection, intersection + 1);
    if (count == 0)
        return inter1;
    if (count == 1)
        inter2 = intersection[0];
    else {
        if (intersection[0].dist(second.a()) < intersection[1].dist(second.b()))
            inter2 = intersection[0];
        else
            inter2 = intersection[1];
    }
    return (inter1 + inter2) / 2;
}

Vector2D* DefensePlan::getIntersectWithDefenseArea(const Segment2D& segment, const Vector2D& blockPoint)
{
    Vector2D* intersectionWithBottomCircle[2];
    Vector2D* intersectionWithTopCircle[2];
    for (int i = 0; i < 2; i++) {
        intersectionWithBottomCircle[i] = new Vector2D();
        intersectionWithTopCircle[i] = new Vector2D();
    }
    Vector2D intersectionWithDefenseLine = defenseAreaLine.intersection(segment);
    int intersectionWithBottomCircleCount = defenseAreaBottomCircle.intersection(segment, intersectionWithBottomCircle[0], intersectionWithBottomCircle[1]);
    int intersectionWithTopCircleCount = defenseAreaTopCircle.intersection(segment, intersectionWithTopCircle[0], intersectionWithTopCircle[1]);

    vector<Vector2D*> points;
    for (int i = 0; i < intersectionWithBottomCircleCount; i++) {
        float angle = getDegree(defenseAreaBottomCircle.center() + Vector2D(1, 0), defenseAreaBottomCircle.center(), *intersectionWithBottomCircle[i]);
        if (angle <= 0 and angle >= -90)
            points.push_back(intersectionWithBottomCircle[i]);
        else
            delete intersectionWithBottomCircle[i];
    }
    for (int i = 0; i < intersectionWithTopCircleCount; i++) {
        float angle = getDegree(defenseAreaTopCircle.center() + Vector2D(1, 0), defenseAreaTopCircle.center(), *intersectionWithTopCircle[i]);
        if (angle >= 0 and angle <= 90)
            points.push_back(intersectionWithTopCircle[i]);
        else
            delete intersectionWithTopCircle[i];
    }

    float minDist = 99999;
    Vector2D* retPoint = NULL;
    if (intersectionWithDefenseLine.valid()) {
        retPoint = new Vector2D(intersectionWithDefenseLine);
        minDist = retPoint->dist(blockPoint);
    }
    for (vector<Vector2D*>::const_iterator it = points.begin(); it != points.end(); it++) {
        float dist = (*it)->dist(blockPoint);
        if (dist < minDist) {
            retPoint = (*it);
            minDist = dist;
        }
    }

    for (int i = 0; i < points.size(); i++)
        if (points[i] != retPoint)
            delete points[i];

    return retPoint;
}

Vector2D* DefensePlan::getIntersectWithDefenseArea(const Line2D& line, const Vector2D& blockPoint)
{
    Vector2D* intersectionWithBottomCircle[2];
    Vector2D* intersectionWithTopCircle[2];
    for (int i = 0; i < 2; i++) {
        intersectionWithBottomCircle[i] = new Vector2D();
        intersectionWithTopCircle[i] = new Vector2D();
    }
    Vector2D intersectionWithDefenseLine = defenseAreaLine.intersection(line);
    int intersectionWithBottomCircleCount = defenseAreaBottomCircle.intersection(line, intersectionWithBottomCircle[0], intersectionWithBottomCircle[1]);
    int intersectionWithTopCircleCount = defenseAreaTopCircle.intersection(line, intersectionWithTopCircle[0], intersectionWithTopCircle[1]);

    vector<Vector2D*> points;
    for (int i = 0; i < intersectionWithBottomCircleCount; i++) {
        float angle = getDegree(defenseAreaBottomCircle.center() + Vector2D(1, 0), defenseAreaBottomCircle.center(), *intersectionWithBottomCircle[i]);
        if (angle <= 0 and angle >= -90)
            points.push_back(intersectionWithBottomCircle[i]);
        else
            delete intersectionWithBottomCircle[i];
    }
    for (int i = 0; i < intersectionWithTopCircleCount; i++) {
        float angle = getDegree(defenseAreaTopCircle.center() + Vector2D(1, 0), defenseAreaTopCircle.center(), *intersectionWithTopCircle[i]);
        if (angle >= 0 and angle <= 90)
            points.push_back(intersectionWithTopCircle[i]);
        else
            delete intersectionWithTopCircle[i];
    }

    float minDist = 99999;
    Vector2D* retPoint = NULL;
    if (intersectionWithDefenseLine.valid()) {
        retPoint = new Vector2D(intersectionWithDefenseLine);
        minDist = retPoint->dist(blockPoint);
    }
    for (vector<Vector2D*>::const_iterator it = points.begin(); it != points.end(); it++) {
        float dist = (*it)->dist(blockPoint);
        if (dist < minDist) {
            retPoint = (*it);
            minDist = dist;
        }
    }

    for (int i = 0; i < points.size(); i++)
        if (points[i] != retPoint)
            delete points[i];

    return retPoint;
}

Vector2D* DefensePlan::getIntersectWithDefenseArea(const Circle2D& circle, bool upperPoint)
{
    Vector2D* intersectionWithBottomCircle[2];
    Vector2D* intersectionWithTopCircle[2];
    Vector2D* intersectionWithLine[2];
    for (int i = 0; i < 2; i++) {
        intersectionWithBottomCircle[i] = new Vector2D();
        intersectionWithTopCircle[i] = new Vector2D();
        intersectionWithLine[i] = new Vector2D();
    }
    int intersectionWithLineCount = circle.intersection(defenseAreaLine, intersectionWithLine[0], intersectionWithLine[1]);
    int intersectionWithBottomCircleCount = defenseAreaBottomCircle.intersection(circle, intersectionWithBottomCircle[0], intersectionWithBottomCircle[1]);
    int intersectionWithTopCircleCount = defenseAreaTopCircle.intersection(circle, intersectionWithTopCircle[0], intersectionWithTopCircle[1]);

    vector<Vector2D*> points;
    for (int i = 0; i < intersectionWithLineCount; i++)
        points.push_back(intersectionWithLine[i]);
    for (int i = 0; i < intersectionWithBottomCircleCount; i++) {
        float angle = getDegree(defenseAreaBottomCircle.center() + Vector2D(1, 0), defenseAreaBottomCircle.center(), *intersectionWithBottomCircle[i]);
        if (angle <= 0 and angle >= -90)
            points.push_back(intersectionWithBottomCircle[i]);
        else
            delete intersectionWithBottomCircle[i];
    }
    for (int i = 0; i < intersectionWithTopCircleCount; i++) {
        float angle = getDegree(defenseAreaTopCircle.center() + Vector2D(1, 0), defenseAreaTopCircle.center(), *intersectionWithTopCircle[i]);
        if (angle >= 0 and angle <= 90)
            points.push_back(intersectionWithTopCircle[i]);
        else
            delete intersectionWithTopCircle[i];
    }

    Vector2D* retPoint = NULL;
    for (vector<Vector2D*>::const_iterator it = points.begin(); it != points.end(); it++) {
        if ((upperPoint and (*it)->y > circle.center().y) or (not upperPoint and (*it)->y < circle.center().y)) {
            retPoint = (*it);
            break;
        }
    }

    for (int i = 0; i < points.size(); i++)
        if (points[i] != retPoint)
            delete points[i];

    return retPoint;
}

Vector2D DefensePlan::checkDefensePoint(CAgent* agent, const Vector2D& point)
{
    Vector2D agentPos = agent->pos();
    double distFromGoal = 1.6;
    if (agentPos.dist(midGoal) < distFromGoal) {
        agentPos = midGoal + Vector2D().setPolar(distFromGoal, (agentPos - midGoal).th().degree());
        Vector2D* inter = getIntersectWithDefenseArea(Segment2D(agentPos, midGoal), agentPos);
        if (inter != NULL) {
            agentPos = midGoal + Vector2D().setPolar(inter->dist(midGoal) + 0.1, (agentPos - midGoal).th().degree());
            delete inter;
        }
    }

    draw(Circle2D(agentPos , 0.02) , 0 , 360 , "brown" , true);


    Vector2D* inter = getIntersectWithDefenseArea(Segment2D(agentPos, point), agentPos);
    if (inter == NULL || agentPos.dist(*inter) < 0.1){ /// edited by AHZ for avoidance our penalty area
        return point;
    }

    draw(Circle2D(*inter , 0.02) , 0 , 360 , "pink" , true);

    Vector2D crossPoint = *inter;
    delete inter;

    AngleDeg deg = (crossPoint - midGoal).th();
    AngleDeg mainDeg = (point - midGoal).th();
    AngleDeg diff = mainDeg - deg;
    double s = min(10.0, fabs(diff.degree()));
    double finalDeg = deg.degree() + s * sign(diff.degree());
    Vector2D finalPos = midGoal + Vector2D().setPolar(100, finalDeg);
    inter = getIntersectWithDefenseArea(Segment2D(finalPos, midGoal), finalPos);
    Vector2D retPoint;
    if( inter != NULL ){
        retPoint = *inter;
        delete inter;
    }
    else{
        retPoint.invalidate();
    }

    draw(Circle2D(retPoint , 0.02) , 0 , 360 , "cyan" , true);

    Vector2D vecDiff = retPoint - crossPoint;
    double dist = crossPoint.dist(point);
    if (dist > 1)
        retPoint = retPoint + vecDiff.setLength(dist * 2);

    draw(Circle2D(retPoint , 0.02) , 0 , 360 , "blue" , true);

    return retPoint;
}

/////////////////////////////////////////////////mhmmd

void DefensePlan::matchingDefPos(int _defenseNum)
{
    QList <CAgent*> ourAgents;
    QList <Vector2D> matchPoints;
    QList <int> matchResult;

    ourAgents.clear();
    ourAgents.append(defenseAgents);
    if(defExeptions.active)
        ourAgents.removeOne(knowledge->getAgent(defExeptions.exepAgentId));
    matchPoints.clear();


    for(int i = 0 ; i < _defenseNum ; i++) {
        draw(tempDefPos.pos[i],0,QColor(Qt::blue));
        matchPoints.append(tempDefPos.pos[i]);

    }
    findOppAgentsToMark(matchPoints);
    findPos(decideNumOfMarks(defPosDecision.overDef));
    matchPoints.append(markPoses);
    draw(QString("aaaaaaaaaa  %1 %2").arg(matchPoints.count()).arg(_defenseNum),Vector2D(-2,2),"red");
    draw(QString("  %1").arg(ourAgents.count()),Vector2D(2,2),"red");

    if(matchPoints.count() == ourAgents.count())
    {
        knowledge->Matching(ourAgents,matchPoints,matchResult);
        debug(QString("matchpoints %1: matchresult %2 : markangles %3 : defensenum %4 ").arg(matchPoints.count()).arg(matchResult.count()).arg(markAngs.count()) .arg(_defenseNum), D_MAHI);
        for(int i =0; i < defenseCount  ; i++)
            defensePoints[i] = matchPoints[i];
        for(int i =0 ; i < matchPoints.count() && i < matchResult.count() ; i++)
        {            

            ///// edited by mahi && AHZ /////
            gpa[ourAgents[i]->id()]->noRelax();
            for (int j = 0; j  < ourAgents.size(); j++) {
                if (j != i) {
                    gpa[ourAgents[i]->id()]->ourRelax(ourAgents[j]->id());
                }
            }
            ///////////////////////////////////////
            assignSkill( ourAgents[i] , gpa[ourAgents[i]->id()]);
            if(ourAgents[i]->pos().dist(matchPoints[matchResult[i]]) > 0.35)
                matchPoints[matchResult[i]] = checkDefensePoint(ourAgents[i], matchPoints[matchResult[i]]);
            draw(Circle2D(matchPoints[matchResult[i]] , 0.05) , 0 , 360 , "black" , true);

            gpa[ourAgents[i]->id()]->setNoAvoid(true);
            gpa[ourAgents[i]->id()]->setSlowMode(false);
            gpa[ourAgents[i]->id()]->setAvoidPenaltyArea(false);
            gpa[ourAgents[i]->id()]->setAvoidBall(false);
            gpa[ourAgents[i]->id()]->setBallObstacleRadius(0);
            //gpa[ourAgents[i]->id()]->setBallObstacleRadius(0.5);
            //gpa[ourAgents[i]->id()]->setLookAt(wm->ball->pos);

           /* if(knowledge->getGameState() == CKnowledge::TheirIndirectKick)
            {
                gpa[ourAgents[i]->id()]->setNoAvoid(false);
                gpa[ourAgents[i]->id()]->setAvoidBall(true);
                gpa[ourAgents[i]->id()]->setBallObstacleRadius(0.5);
            }
            */
            if(knowledge->getGameState() == CKnowledge::TheirIndirectKick)
            {
                gpa[ourAgents[i]->id()]->setNoAvoid(false);
                gpa[ourAgents[i]->id()]->setSlowMode(false);
                gpa[ourAgents[i]->id()]->setAvoidPenaltyArea(false);
                gpa[ourAgents[i]->id()]->setAvoidBall(true);
                gpa[ourAgents[i]->id()]->setBallObstacleRadius(0.5);

            }

            if(matchResult[i] < _defenseNum){

                gpa[ourAgents[i]->id()]->init(matchPoints[matchResult[i]] , matchPoints[matchResult[i]] - wm->field->ourGoal());

            }
            else
            {

                gpa[ourAgents[i]->id()]->init(matchPoints[matchResult[i]] , matchPoints[matchResult[i]] - wm->field->ourGoal());

            }

        }
    }

}


void DefensePlan::execute()
{
    ////////////initialize////////////////
    initVars();
    preCalculate();
    ballPosHistory.prepend(Vector2D(wm->ball->pos.x, wm->ball->pos.y));
    if(ballPosHistory.count() > 7)
        ballPosHistory.removeLast();
    //////////////////////////////////////
    bool playOn = knowledge->getGameMode() == CKnowledge::Start;
    //   draw(QString("gID : %1").arg(knowledge->goalie->id()),Vector2D(0,0));
    ///////Arash.Z Test/////
    if (knowledge->getGameState() == CKnowledge::TheirPenaltyKick) //Penalty Mode
    {
        if( goalieAgent != NULL ){
            draw(QString("Penalty") , Vector2D(1,2) , "white");
            penaltyGoalie();
        }
        else{
            draw(QString("No Goalie!") , Vector2D(1,2) , "white");
        }
        return;
    }
    else {
        if(knowledge->goalie != NULL) {
            checkGoalieState();
            runGoalie();
            executeGoalie();
        }
        if(defenseAgents.size() > 0){
            if(playOn){
                checkDefenseExeptions();
                if(defExeptions.active){
                    runDefenseExeptions();
                    defenseCount = defenseAgents.size() - 1;
                }
                else{
                    defExeptions.exepAgentId = -1;
                    defExeptions.exeptionMode = NoneExep;
                    defenseCount = defenseAgents.size();
                }
            }
            else {
                defenseCount = defenseAgents.size();
            }
            if(defenseCount > 0) {
                defPosDecision = defPos.getDefPositions(wm->ball->pos, defenseCount, 1.5, 2.5);
                int realDefSize = defenseCount - decideNumOfMarks(defPosDecision.overDef);
                tempDefPos = defPos.getDefPositions(ballPrediction(false), realDefSize, 1.5, 2.5);
                matchingDefPos(realDefSize);
            }

        }
    }
    return;

}


void DefensePlan::announceClearing(bool state)
{
    if (state) knowledge->variables["clearing"] = "true";
    else knowledge->variables["clearing"] = "false";
}


void DefensePlan::penaltyGoalie()
{

    for( int i=0; i<5 ; i++ ){
        executeSkill[i] = false;
    }

    int defAgentNum = defenseAgents.count();
    switch (defAgentNum){
    case 1:
        defenseTargets[0]=Vector2D(-2,1);
        defenseDirs[0]=defenseTargets[0]-wm->ball->pos;
        executeSkill[0]=1;
        break;
    case 2:
        defensePoints[0]=Vector2D(-2,1);
        executeSkill[0]=1;
        defensePoints[1]=Vector2D(-2,1.5);
        executeSkill[1]=1;
        doMatch(defenseAgents[0]->pos() , defenseAgents[1]->pos() , defensePoints[0] , defensePoints[1] , -1 , 1);
        defenseDirs[0]=defensePoints[0]-wm->ball->pos;
        defenseDirs[1]=defensePoints[1]-wm->ball->pos;
        defenseTargets[0]=defensePoints[0];
        defenseTargets[1]=defensePoints[1];
        break;
    case 3:
        defensePoints[0]=Vector2D(-2,1);
        executeSkill[0]=1;
        defensePoints[1]=Vector2D(-2,1.5);
        executeSkill[1]=1;
        defensePoints[2]=Vector2D(-2,-1);
        executeSkill[2]=1;

        doMatchThree(defenseAgents[0]->pos() , defenseAgents[1]->pos() , defensePoints[0] , defensePoints[1] , 0 , 1);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[0]->pos() , defensePoints[2] , defensePoints[0] , 2 , 0);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[1]->pos() , defensePoints[2] , defensePoints[1] , 2 , 1);
        doMatchThree(defenseAgents[0]->pos() , defenseAgents[1]->pos() , defensePoints[0] , defensePoints[1] , 0 , 1);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[0]->pos() , defensePoints[2] , defensePoints[0] , 2 , 0);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[1]->pos() , defensePoints[2] , defensePoints[1] , 2 , 1);


        defenseTargets[0]=defensePoints[0];
        defenseTargets[1]=defensePoints[1];
        defenseTargets[2]=defensePoints[2];

        defenseDirs[0]=defenseTargets[0]-wm->ball->pos;
        defenseDirs[1]=defenseTargets[1]-wm->ball->pos;
        defenseDirs[2]=defenseTargets[2]-wm->ball->pos;
        break;
    case 4:
        defensePoints[0]=Vector2D(-2,1);
        executeSkill[0]=1;
        defensePoints[1]=Vector2D(-2,1.5);
        executeSkill[1]=1;
        defensePoints[2]=Vector2D(-2,-1);
        executeSkill[2]=1;
        defensePoints[3]=Vector2D(-2,-1.5);
        executeSkill[3]=1;

        doMatchThree(defenseAgents[0]->pos() , defenseAgents[1]->pos() , defensePoints[0] , defensePoints[1] , 0 , 1);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[0]->pos() , defensePoints[2] , defensePoints[0] , 2 , 0);
        doMatchThree(defenseAgents[0]->pos() , defenseAgents[3]->pos() , defensePoints[0] , defensePoints[3] , 0 , 3);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[1]->pos() , defensePoints[2] , defensePoints[1] , 2 , 1);
        doMatchThree(defenseAgents[3]->pos() , defenseAgents[1]->pos() , defensePoints[3] , defensePoints[1] , 3 , 1);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[3]->pos() , defensePoints[2] , defensePoints[3] , 2 , 3);
        doMatchThree(defenseAgents[0]->pos() , defenseAgents[1]->pos() , defensePoints[0] , defensePoints[1] , 0 , 1);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[0]->pos() , defensePoints[2] , defensePoints[0] , 2 , 0);
        doMatchThree(defenseAgents[0]->pos() , defenseAgents[3]->pos() , defensePoints[0] , defensePoints[3] , 0 , 3);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[1]->pos() , defensePoints[2] , defensePoints[1] , 2 , 1);
        doMatchThree(defenseAgents[3]->pos() , defenseAgents[1]->pos() , defensePoints[3] , defensePoints[1] , 3 , 1);
        doMatchThree(defenseAgents[2]->pos() , defenseAgents[3]->pos() , defensePoints[2] , defensePoints[3] , 2 , 3);

        defenseTargets[0]=defensePoints[0];
        defenseTargets[1]=defensePoints[1];
        defenseTargets[2]=defensePoints[2];
        defenseTargets[3]=defensePoints[3];

        defenseDirs[0]=-(defenseTargets[0]-wm->ball->pos);
        defenseDirs[1]=-(defenseTargets[1]-wm->ball->pos);
        defenseDirs[2]=-(defenseTargets[2]-wm->ball->pos);
        defenseDirs[3]=(defenseTargets[3]-wm->ball->pos);

        break;
    }
    Vector2D ballPos = wm->ball->pos;
    const float goalLineExtra = 0.03;
    const double xDiff = 0.10;
    Line2D newLine(wm->field->ourGoalL()+Vector2D(+xDiff,+goalLineExtra),
                   wm->field->ourGoalR()+Vector2D(+xDiff,-goalLineExtra));

    CRobot* opp = NULL;
    float minDist = 9999.9;
    for (int i = 0 ; i < wm->opp.activeAgentsCount() ; i++)
    {
        float dist = wm->opp.active(i)->pos.dist(ballPos);
        if (dist < minDist)
        {
            opp = wm->opp.active(i);
            minDist = dist;
        }
    }

    if (opp == NULL) return; //### Added By Ali

    const double epsilon = 0.12;
    Vector2D target(-2.93, 0.0);

    Line2D ballRay(ballPos, ballPos + opp->dir);
    Vector2D intersectionPoint = newLine.intersection(ballRay);
    if (intersectionPoint.valid())
        target = intersectionPoint;
    else
        target.y = 0.0;
    //sounds like this target is tokhom
    target.y = min(max(target.y, wm->field->ourGoalR().y + epsilon), wm->field->ourGoalL().y - epsilon + 0.03);

    Vector2D targetDir(10, 10);
    targetDir.setDir(AngleDeg(60));
    targetDir.setLength(1);

    assignSkill(goalieAgent , gpa[goalieAgent->id()]);
    gpa[goalieAgent->id()]->init(target, targetDir); // in Ejdeha code : gpa[goalieAgent->id()]->init(target, wm->ball->pos - goalieTarget);
    if( executeSkill[0] ){
        defenseTargets[0] = checkDefensePoint(defenseAgents.at(0), defenseTargets[0]);
        assignSkill( defenseAgents.at(0) , gpa[defenseAgents.at(0)->id()]);
        gpa[defenseAgents.at(0)->id()]->setTarget(defenseTargets[0], -defenseTargets[0] + wm->ball->pos);
        draw(Circle2D(defenseTargets[0] , 0.05) , 0 , 360 , "black" , true);

    }

    if( executeSkill[1] ){
        defenseTargets[1] = checkDefensePoint(defenseAgents.at(1), defenseTargets[1]);
        assignSkill( defenseAgents.at(1) , gpa[defenseAgents.at(1)->id()]);
        gpa[defenseAgents.at(1)->id()]->setTarget(defenseTargets[1] , -defenseTargets[1] + wm->ball->pos);
        draw(Circle2D(defenseTargets[1] , 0.05) , 0 , 360 , "black" , true);
    }


    if( executeSkill[2] ){
        defenseTargets[2] = checkDefensePoint(defenseAgents.at(2), defenseTargets[2]);
        assignSkill( defenseAgents.at(2) , gpa[defenseAgents.at(2)->id()]);
        gpa[defenseAgents.at(2)->id()]->setTarget(defenseTargets[2] , -defenseTargets[2]  + wm->ball->pos);
        draw(Circle2D(defenseTargets[2] , 0.05) , 0 , 360 , "black" , true);

    }

    if( executeSkill[3] ){
        defenseTargets[3] = checkDefensePoint(defenseAgents.at(3), defenseTargets[3]);
        assignSkill( defenseAgents.at(3) , gpa[defenseAgents.at(3)->id()]);
        gpa[defenseAgents.at(3)->id()]->setTarget(defenseTargets[3] , -defenseTargets[3]  + wm->ball->pos);
        draw(Circle2D(defenseTargets[3] , 0.05) , 0 , 360 , "black" , true);
    }
}



bool DefensePlan::defenseClearOrNot(){
    Vector2D ballPos = wm->ball->pos;
    defenseClearIndex = -1;

    bool isGameStarted = knowledge->isStart();

    if (ballPos.dist(wm->field->ourGoal()) > 3.5) {
        distClearHysteresis = false;
    }
    else if (ballPos.dist(wm->field->ourGoal()) < 2.5) {
        distClearHysteresis = true;
    }

    bool ballVelOrDirection = true;
    if( wm->ball->vel.length() > 1.5 || isBallGoingToOppArea() )
        ballVelOrDirection = false;

    bool isOutOfPenaltyArea = true;
    //Goalie Clear Condition
    Vector2D BallPosPenalty = wm->ball->pos + (wm->field->ourGoal() - wm->ball->pos).norm()*0.07;
    if ( wm->field->isInOurPenaltyArea(BallPosPenalty) && (wm->ball->vel.length() < 0.9) ){
        isOutOfPenaltyArea = false;
    }

    if( distClearHysteresis == false
            || isGameStarted == false
            || ballVelOrDirection == false
            || isItPossibleToClear == false
            || isOutOfPenaltyArea == false ){
        lastClearID = defenseClearIndex = -1;
        clearFrameCnt = 0;
        savedClearDist = 0;
        return false;
    }

    if( lastClearID != -1 && ((clearFrameCnt < 30 && savedClearDist < 0.05) || (clearFrameCnt < 80 && savedClearDist >= 0.05)) && !isBallGoingToOppArea() ){
        defenseClearIndex = lastClearID;
        if(knowledge->goalie != NULL) {
            for(int i=0; i < wm->our.activeAgentsCount(); i++){
                if (defenseAgents[defenseClearIndex]->id() != wm->our.active(i)->id && wm->our.active(i)->id != knowledge->goalie->id() ){
                    if((defenseAgents[defenseClearIndex]->distToBall().length() - 0.3) > wm->our.active(i)->pos.dist(ballPos)|| defenseAgents[defenseClearIndex]->distToBall().length() > 1.5 + defClearThr)
                        return false;
                }
            }
        }
        else {
            for(int i=0; i < wm->our.activeAgentsCount(); i++){
                if (defenseAgents[defenseClearIndex]->id() != wm->our.active(i)->id) {
                    if((defenseAgents[defenseClearIndex]->distToBall().length() - 0.3) > wm->our.active(i)->pos.dist(ballPos)|| defenseAgents[defenseClearIndex]->distToBall().length() > 1.5 + defClearThr)
                        return false;
                }
                for(int i=0; i < wm->opp.activeAgentsCount(); i++){
                    if((defenseAgents[defenseClearIndex]->distToBall().length() - 0.3) > wm->opp.active(i)->pos.dist(ballPos) || defenseAgents[defenseClearIndex]->distToBall().length() > 1.5 + defClearThr)
                        return false;
                }
            }
        }
        clearFrameCnt++;
        return true;
    }

#ifndef OLD_FASTEST
    double ourF = fastestToBall.ourFastestTime();
    double oppF = fastestToBall.oppFastestTime();
#else
    double ourF = fastestToBall.ourFastestTime;
    double oppF = fastestToBall.oppFastestTime;
#endif

    bool clearHysteresis = false;
    if( oppF != -1 && ourF == -1 )
        clearHysteresis = false;
    else if( isDefenseFastest && oppF == -1 && ourF != -1 ){
        clearHysteresis = true;
    }
    else if( isDefenseFastest && oppF-0.2 > ourF ){
        clearHysteresis = true;
    }

    if( clearHysteresis ){
        double minDist = 1000;
        for (int i = 0; i < defenseAgents.size(); i++){
            double dist = defenseAgents.at(i)->pos().dist(ballPos);
            if( i == lastClearID )
                continue;
            if (dist < minDist){
                defenseClearIndex = i;
                minDist = dist;
            }
        }
        if( defenseClearIndex != -1 && lastClearID != -1 ){
            if( minDist > lastClearDist-0.05 ){
                defenseClearIndex = lastClearID;
            }
            else{
                lastClearID = defenseClearIndex;
                lastClearDist = minDist;
            }
        }
        if( defenseClearIndex != -1 && wm->our.activeAgentsCount() > 0){
            if(knowledge->goalie != NULL) {
                for(int i=0; i < wm->our.activeAgentsCount(); i++){
                    if (defenseAgents[defenseClearIndex]->id() != wm->our.active(i)->id && wm->our.active(i)->id != knowledge->goalie->id() ){
                        if((defenseAgents[defenseClearIndex]->distToBall().length() - 0.3) > wm->our.active(i)->pos.dist(ballPos)|| defenseAgents[defenseClearIndex]->distToBall().length() > 1.5 + defClearThr)
                            return false;
                    }
                }
            }
            else {
                for(int i=0; i < wm->our.activeAgentsCount(); i++){
                    if (defenseAgents[defenseClearIndex]->id() != wm->our.active(i)->id) {
                        if((defenseAgents[defenseClearIndex]->distToBall().length() - 0.3) > wm->our.active(i)->pos.dist(ballPos)|| defenseAgents[defenseClearIndex]->distToBall().length() > 1.5 + defClearThr)
                            return false;
                    }
                }
                for(int i=0; i < wm->opp.activeAgentsCount(); i++){
                    if((defenseAgents[defenseClearIndex]->distToBall().length() - 0.3) > wm->opp.active(i)->pos.dist(ballPos) || defenseAgents[defenseClearIndex]->distToBall().length() > 1.5 + defClearThr)
                        return false;
                }
            }
            lastClearID = defenseClearIndex;
            clearFrameCnt = 0;
            return true;
        }


        else{
            lastClearID = defenseClearIndex = -1;
            clearFrameCnt = 0;
            savedClearDist = 0;
            return false;
        }
    }
    else{
        lastClearID = defenseClearIndex = -1;
        clearFrameCnt = 0;
        isBallGoingToOppAreaCnt = -1;
        savedClearDist = 0;
        return false;
    }
    savedClearDist = 0;
    return false;
}


void DefensePlan::runClear(){


    if( defenseClearIndex < 0 || defenseClearIndex >= defenseAgents.size() ){
        return;
    }
    announceClearing(true);
    assignSkill(defenseAgents.at(defenseClearIndex), kickSkill);
    kickSkill->setKickSpeed(kickSkill->getAgent()->kickSpeedValue(7.2,false));
    kickSkill->setTolerance(1.5);
    kickSkill->setDontKick(false);
    kickSkill->setInterceptMode(true);
    kickSkill->setClear(false);
    kickSkill->setTarget(wm->field->oppGoal());
    kickSkill->setSlow(false);
    kickSkill->setSpin(false);
    kickSkill->setChip(false);
    knowledge->defenseClearer = defenseAgents.at(defenseClearIndex)->id();
    kickSkill->setAvoidPenaltyArea(true);

    if(!isPathToOppGoalieClear() || savedClearDist > 0.05)
    {

        savedClearPos = findBestPointForChipTarget(savedClearDist,0);
        kickSkill->setTarget(savedClearPos);
        kickSkill->getAgent()->chipDistanceValue(savedClearDist, false);

        draw(savedClearPos);
        kickSkill->setChip(true);
    }
    if ( (defenseAgents.at(defenseClearIndex)->pos().dist(wm->ball->pos) < 0.5) &&
         (fabs(Vector2D::angleBetween(Vector2D(1.0, 0.0),(wm->ball->pos - wm->field->ourGoal())).degree()) > 40) &&
         (wm->ball->pos.dist(wm->field->ourGoal()) > 1.4)
         ) //good conditions for swapping clearer agent with an idle attacker
    {
        knowledge->formationChanges["clearswap"] = defenseAgents.at(defenseClearIndex)->id();
    }

}


void DefensePlan::doMatch(Vector2D pos1 , Vector2D pos2 , Vector2D point0 , Vector2D point1 , int first , int second ){
    if( match( pos1 , pos2 , point0 , point1 ) )
    {
        defenseTargets[0] = point0;
        lastMarker[0] = first;
        defenseTargets[1] = point1;
        lastMarker[1] = second;
    }
    else
    {
        defenseTargets[0] = point1;
        lastMarker[0] = second;
        defenseTargets[1] = point0;
        lastMarker[1] = first;
    }
}



void DefensePlan::doMatchThree(Vector2D pos1 , Vector2D pos2 , Vector2D point0 , Vector2D point1 , int first , int second  ){
    if( match( pos1 , pos2 , point0 , point1 ) )
    {
        defensePoints[first] = point0;

        defensePoints[second] = point1;

    }
    else
    {
        defensePoints[first] = point1;

        defensePoints[second] = point0;

    }


}




void DefensePlan::calcPointForOneTouch()
{
    if( wm->field->isInOurPenaltyArea(defensePoints[0]) == true
            || wm->field->fieldRect().contains(defensePoints[0]) == false /*isValidPoint(defensePoints[0]) == false*/ ){
        //CHECK
        //calculate intersect point for one touch!
        Vector2D* inter2 = getIntersectWithDefenseArea(Line2D(wm->ball->pos , defensePoints[0]) , ballPos);
        if (inter2 != NULL && inter2->valid()){
            defensePoints[0] = *inter2;
            delete inter2;
        }
        else{
            draw("oneDefenseAndGoalie! ERROR1" , Vector2D(-0.1,2.2) , "red");
        }
    }
}


//added by KK

velAndAccByKK DefensePlan::getVelocityByPos(){
    /*this function calculate ball velocity and acceleration using last three ball position
   *ballPosHistory is a QList that we prepend ball position in it
   */
    velAndAccByKK res;
    if(ballPosHistory.count() < 3 ){
        res.vel = 0.0;
        res.acc = 0.0;
        return res;
    }

    double vel1 = ballPosHistory.at(0).dist(ballPosHistory.at(1))/LOOP_TIME_BYKK;
    double vel2 = ballPosHistory.at(1).dist(ballPosHistory.at(2))/LOOP_TIME_BYKK;
    double acc1 = fabs(vel1 - vel2)/LOOP_TIME_BYKK;
    res.vel = vel1;
    res.acc = acc1;
    return res;
}


bool DefensePlan::checkBallDangerForOneTouch(){
    //check dangerous velocity according to distance
    //velocity will be calculated using ball position in getVelocityByPos func.
    Segment2D ballLine = Segment2D(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm()*10);
    Segment2D goalLine(wm->field->ourGoal()+Vector2D(0,1) , wm->field->ourGoal()+Vector2D(0,-1));
    Vector2D goalLineIntersect = goalLine.intersection(ballLine);
    double ballVel = getVelocityByPos().vel;
    double ballDist = wm->ball->pos.dist(wm->field->ourGoal());
    if(((ballDist<6 && ballVel>3)
        || (ballDist<5 && ballVel>2.5)
        || (ballDist<4 && ballVel>2)
        || (ballDist<3 && ballVel>1.5)
        || (ballDist<2 && ballVel>.5)) && goalLineIntersect.valid() && !behindAgent)
        /*|| (ballDist<1.5 && ballVel>0.5)*/

        //if(temp.vel > 3.5)
        return true;
    return false;
}

bool DefensePlan::defenseCheckBallDangerForOneTouch(){
    //check dangerous velocity according to distance
    //velocity will be calculated using ball position in getVelocityByPos func.
    double defCircleRad = 0;
    Circle2D defCircle;
    if(defenseAgents.count() > 0){
        for(int i=0; i < defenseAgents.count(); i++){
            if(defenseAgents[i]->pos().dist(ballPos) > defCircleRad)
                defCircleRad = defenseAgents[i]->pos().dist(wm->field->ourGoal());
        }
    }
    if(defCircleRad > 0){
        defCircle = Circle2D(wm->field->ourGoal(),defCircleRad);
    }
    else {
        defCircle = Circle2D((wm->field->ourGoal() - Vector2D(0.2,0)),1.50);
    }
    double ballVel = getVelocityByPos().vel;
    double ballDist = wm->ball->pos.dist(wm->field->ourGoal());
    if(((ballDist<6 && ballVel>3)
        || (ballDist<5 && ballVel>2.5)
        || (ballDist<4 && ballVel>2)
        || (ballDist<3 && ballVel>1.5)
        || (ballDist<2 && ballVel>.5)) && !defCircle.contains(ballPos))
        return true;
    return false;
}

int DefensePlan::decideNumOfMarksInPlayOff(int _defenseCount) {

    // used in playoff without counting goalie
    // TODO: knowlege->tobemark should be replaced
    if(policy()->Mark_ManToManAllTransiant() && knowledge->toBeMopps.count() < _defenseCount ){
         return _defenseCount - knowledge->toBeMopps.count();
    } else {
        return _defenseCount;
    }
}

bool DefensePlan::checkStillBeingInOneTouch(bool goalieFlag){
    if(goalieFlag) {

        if(knowledge->goalie != NULL)
        {
            //////////////////////////////Edited by Arash.Z//////////////////////////
            Segment2D goalLine(wm->field->ourGoal()+Vector2D(0,1) , wm->field->ourGoal()+Vector2D(0,-1));
            Segment2D ballLine = Segment2D(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm()*10);
            Vector2D goalLineIntersect = goalLine.intersection(ballLine);
            Vector2D ballPos = wm->ball->pos;
            Vector2D goaliePos = knowledge->goalie->pos();
            Circle2D defCircle((wm->field->ourGoal() - Vector2D(0.2,0)),1.40);
            if(goalieFlag){
                if((wm->ball->pos.x + 0.09) < goaliePos.x)
                    behindAgent = 1;
                else
                    behindAgent = 0;
            }
            else{
                if(defCircle.contains(wm->ball->pos))
                    behindAgent = 1;
                else
                    behindAgent = 0;
            }
            if(checkBallDangerForOneTouch() && !isOnetouch)
                isOnetouch = 1;
            else
            {
                if(!goalLineIntersect.valid() || behindAgent )
                    isOnetouch = 0;
            }
            return isOnetouch;
        }
    }
    else {

        ////////////////////////////Commented by Arash.Z/////////////////////////
        //    //check oneTouch timer so it wont switch state machine
        if(!isInOneTouch){
            cycleCounter = 0;
            return false;
        }

        Segment2D ballLine = Segment2D(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm()*10);
        Segment2D goalLine(wm->field->ourGoal()+Vector2D(0,1) , wm->field->ourGoal()+Vector2D(0,-1));
        Vector2D goalLineIntersect = goalLine.intersection(ballLine);
        if(!goalLineIntersect.valid() )
        {
            cycleCounter = 0;
            isInOneTouch = false;
            return false;
        }

        draw(QString("cyclePredict %1").arg(oneTouchCycleTest - cycleCounter) , Vector2D(1,1.4) , "white");
        cycleCounter++;

        //timer is based on ai cycle
        if(cycleCounter < oneTouchCycleTest + 5)
            return true;
        else{
            cycleCounter = 0;
            isInOneTouch = false;
            return false;
        }
    }
}

int DefensePlan::checkOppPassDanger(QList<int> &arr)
{
    /*this function check the dangerous opp according to pass line
   *if any of opponents were in pass path with a few margin, we consider it as a danger for recieving pass
   *ID of each dangerous robot would be store in a Qlist
   *and number of dangerous oppenents would be returned
   */

    double ballDist = wm->ball->pos.dist(wm->field->ourGoal());
    double ballVel = wm->ball->vel.length();
    bool ballDistVelFlag =     (ballDist<6 && ballVel>3)
            || (ballDist<5 && ballVel>2.5)
            || (ballDist<4 && ballVel>2)
            || (ballDist<3 && ballVel>1.5)
            || (ballDist<2 && ballVel>1)
            || (ballDist<1.5 && ballVel>0.5)
            || (ballDist<1 && ballVel>0.2);
    if(ballDistVelFlag)
        return 0;
    Segment2D ballToGoal(wm->ball->pos,wm->ball->pos + wm->ball->vel.norm()*20);
    if(wm->ball->vel.length() > 0.1 )
        draw(ballToGoal,QColor(Qt::yellow));
    Vector2D temPos;
    Vector2D point1, point2;
    Circle2D tempCircle;
    Segment2D ourLine(wm->field->ourGoal()+Vector2D(0,wm->field->ourGoalL().y+_GOAL_WIDTH),wm->field->ourGoal()+Vector2D(0,wm->field->ourGoalR().y-_GOAL_WIDTH));
    int count = 0;
    if(!ballToGoal.intersection(ourLine).valid())
        return 0;
    for(int i=0; i < wm->opp.activeAgentsCount() ; i++ )
    {
        temPos = wm->opp.active(i)->pos;
        tempCircle.assign(temPos,0.3);
        if(tempCircle.intersection(ballToGoal,&point1, &point2))
        {
            arr.append(wm->opp.active(i)->id);
            count++;
        }
    }
    return count;
}


double DefensePlan::timeToReachPoint(double dist)
{
    //get reach time according to robot acc and vel (x=0.5*a*t^2 + V*t
    //set robot maximum velocity in worst condition
    double velocity = 2.1;
    //set robot maximum acceleration in worst condition
    double acc = 3;
    double time;
    time = sqrt((2*dist)/acc);
    double tt = velocity/acc;
    if(time < tt )
        return time;
    dist = dist - (0.5*acc*tt*tt);
    time = dist/velocity + tt;
    return time;
}



Vector2D DefensePlan::findBestPointForChipTarget(double &chipDist,bool isGoalie)
{
    double region;
    QList <int> ourRel;
    QList <int> oppRel;
    QList <Vector2D> points;
    QList <int> IDs;
    Vector2D dangerPoint;
    Vector2D dangerPointOnMiddleLine;
    Vector2D bestPos;
    Vector2D dirTemp;
    Vector2D regionPoses;
    Vector2D agentPos = Vector2D(0,0);
    Vector2D dir = Vector2D(0,0);
    //in sharte ezafe shode vase vaghti ke defence nis at(0) ro ke mikhonim segment mide
    if(defenseAgents.size() && !isGoalie)
    {
        agentPos = (defenseClearIndex == -1)? defenseAgents.at(0)->pos() : defenseAgents.at(defenseClearIndex)->pos();
        dir = (defenseClearIndex == -1)? defenseAgents.at(0)->dir() : defenseAgents.at(defenseClearIndex)->dir();
    }
    else if(isGoalie && goalieAgent->isVisible())
    {
        agentPos = goalieAgent->pos();
        dir = goalieAgent->dir();
    }
    ourRel.clear();

    double minRegion = 1000;
    int num = 0;
    for(double i = -(_FIELD_HEIGHT/2); i <= (_FIELD_HEIGHT/2); i = i + (_FIELD_HEIGHT/25))
    {
        if(!isGoalie) {
            dangerPoint = knowledge->getEmptyPosOnGoal(Vector2D(3, i), region, false, oppRel, ourRel);
            dangerPointOnMiddleLine = Vector2D(3, i);
        }
        else {
            dangerPoint = knowledge->getEmptyPosOnGoal(Vector2D(0, i), region, false, oppRel, ourRel);
            dangerPointOnMiddleLine = Vector2D(0, i);
        }
        if(region < minRegion)
        {
            minRegion = region;
            bestPos = dangerPointOnMiddleLine;
        }
        if(region < 0.25)
        {
            points.append(dangerPointOnMiddleLine);
            IDs.append(num);
        }
        num++;
    }
    num = 1;
    if(points.count() <= 0)
    {
        draw(bestPos,0,QColor(Qt::darkRed));
        draw(QString("chip target") , bestPos + Vector2D(0, 0.13), "white", 10);
        chipDist = agentPos.dist(bestPos);
        return bestPos;
    }
    else
    {
        Vector2D ballPosTemp = wm->ball->pos;
        //int bestPosID = -1;
        double minDiff = 1000;
        for(int j=0; j < IDs.count(); j++)
        {
            regionPoses = Vector2D(0, -(_FIELD_HEIGHT/2) + (_FIELD_HEIGHT/25)* IDs.at(j) );
            dirTemp = regionPoses - ballPosTemp;
            if(fabs(dirTemp.th().degree() - dir.th().degree()) < minDiff)
            {
                bestPos = regionPoses;
                minDiff = fabs(dirTemp.th().degree() - dir.th().degree());
            }
            draw(Segment2D(regionPoses, regionPoses + dirTemp.norm()*2),QColor(Qt::darkRed));
        }
        draw(bestPos,0,QColor(Qt::darkRed));
        draw(QString("chip target") , bestPos + Vector2D(0, 0.13), "white", 10);

        chipDist = agentPos.dist(bestPos);
        return bestPos;
    }
}


bool DefensePlan::isPathToOppGoalieClear()
{
    Vector2D oppPosTemp;
    Circle2D oppCircleTemp;
    Vector2D point1, point2;
    double radiusTemp;
    radiusTemp = 0.1 ;
    Segment2D lineToGoal;

    for(int k = 0; k < 2; k++)
    {
        if(k == 0)
            lineToGoal.assign(wm->ball->pos, wm->field->oppGoalL()-Vector2D(0, 0.05));
        else if(k == 1)
            lineToGoal.assign(wm->ball->pos, wm->field->oppGoal());
        else if(k == 2)
            lineToGoal.assign(wm->ball->pos, wm->field->oppGoalR()+Vector2D(0, 0.05));
        for(int i=0; i < wm->opp.activeAgentsCount(); i++)
        {
            oppPosTemp = wm->opp.active(i)->pos;
            oppCircleTemp.assign(oppPosTemp, radiusTemp);
            //draw(ourCircleTemp,QColor(Qt::yellow));
            if(oppCircleTemp.intersection(lineToGoal, &point1, &point2))
            {
                return false;
            }
        }
        for(int i=0; i < wm->our.activeAgentsCount(); i++)
        {
            if(wm->our.active(i)->id == defenseClearIndex)
                continue;
            oppPosTemp = wm->our.active(i)->pos;
            oppCircleTemp.assign(oppPosTemp, radiusTemp);
            //draw(ourCircleTemp,QColor(Qt::yellow));
            if(oppCircleTemp.intersection(lineToGoal, &point1, &point2))
            {
                return false;
            }
        }
    }
    return true;
}


bool DefensePlan::isPathClear(Segment2D line, int myDefID, double vel)
{
    Vector2D ourPosTemp;
    Circle2D ourCircleTemp;
    Vector2D point1, point2;
    double radiusTemp;
    radiusTemp = 0.1 + ((vel > 8)? 0: (8-vel)/300);
    //draw(QString("active againt %1").arg(wm->our.activeAgentsCount()) , Vector2D(1,1.5) , "white");
    for(int i=0; i < wm->our.activeAgentsCount(); i++)
    {
        //draw(line,QColor(Qt::yellow));
        if(wm->our.active(i)->id == myDefID)
            continue;
        ourPosTemp = wm->our.active(i)->pos;
        ourCircleTemp.assign(ourPosTemp, radiusTemp);
        //draw(ourCircleTemp,QColor(Qt::yellow));
        if(ourCircleTemp.intersection(line, &point1, &point2))
        {
            //draw(QString("active againt %1").arg(vel) , Vector2D(1,1.5) , "white");
            return false;
        }
    }
    return true;
}


long DefensePlan::getCurrentTimeInMsec()
{
    // get time in miliseconds using QTime
    QTime tempTime;
    tempTime = tempTime.currentTime();
    return tempTime.msec()+ tempTime.second()*1000 + tempTime.minute()*60*1000 + tempTime.hour()*60*60*1000;
}

bool DefensePlan::isBallGoingToOppArea()
{
    if(wm->ball->vel.length() > 1.5)
    {
        Segment2D ballPathTemp(ballPos, ballPos + wm->ball->vel.norm()*10);
        Segment2D oppGoalieLine(wm->field->oppGoal()+Vector2D(0,5), wm->field->oppGoal()+Vector2D(0,-5));
        if(oppGoalieLine.intersection(ballPathTemp).valid())
        {
            double velTemp = getVelocityByPos().vel;
            if(velTemp > 2 && isBallGoingToOppAreaCnt == -1)
            {
                isBallGoingToOppAreaCnt = 0;
                return true;
            }
            if(isBallGoingToOppAreaCnt >= 0)
            {
                isBallGoingToOppAreaCnt++;
                return true;
            }
            if(isBallGoingToOppAreaCnt > 20)
            {
                isBallGoingToOppAreaCnt = -1;
                return false;
            }

        }
    }
    isBallGoingToOppAreaCnt = -1;
    return false;
}

//////// Mhmmd/////////For sending the positioning data of defences to coach////////

void DefensePlan::getDefencePoses(Vector2D *poses)
{
    for (int i=0 ; i < defenseAgents.count() ; i++) {
        poses[i] = defensePoints[i];
    }
}

///////////Arash.Z//////////Functions needed for goalie//////////

Vector2D DefensePlan::blockTheBall()
{

    if(knowledge->goalie != NULL){
        Segment2D ballSeg(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm()*10);
        Vector2D goaliePos(knowledge->goalie->pos());
        Vector2D target(ballSeg.nearestPoint(goaliePos));
        Segment2D goalLine(wm->field->ourGoal()+Vector2D(0,-0.6) , wm->field->ourGoal()+Vector2D(0,0.6));
        if(!wm->field->isInField(target))
            target = ballSeg.intersection(goalLine);
        return target;
    }
}

bool DefensePlan::isInThePenaltyArea(Vector2D _posofsth)
{
    double pos_x = _posofsth.x;
    double pos_y = _posofsth.y;
    double ashar = 0.000000000000001;
    double penaltyAreaEquation = pos_x + ((3.724*ashar)*pow(pos_y,5)) + (0.2302*pow(pos_y,4)) - ((6.889*ashar)*pow(pos_y,3)) + (0.1131*pow(pos_y,2)) + ((2.643*ashar)*pos_y);
    if (penaltyAreaEquation < -3.496 && wm->field->isInField(_posofsth))
        return true;
    else
        return false;
}

Vector2D DefensePlan::strictFollowBall(Vector2D _ballPos)
{
    //////////////////////// Variables of this function //////////////////////
    Vector2D ballPos = _ballPos;
    Vector2D i[2];
    Vector2D sol[2];
    Vector2D target(Vector2D(0,0));
    Segment2D goal2Ball;
    QList<Circle2D> defs;
    Circle2D theCircleAroundTheGoalCircle(wm->field->ourGoal()-Vector2D(0.3,0),1.78);
    Circle2D aimLessCircle(wm->field->ourGoal()-Vector2D(0.3,0),1.21);
    double AZBisecOpenAngle = 0,AZBigestOpenAngle = 0,AZDangerPercent = 0,aimLessChord = 0;
    double topFaceLength;
    double bottomFaceLength;
    double ballheight;
    bool dangerFlag = 0;
    double nearestDist2Ball;
    int nearestDef2BallId= -1 ;
    int g;
    //////////////////////////////////////////////////////////////////////////
    if(knowledge->goalie != NULL){
        ballPos = _ballPos;
        Segment2D goalLine(wm->field->ourGoal()+Vector2D(0,-0.8) , wm->field->ourGoal()+Vector2D(0,0.8));
        Segment2D downFieldLine(Vector2D(-_FIELD_WIDTH/2,-_FIELD_HEIGHT/2),Vector2D(-_FIELD_WIDTH/2,_FIELD_HEIGHT/2));
        //////////////////////////////// Appending circles on defense agents /////////////////////////////////////////
        for (g = 0; g < defenseAgents.count() ; g++){
            defs.append(Circle2D(defenseAgents[g]->pos(), CRobot::robot_radius_new));

            if(defenseAgents[g]->pos().dist(wm->ball->pos) < nearestDist2Ball) {
                nearestDef2BallId = defenseAgents[g]->id();
                nearestDist2Ball = defenseAgents[g]->pos().dist(wm->ball->pos);
            }
        }
        if(nearestDef2BallId != -1 && 0) {
            if(knowledge->getAgent(nearestDef2BallId)->pos().dist(wm->field->ourGoal()) > wm->ball->pos.dist(wm->field->ourGoal()) - noDefThr) {
                defs.clear();
                noDefThr = 0.15;
            }
            else {
                noDefThr = 0;
            }
        }
        else {
            noDefThr = 0;
        }
        draw(ballPrediction(true));
        //debug(QString("defs counter : %1").arg(defs.count()),D_SEPEHR);
        ///////////////////////////// Empty region between defense agents //////////////////////////
        knowledge->getEmptyAngle(ballPos, wm->field->ourGoalL(), wm->field->ourGoalR(), defs, AZDangerPercent, AZBisecOpenAngle, AZBigestOpenAngle,true);
        ////////// Bisector of triangle that is made up of with this points : [ballPossition , topGoal , bottom Goal]  //////////////////////////
        Segment2D AZBisecOpenSeg(ballPos , ballPos + Vector2D(cos(_PI*(AZBisecOpenAngle)/180),sin(_PI*(AZBisecOpenAngle)/180)).norm()*12);
        ////////// Top and bottom line of triangle that is made up of with this points : [ballPossition , topGoal , bottom Goal]  //////////////////////////
        Segment2D AZTopOfOpenSeg(ballPos , ballPos + Vector2D(cos(_PI*(AZBisecOpenAngle + (AZBigestOpenAngle/2))/180),sin(_PI*(AZBisecOpenAngle + (AZBigestOpenAngle/2))/180)).norm()*12);
        Segment2D AZBottomOfOpenSeg(ballPos , ballPos + Vector2D(cos(_PI*(AZBisecOpenAngle - (AZBigestOpenAngle/2))/180),sin(_PI*(AZBisecOpenAngle - (AZBigestOpenAngle/2))/180)).norm()*12);
        ///////////// Intersection of the top and bottom line of the triangle with goalLine ////////////////////////////////////////////
        Vector2D openAngGoalIntersectionTop(AZTopOfOpenSeg.intersection(goalLine));
        Vector2D openAngGoalIntersectionBottom(AZBottomOfOpenSeg.intersection(goalLine));
        /////////////////// Real length of top and bottom line of the triangle for talles //////////////////////////////////////////////////////////////
        topFaceLength = ballPos.dist(openAngGoalIntersectionTop);
        bottomFaceLength = ballPos.dist(openAngGoalIntersectionBottom);
        ///////////////////  top and bottom line of the triangle for talles for talles (with real length)//////////////////////////////////////////////////////////////
        Segment2D topFaceLength_forTalles(ballPos , ballPos + Vector2D(cos(_PI*(AZBisecOpenAngle + (AZBigestOpenAngle/2))/180),sin(_PI*(AZBisecOpenAngle + (AZBigestOpenAngle/2))/180)).norm()*topFaceLength);
        Segment2D bottomFaceLength_forTalles(ballPos , ballPos + Vector2D(cos(_PI*(AZBisecOpenAngle - (AZBigestOpenAngle / 2))/180),sin(_PI*(AZBisecOpenAngle - (AZBigestOpenAngle/2))/180)).norm()*bottomFaceLength);
        //////////////////////////Height of the triangle ///////////////////////////////////////////////////////
        ballheight = ballPos.dist(downFieldLine.nearestPoint(ballPos));
        Line2D aimLessLine(Vector2D(0,0),Vector2D(-1,-1));
        draw(AZBisecOpenSeg,"blue");
        goal2Ball.assign(wm->field->ourGoal(),wm->ball->pos);
        //////////////////////////// AHZ ////////////////////////
        //        ballPos = wm->ball->pos;
        //        if(!goalcir.contains(ballPos) && theCircleAroundTheGoalCircle.contains(ballPos)){
        //            dangerFlag = 1;
        //        }
        //        if(dangerFlag && wm->ball->vel.length() < 0.5){
        //            debug(QString("Danger State"),D_SEPEHR);
        //            aimLessCircle.intersection(goal2Ball,&sol[0],&sol[2]);
        //            target = (sol[0].dist(knowledge->goalie->pos()) < sol[1].dist(knowledge->goalie->pos()) ? sol[0] : sol[1]);
        //            //defs.clear();
        //            dangerFlag = 0;
        //        }
        /////////////////////////////////////////////////////////////
        //else{
        dangerFlag = 0;
        /////changes for RoboCup 2016////////in one def, goalie dont move to nearest point///////
#ifdef THEIR_DIRECT_CENTER
        if(knowledge->getGameState() == CKnowledge::TheirIndirectKick) {
            target = AZBisecOpenSeg.intersection(goalLine) + Vector2D(0.1,0.0);
            return target;
        }
#endif
        if(knowledge->goalie->pos().dist(AZBisecOpenSeg.nearestPoint(knowledge->goalie->pos())) > 0.2 + thr && defenseAgents.size() > 1){
            debug(QString("1"),D_SEPEHR);
            target = AZBisecOpenSeg.nearestPoint(knowledge->goalie->pos());
            draw(target);
            thr = 0;
        }
        else{
            thr = 0.2;
            if (topFaceLength < bottomFaceLength){
                aimLessChord = bottomFaceLength_forTalles.nearestPoint(openAngGoalIntersectionTop).dist(openAngGoalIntersectionTop);
                debug(QString("top koochik tar"),D_SEPEHR);
                aimLessLine = Line2D(bottomFaceLength_forTalles.nearestPoint(openAngGoalIntersectionTop),openAngGoalIntersectionTop);
                draw(Segment2D(bottomFaceLength_forTalles.nearestPoint(openAngGoalIntersectionTop),openAngGoalIntersectionTop), QColor(Qt::black));
            }
            else{
                aimLessChord = topFaceLength_forTalles.nearestPoint(openAngGoalIntersectionBottom).dist(openAngGoalIntersectionBottom);
                debug(QString("bottom koochik tar"),D_SEPEHR);
                aimLessLine = Line2D(topFaceLength_forTalles.nearestPoint(openAngGoalIntersectionBottom),openAngGoalIntersectionBottom);
                draw(Segment2D(topFaceLength_forTalles.nearestPoint(openAngGoalIntersectionBottom),openAngGoalIntersectionBottom),QColor(Qt::black));
            }
            if(aimLessChord > 2 * CRobot::robot_radius_new){
                debug(QString("chord > 2radus"),D_SEPEHR);
                aimLessLine = Line2D(Vector2D(ballPos.x-(.15*ballheight/aimLessChord),ballPos.y),Vector2D(ballPos.x-(.15*ballheight/aimLessChord),ballPos.y-0.1));
                draw(Segment2D(Vector2D(ballPos.x-(.15*ballheight/aimLessChord),ballPos.y),Vector2D(ballPos.x-(.15*ballheight/aimLessChord),ballPos.y-0.1)),QColor(Qt::black));
            }
            Segment2D AZBisecOpenSeg(ballPos , ballPos + Vector2D(cos(_PI*(AZBisecOpenAngle)/180),sin(_PI*(AZBisecOpenAngle)/180)).norm()*12);
            if(AZBisecOpenSeg.intersection(aimLessLine).isValid()){
                ////////////////////////////////////////// AHZ /////////////////////////////

                if(Vector2D::angleOf(wm->ball->pos,wm->field->ourGoal(),wm->field->ourGoalL()).degree() < 10 + angleDegreeThrNotStop){
                    target = wm->field->ourGoalL() + Vector2D(0.12,0);
                    angleDegreeThrNotStop = 5;
                }
                else if(Vector2D::angleOf(wm->ball->pos,wm->field->ourGoal(),wm->field->ourGoalR()).degree() < 10 + angleDegreeThrNotStop){
                    target = wm->field->ourGoalR() + Vector2D(0.12,0);
                    angleDegreeThrNotStop = 5;
                }
                else {
                    target = AZBisecOpenSeg.intersection((aimLessLine));
                    angleDegreeThrNotStop = 0;
                }

                //////////////////////////////////////////////////////////////////////////
                if(isInThePenaltyArea(target)){
                    debug(QString("penalty"),D_SEPEHR);
                    draw(target);
                }
                else{
                    target = getIntersectionWithPenaltyAreaGk(AZBisecOpenSeg);
                    //////////// Added By AHZ ///////////////
                    draw(target);
                }
            }
            else{
                debug(QString("3"),D_SEPEHR);
                target = AZBisecOpenSeg.intersection(goalLine) + Vector2D(0.1,0.0);
            }
        }
        if (!wm->field->isInField(target) || target.x < -4.4){

            debug(QString("4"),D_SEPEHR);
            target = AZBisecOpenSeg.intersection(goalLine) + Vector2D(0.1,0.0);
        }
    }
    return target;
    // }
}

Vector2D DefensePlan::followBall(Vector2D _ballPos)
{
    ////////////////////////////// Variables of this function //////////////////////////
    Vector2D ballPos;
    Vector2D i[2];
    Vector2D target(Vector2D(0,0));
    QList<Circle2D> defs;
    double AZBisecOpenAngle = 0 , AZBigestOpenAngle = 0, AZDangerPercent = 0;
    double goalCirRad = 0.45;
    int g=0;
    ////////////////////////////////////////////////////////////////
    if(knowledge->goalie != NULL){
        Segment2D goalLine(wm->field->ourGoal() + Vector2D(0,-0.6) , wm->field->ourGoal() + Vector2D(0,0.6));
        ballPos = _ballPos;
        for (g = 0; g < defenseAgents.count() ; g++){
            defs.append(Circle2D(defenseAgents[g]->pos(), CRobot::robot_radius_new));
        }

        knowledge->getEmptyAngle(ballPos, wm->field->ourGoalL(), wm->field->ourGoalR(), defs, AZDangerPercent, AZBisecOpenAngle, AZBigestOpenAngle,false);
        Segment2D AZBisecOpenSeg(ballPos , ballPos + Vector2D(cos(_PI*(AZBisecOpenAngle)/180),sin(_PI*(AZBisecOpenAngle)/180)).norm()*12);

        goalCirRad = (ballPos.dist(wm->field->ourGoal()) / 10) + 0.35;
        if (goalCirRad < 0.45) goalCirRad = 0.45;
        if (goalCirRad > 0.8) goalCirRad = 0.80;
        Circle2D goalcir(wm->field->ourGoal(),goalCirRad);
        if (knowledge->goalie->pos().dist(AZBisecOpenSeg.nearestPoint(knowledge->goalie->pos())) > 0.1 + thr){
            debug(QString("Intersection Bisec and nearest Point"),D_SEPEHR);
            target = AZBisecOpenSeg.nearestPoint(knowledge->goalie->pos());
            draw(target);
            thr = 0.0;
        }
        else{
            thr = 0.1;
            debug(QString("StrictFollow"),D_SEPEHR);
            target = strictFollowBall(ballPos);
        }
        if(!goalcir.contains(target)){
            goalcir.intersection(AZBisecOpenSeg,&i[0],&i[1]);
            if(!wm->field->isInField(ballPos)){
                debug(QString("not containing goal circle"),D_SEPEHR);
                target = (i[0] + i[1]) / 2 + Vector2D(0.15,0);
            }
            else{
                debug(QString("not containing goal circle 1"),D_SEPEHR);
                target = wm->field->isInField(i[0]) ? i[0] : i[1];
            }
        }
        if(!wm->field->isInField(target)){
            debug(QString("Is not in field"),D_SEPEHR);
            target = AZBisecOpenSeg.intersection(goalLine);
        }
        draw(target);
        draw(goalcir,0,360);
        draw(AZBisecOpenSeg);

        return target;

    }
}


int DefensePlan::predictMostDangrousOppToBall(){
    //////////////////////////////////////////////////////// GOALIE CHECK ONE TOUCHE
    // make it to a function ... oppRoles
    // vector2D(-100,100)----> shows Numbers
    // assign opponents in their positions. depends on values of opponents
    if(wm->opp.activeAgentsCount()>0){
        Vector2D OppPos[wm->opp.activeAgentsCount()];
        for(int i=0; i < wm->opp.activeAgentsCount();i++){
            OppPos[i] = wm->opp.active(i)->pos;
        }
        ////////////////////////////////////////////// detect pass or shoot (peredict)
        // check which one of the variables could be pointer and use them
        Vector2D OppPosProjectionsOnBallSegmentByVel[wm->opp.activeAgentsCount()];
        Segment2D BallSegmentByVel(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm()*10);

        double DistanceFromProjectionsToBallPos[wm->opp.activeAgentsCount()];
        double DistanceFromOppPosToProjections[wm->opp.activeAgentsCount()];
        double BallVelocity= wm->ball->vel.length();
        double MinDistanceToBallPosY=10000;
        double  DistanceToBallPosY[wm->opp.activeAgentsCount()];

        int MinDistanceToBallPosYRoboID = 100;

        bool CheckIntersectionsOfOppVelocityByBallSegment[wm->opp.activeAgentsCount()];
        bool CheckIntersectionsOfOppPosByBallSegment[wm->opp.activeAgentsCount()];

        for(int i=0;i < wm->opp.activeAgentsCount();i++)
        {
            OppPosProjectionsOnBallSegmentByVel[i]=NearestDistanceToBallSegment(OppPos[i]);
            DistanceFromProjectionsToBallPos[i]=(wm->ball->pos -OppPosProjectionsOnBallSegmentByVel[i]).length();
            DistanceFromOppPosToProjections[i]=(OppPosProjectionsOnBallSegmentByVel[i] - OppPos[i]).length();

            Segment2D OppSegmentByvel(wm->opp.active(i)->pos,wm->opp.active(i)->pos +wm->opp.active(i)->vel.norm()*10);
            Segment2D OppPosTotheirProjections(OppPosProjectionsOnBallSegmentByVel[i],OppPos[i]);
            Segment2D OppSegmentByDir(wm->opp.active(i)->pos,wm->opp.active(i)->pos + wm->opp.active(i)->dir*10);


            if(OppPosTotheirProjections.intersection(BallSegmentByVel).isValid()){ // is front to ball

                CheckIntersectionsOfOppPosByBallSegment[i]=true;

                if(wm->opp.active(i)->vel.length() > 0.5)// is moving
                {

                    if(OppSegmentByvel.intersection(BallSegmentByVel).valid() && DistanceFromProjectionsToBallPos[i] * BallVelocity/DistanceFromOppPosToProjections[i]>5)
                    {
                        // find the lowest amount of distToBallY
                        DistanceToBallPosY[i] = fabs(OppPos[i].y - wm->ball->pos.y);
                        if(MinDistanceToBallPosY > DistanceToBallPosY[i]){
                            MinDistanceToBallPosY=DistanceToBallPosY[i];
                            MinDistanceToBallPosYRoboID=wm->opp.active(i)->id;
                        }

                    }

                }

                else// is n't moving
                {

                    if(OppSegmentByDir.intersection(BallSegmentByVel).valid() && DistanceFromOppPosToProjections[i] < 0.3)
                    {
                        //
                        DistanceToBallPosY[i] = fabs(wm->opp.active(i)->pos.y - wm->ball->pos.y);

                        if(DistanceToBallPosY[i] < MinDistanceToBallPosY){
                            MinDistanceToBallPosY=DistanceToBallPosY[i];
                            MinDistanceToBallPosYRoboID=wm->opp.active(i)->id;
                        }


                    }
                    //
                }
            }
            else    CheckIntersectionsOfOppPosByBallSegment[i]=false;// is behind to ball
        }
        return MinDistanceToBallPosYRoboID;
    }
}

Vector2D DefensePlan::NearestDistanceToBallSegment(Vector2D point)
{
    //  if(wm->ball->vel.length() < 1){
    Segment2D OurGoalAreaLine(wm->field->ourGoal()+Vector2D(0,-0.6) , wm->field->ourGoal()+Vector2D(0,0.6));
    Segment2D Ballsegment(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm()*10);
    Vector2D  OurGoalAreaLineIntersectionByBallSegment(OurGoalAreaLine.intersection(Ballsegment));
    Vector2D NearestTarget = OurGoalAreaLine.nearestPoint(point);
    if(OurGoalAreaLineIntersectionByBallSegment.valid())
        return NearestTarget;
    //}
}

//////////////////////////Defense Functions/////////////////////

void DefensePlan::checkDefenseExeptions()
{
    if(defenseAgents.size() > 0) {
        setPointToKick();
        doOneTouch = defenseOneTouchOrNot();
        doClear = defenseClearOrNot();
        bool forceBeingInOneTouch = checkStillBeingInOneTouch(false);
        if(forceBeingInOneTouch)
            oneTouchPointFlag = false;
        if( doOneTouch || forceBeingInOneTouch){
            defExeptions.active = true;
            defExeptions.exeptionMode = defOneTouch;
        }
        else if((defenseClearOrNot() || (lastOneTouchClearState == ClearState && histOneTouchClearCnt < 30 )) && !policy()->Defense_NoClear() && wm->ball->pos.dist(wm->field->ourGoal()) > 1.4){
            if(defenseClearOrNot()) {
                lastOneTouchClearState = ClearState;
                histOneTouchClearCnt = 0;
            }
            if(isBallGoingToOppArea()) {
                lastOneTouchClearState = NoState;
            }
            histOneTouchClearCnt++;
            defExeptions.active = true;
            defExeptions.exeptionMode = defClear;
            defClearThr = 0.5;

        }
        else {
            defExeptions.active = false;
            defExeptions.exeptionMode = NoneExep;
            defClearThr = 0;
        }

    }
}

void DefensePlan::runDefenseExeptions()
{
    if(defenseAgents.size() > 0){
        Vector2D agentTarget;
        if (defExeptions.exeptionMode == defOneTouch) {
            agentTarget = runDefenseOneTouch();
            draw(QString("Defense OneTouch"), Vector2D(0,2), "red");

            if(agentTarget.x != -100) {
                draw(QString("Defense OneTouch"), Vector2D(0,2), "red");
                assignSkill( defenseAgents.at(oneToucher) , gpa[defenseAgents.at(oneToucher)->id()]);
                gpa[defenseAgents.at(oneToucher)->id()]->init(defenseTargets[oneToucher], defenseTargets[oneToucher] - wm->field->ourGoal());
                draw(Circle2D(defenseTargets[0] , 0.05) , 0 , 360 , "black" , true);

                defExeptions.exepAgentId = defenseAgents.at(oneToucher)->id();
            }
        }
        else if(defExeptions.exeptionMode == defClear) {
            draw(QString("Defense Clear"), Vector2D(0,2), "red");
            if (defenseClearIndex != -1) {
                if(defExeptions.exepAgentId == -1 ) {
                    defExeptions.exepAgentId = defenseAgents.at(defenseClearIndex)->id();
                }
                else if (defExeptions.exepAgentId != defenseAgents.at(defenseClearIndex)->id()){
                    defenseClearIndex = (defenseClearIndex == 0) ? 1 : 0;

                }
            }
            if(defExeptions.exepAgentId != -1 && defenseClearIndex == -1){
                defenseClearIndex = (defenseAgents.at(0)->id() == defExeptions.exepAgentId) ? 0 : 1;
            }
            runClear();
            executeSkill[defenseClearIndex] = false;
        }
    }
}

Vector2D DefensePlan::runDefenseOneTouch()
{
    if(defenseAgents.size() > 0){
        if(!oneTouchPointFlag)
        {
            //save targets in order not to switch target rapidaly
            defensePoints[oneToucher] = pointForKick;
            defenseDirs[oneToucher] = oneToucherDir;
            oneTouchPoint[0] = pointForKick;
            oneTouchPoint[1] = pointForKick;
            oneTouchPointFlag = true;
            calcPointForOneTouch();

            defenseTargets[oneToucher] = defensePoints[oneToucher];
            defenseTargets[oneToucher] = checkDefensePoint(defenseAgents[oneToucher], defenseTargets[oneToucher]);
            return defenseTargets[oneToucher];
        }
        return Vector2D(-100,100);
    }
}

bool DefensePlan::defenseOneTouchOrNot(){
    if( defenseAgents.size() > 0 ){
        QList <int> otherAgents;
        otherAgents.clear();
        if( oneToucher < 0 || oneToucher > defenseAgents.size()-1 ){
            oneToucher = 0;
        }
        for (int i=0; i < defenseAgents.size(); i++) {
            if(i != oneToucher) {
                otherAgents.append(i);
            }
        }
        for(int i=0; i < otherAgents.size(); i++) {
            Segment2D otherAgentIntersect(wm->ball->pos , pointForKick);
            Vector2D sol1 , sol2;
            Circle2D circ(defenseAgents.at(otherAgents[i])->pos() , CRobot::robot_radius_new);
            int num = circ.intersection(otherAgentIntersect , &sol1 , &sol2);
            Segment2D def2seg(defensePoints[oneToucher],defensePoints[otherAgents[i]]);
            Line2D ballWay(wm->ball->pos,wm->ball->vel+wm->ball->pos);
            if(def2seg.intersection(ballWay).valid() && def2seg.length() <0.19)
                return false;

            if( pointForKick.valid() && (pointForKick.dist(defenseAgents.at(oneToucher)->pos())-0.2 > pointForKick.dist(defenseAgents.at(otherAgents[i])->pos()) || (num && defenseAgents.at(oneToucher)->pos().dist(defenseAgents.at(otherAgents[i])->pos()) > 0.19) ) ){
                int temp = oneToucher;
                oneToucher = otherAgents[i];
                otherAgents[i] = temp;
                setPointToKick();
            }
            else if( pointForKick.valid() == false ){
                int temp = oneToucher;
                oneToucher = otherAgents[i];
                otherAgents[i] = temp;
                setPointToKick();
                if( pointForKick.valid() == false ){

                    return false;
                }
            }
        }
    }

    bool oneTouchFlag = false;
    Segment2D ballLine = Segment2D(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm()*10);
    Segment2D goalLine(wm->field->ourGoal()+Vector2D(0,1) , wm->field->ourGoal()+Vector2D(0,-1));
    Vector2D goalLineIntersect = goalLine.intersection(ballLine);
    if( goalLineIntersect.valid() ){
        Segment2D secondGoalLine(wm->field->ourGoal()+Vector2D(0,0.7) , wm->field->ourGoal()+Vector2D(0,-0.7));
        Vector2D goalLineIntersect = secondGoalLine.intersection(ballLine);
        bool ballDistVelFlag = defenseCheckBallDangerForOneTouch();
        bool isItClearInFrontOfBall = knowledge->isPointClear(pointForKick , wm->ball->pos , 0.025);
        if( ballDistVelFlag && isItClearInFrontOfBall ){

            if( goalLineIntersect.valid() ){
                oneTouchFlag = true;
            }
            else{

                Vector2D *crossDefenseArea = getIntersectWithDefenseArea(ballLine , wm->ball->pos);
                if( crossDefenseArea != NULL && crossDefenseArea->valid() ){
                    oneTouchFlag = true;
                }
            }
        }
    }
    if(oneTouchFlag){
        if(!isInOneTouch)
        {
            velAndAccByKK temp = getVelocityByPos();
            isInOneTouch = true;
            double ballDist = wm->ball->pos.dist(pointForKick);
            double cycle =  ballDist/temp.vel;
            oneTouchCycleTest = cycle/LOOP_TIME_BYKK;
        }
    }
    return oneTouchFlag;
}



/////////End of Arash.z/////////////////////////////////////////


/////////////////////// KK Def Positioning Funcs///////////////

CDefPos::CDefPos()
{
    penaltyAreaOffset = 0.2;
    penaltyAreaRadius = 1.33;
    penaltyAreaCircle.assign(wm->field->ourGoal()-Vector2D(penaltyAreaOffset, 0.001), penaltyAreaRadius);
    oneDefThr = 0;
    isNearPenaltyArea = false;
}


Vector2D CDefPos::getXYByAngle(double _angle, double _radius)
{
    double tempX;
    double tempY;
    _angle += _PI/2;
    tempX = _radius*cos(_PI-_angle);
    tempY = _radius*sin(_angle);
    tempX += wm->field->ourGoal().x - penaltyAreaOffset;
    if (tempX < wm->field->ourGoal().x + 0.08) {
        tempX = wm->field->ourGoal().x + 0.08;
    }
    return Vector2D(tempX, tempY);
}


double CDefPos::getRobotAngle(double _radius)
{
    return atan(CRobot::robot_radius_old/_radius)*2;
}


double CDefPos::getAngleByXY(Vector2D _point)
{
    double w, h;
    w = wm->field->ourGoal().y - _point.y;
    h = _point.x - (wm->field->ourGoal().x - penaltyAreaOffset);
    return _PI-atan2(h, w);
}


kk2Angles CDefPos::getIntersections(Vector2D _ballPos, double _radius)
{
    kk2Angles tempAngles;
    Vector2D inter1, inter2, inter3;
    Circle2D tempCircle(wm->field->ourGoal()-Vector2D(penaltyAreaOffset, 0), _radius);
    if (tempCircle.contains(_ballPos)) {
        Segment2D tempSegment(wm->field->ourGoal(),
                              (_ballPos-wm->field->ourGoal()).norm()*3+(wm->field->ourGoal()-Vector2D(penaltyAreaOffset, 0)));
        tempCircle.intersection(tempSegment, &inter3, &inter1);
        _ballPos = inter3;
    }
    Segment2D tempSeg1(wm->field->ourGoalL(), _ballPos);
    Segment2D tempSeg2(wm->field->ourGoalR(), _ballPos);
    if(isNearPenaltyArea) {
        inter1 = getIntersectionWithPenaltyAreaDef(_radius , tempSeg1);
        inter2 = getIntersectionWithPenaltyAreaDef(_radius , tempSeg2);
        nearRadius[0] = (wm->field->ourGoal() - Vector2D(penaltyAreaOffset,0)).dist(inter1);
        nearRadius[1] = (wm->field->ourGoal()- Vector2D(penaltyAreaOffset,0)).dist(inter2);
    }

    else {
        tempCircle.intersection(tempSeg1, &inter1, &inter3);
        tempCircle.intersection(tempSeg2, &inter2, &inter3);
    }

    draw(inter1,0,QColor(Qt::red));
    draw(inter2,0,QColor(Qt::red));
    draw(tempSeg1, QColor(Qt::black));
    draw(tempSeg2, QColor(Qt::black));

    tempAngles.angle1 = getAngleByXY(inter1);
    tempAngles.angle2 = getAngleByXY(inter2);
    if (tempAngles.angle1 > tempAngles.angle2) {
        double tempSwap;
        tempSwap = tempAngles.angle1;
        tempAngles.angle1 = tempAngles.angle2;
        tempAngles.angle2 = tempSwap;
    }
    return tempAngles;
}


double CDefPos::getBestRadiusBySize(double _openAngle, int _size)
{
    double defWidth = CRobot::robot_radius_old*_size*2;
    return tan(_openAngle)/defWidth;
}


kkDefPos CDefPos::getDefPositions(Vector2D _ballPos, int _size, double _limit1, double _limit2)
{
    kkDefPos tempDefPos;
    tempDefPos.size = _size;
    if (_size <= 0) {
        return tempDefPos;
    }
    double ballDistLimit = _ballPos.dist(wm->field->ourGoal())/2;
    double tempBestRadius = ballDistLimit;
    if(findBestRadius(tempDefPos.size) != -1){
        tempBestRadius = findBestRadius(tempDefPos.size);
        isNearPenaltyArea = false;
    }
    if (tempBestRadius > _limit2) {
        tempBestRadius = _limit2;
        isNearPenaltyArea = false;
    }

    else if (tempBestRadius < _limit1) {
        tempBestRadius = _limit1;
        isNearPenaltyArea = true;
    }
    else {
        isNearPenaltyArea = false;
    }

    kk2Angles tempAngles = getIntersections(_ballPos, tempBestRadius);
    double tempOpenAngle = fabs(tempAngles.angle2 - tempAngles.angle1);
    //double tempBestRadius = getBestRadiusBySize(tempOpenAngle, _size);

    draw(QString::number(tempBestRadius), Vector2D(-1, _FIELD_HEIGHT/2 - 0.2));
    draw(QString::number(tempOpenAngle), Vector2D(-1, _FIELD_HEIGHT/2 - 0.4));
    double agentAngle = getRobotAngle(tempBestRadius);
    double openAngleAfterPositioning = tempOpenAngle - agentAngle*_size;

    if (openAngleAfterPositioning > 0) {
        tempDefPos.overDef = 0;
        //        draw("POS", Vector2D(-1, _FIELD_HEIGHT/2 - 0.6));
        if (_size <= 1) {
            if(isNearPenaltyArea) {
                tempBestRadius = nearRadius[0];
            }
            if (_ballPos.y < 0 + oneDefThr) {
                tempDefPos.pos[0] = getXYByAngle(tempAngles.angle1+agentAngle/2, tempBestRadius);
                oneDefThr = 1;
            }
            else {
                tempDefPos.pos[0] = getXYByAngle(tempAngles.angle2-agentAngle/2, tempBestRadius);
                oneDefThr = -1;
            }
        }
        else if(_size == 2 && isNearPenaltyArea){
            draw(QString("Near"),Vector2D(0,-2),QColor(Qt::red));
            double angleOffset = openAngleAfterPositioning/(_size-1);
            double defAngle = tempAngles.angle1 + agentAngle/2;
            for (int i = 0; i < _size; i++) {
                tempBestRadius = nearRadius[i];
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
        }
        else {
            oneDefThr = 0;
            double angleOffset = openAngleAfterPositioning/(_size-1);
            double defAngle = tempAngles.angle1 + agentAngle/2;
            for (int i = 0; i < _size; i++) {
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
        }
    }
    else {
        oneDefThr = 0;
        tempDefPos.overDef = (agentAngle*_size/tempOpenAngle) - 1;
        //        draw("NEG", Vector2D(-1, _FIELD_HEIGHT/2 - 0.6));
        if (_size <= 1) {
            tempDefPos.pos[0] = getXYByAngle((tempAngles.angle1+tempAngles.angle2)/2, tempBestRadius);
        }
        else {
            double angleOffset = _PI/720;
            double defAngle = (tempAngles.angle1 + openAngleAfterPositioning/2) + agentAngle/2;
            for (int i = 0; i < _size; i++) {
                tempDefPos.pos[i] = getXYByAngle(defAngle, tempBestRadius);
                defAngle += angleOffset + agentAngle;
            }
        }
    }
    return tempDefPos;
}

Vector2D CDefPos::getIntersectionWithPenaltyAreaDef(double _tempBestRadius , Segment2D _seg)
{
    ////////////////////////////////////////////////
    Vector2D ins[2];
    Vector2D finter;
    Vector2D fOurGoal(- _FIELD_WIDTH/2.0 , 0.0);
    double PAreaOffset = _tempBestRadius - 1.37;
    if(PAreaOffset < 0.12) {
        PAreaOffset = 0.12;
    }
    Circle2D c1(wm->field->ourGoal() + Vector2D(0,-_GOAL_WIDTH/4),_GOAL_RAD + PAreaOffset);
    Circle2D c2(fOurGoal + Vector2D(0,+_GOAL_WIDTH/4),_GOAL_RAD + PAreaOffset);
    Rect2D r(fOurGoal + Vector2D(+0,-_GOAL_WIDTH/4),fOurGoal + Vector2D(+_GOAL_RAD + PAreaOffset,+_GOAL_WIDTH/4));
    draw(c1, 0, 90,QColor(Qt::black));
    //    draw(c2, 90, 180,QColor(Qt::black));
    draw(r,QColor(Qt::black));
    ////////////////////////////////////////////////
    r.intersection(_seg,&ins[0],&ins[1]);
    if(ins[0].valid() || ins[1].valid()) {
        finter = (ins[0].x > ins[1].x) ? ins[0] : ins[1];

        if(finter.x > -3.42) {
            draw(QString("rect"),Vector2D(0,-2),"red");
            draw(finter);
            return finter;
        }
    }
    c1.intersection(_seg,&ins[0],&ins[1]);
    if(((wm->field->isInField(ins[0])) && (ins[0].y <= -_GOAL_WIDTH/4) && ins[0].valid()) || ((wm->field->isInField(ins[1]) && ins[1].y <= -_GOAL_WIDTH/4 && ins[1].valid()))) {
        if(!wm->field->isInField(ins[1]))
            finter = ins[0];
        else if(!wm->field->isInField(ins[0]))
            finter = ins[1];
        else
            finter = (ins[0].x > ins[1].x) ? ins[0] : ins[1];

        draw(QString("c1"),Vector2D(0,-2),"red");
        draw(finter);
        return finter;
    }
    c2.intersection(_seg,&ins[0],&ins[1]);
    if(!wm->field->isInField(ins[1]))
        finter = ins[0];
    else if(!wm->field->isInField(ins[0]))
        finter = ins[1];
    else
        finter = (ins[0].x > ins[1].x) ? ins[0] : ins[1];
    draw(QString("c2"),Vector2D(0,-2),"red");
    draw(finter);
    return finter;
}


void DefensePlan::executeGoalie()
{
    if (goalieAgent != NULL){
        if(savedClearPos.valid() && clearCnt > 30) {
            savedClearPos.invalidate();
            clearCnt = 0;
        }
        else if(savedClearPos.valid() && clearCnt <= 30){
            clearCnt++;
        }
        else {
            clearCnt = 0;
        }
        // edited by AHZ
        // ommite the setChip(CHIP_POWER)
        if (goalieClearMode) {
            goalieAgent->setChip(LONG_CHIP_POWER);
        }
        else if (knowledge->isCrowdedInFrontOfAgent(goalieAgent->id(),0.5) && goalieClearMode) {
            goalieAgent->setChip(0); // edited by AHZ
        }
        Rect2D fieldRect(Vector2D(- _FIELD_WIDTH/2.0 , - _FIELD_HEIGHT/2.0)+Vector2D(-0.005,-0.005),Vector2D(_FIELD_WIDTH/2.0 , _FIELD_HEIGHT/2.0)+Vector2D(+0.005,+0.005));
        Line2D ballPrGoalLine(wm->ball->pos, Vector2D(wm->ball->pos.x,(wm->ball->pos.y + 0.01)));
        Vector2D solut[2];
        fieldRect.intersection(ballPrGoalLine, &solut[0], &solut[1]);
        if(ballBehindGoalie){
            assignSkill(goalieAgent, kickSkill);
            kickSkill->setKickSpeed(1023);
            kickSkill->setTolerance(1.5);
            kickSkill->setDontKick(false);
            kickSkill->setTarget(wm->field->oppGoal());
            kickSkill->setSlow(false);
            kickSkill->setSpin(false);
            kickSkill->setChip(false);
            kickSkill->setAvoidPenaltyArea(false);
            kickSkill->setGoalieMode(true);
            kickSkill->setTarget(wm->field->oppGoal());

        }
        else if(besidePoleFlag) {
            Vector2D noKickTarget = (solut[0].dist(wm->ball->pos) < solut[1].dist(wm->ball->pos)) ? solut[0] : solut[0];
            assignSkill(goalieAgent, kickSkill);
            kickSkill->setKickSpeed(kickSkill->getAgent()->kickSpeedValue(7.2,false));
            kickSkill->setTolerance(1.5);
            kickSkill->setDontKick(true);
            kickSkill->setTarget(noKickTarget);
            kickSkill->setSlow(true);
            kickSkill->setSpin(false);
            kickSkill->setChip(false);
            kickSkill->setAvoidPenaltyArea(false);
            kickSkill->setGoalieMode(true);


        }
        else if(goalieClearMode && !dangerForGoalieClear) {
            assignSkill(goalieAgent, kickSkill);
            kickSkill->setKickSpeed(kickSkill->getAgent()->kickSpeedValue(7.2,false));
            kickSkill->setTolerance(1.5);
            kickSkill->setDontKick(false);
            kickSkill->setTarget(wm->field->oppGoal());
            kickSkill->setSlow(false);
            kickSkill->setSpin(false);
            kickSkill->setChip(false);
            kickSkill->setAvoidPenaltyArea(false);
            kickSkill->setGoalieMode(true);
            if(!isPathToOppGoalieClear())
            {
                if(!savedClearPos.valid())
                    savedClearPos = findBestPointForChipTarget(savedClearDist,1);

                kickSkill->setTarget(savedClearPos);
                kickSkill->getAgent()->chipDistanceValue(savedClearDist, false);
                kickSkill->setChip(true);
            }

        }
        else {
            assignSkill( goalieAgent , gpa[goalieAgent->id()]);
            if (goalieOneTouch) {
                gpa[goalieAgent->id()]->setSlowMode(false);
                gpa[goalieAgent->id()]->setADiveMode(true);
                Vector2D targetDir(0, goalieAgent->pos().y);
                gpa[goalieAgent->id()]->init(goalieTarget, wm->ball->pos - goalieTarget);
            }
            if(dangerForGoalieClear) {
                // edited by AHZ
                //                gpa[goalieAgent->id()]->setSlowMode(true);
                //                gpa[goalieAgent->id()]->setADiveMode(false);
                //                goalieAgent->setChip(0);
                gpa[goalieAgent->id()]->init(goalieTarget,  wm->ball->pos - goalieTarget); //edited by AHZ
            }
            else {
                gpa[goalieAgent->id()]->setSlowMode(false);
                gpa[goalieAgent->id()]->setADiveMode(false);
                gpa[goalieAgent->id()]->init(goalieTarget, wm->ball->pos - goalieTarget);

            }

            gpa[goalieAgent->id()]->setAvoidPenaltyArea(false);
            gpa[goalieAgent->id()]->setNoAvoid(true);
            gpa[goalieAgent->id()]->execute();
            draw(Circle2D(goalieTarget , 0.05) , 0 , 360 , "black" , true);
        }
    }
}

int DefensePlan::decideNumOfMarks(double _overDef)
{
    Vector2D BallPos= wm->ball->pos;
    Vector2D ourGoal = wm->field->ourGoal();
    Vector2D leftCorner = wm->field->ourCornerL();
    Vector2D rightCorner = wm->field->ourCornerR();
    bool playOn = knowledge->getGameMode() == CKnowledge::Start;
    bool playOff = ((knowledge->getGameState() == CKnowledge::TheirDirectKick)
                    || (knowledge->getGameState() == CKnowledge::TheirIndirectKick)
                    );
    if(defenseCount > 0) {
        if (playOff) {
            return decideNumOfMarksInPlayOff(defenseCount);
        } else if(knowledge->transientFlag) {
            return defenseCount - 1;
        } else if(playOn) {
            if((Vector2D::angleOf(BallPos,ourGoal,leftCorner).abs() < 20 + overDefThr
                ||Vector2D::angleOf(BallPos,ourGoal,rightCorner).abs() < 20 + overDefThr)
                    && defenseCount > 1 && !Circle2D((wm->field->ourGoal() - Vector2D(0.2,0)),1.60).contains(wm->ball->pos)) {
                overDefThr = 5;
                return 1;
            } else {
                overDefThr = 0;
            }
        }

    }
    return 0;
}



///////////////////////////////HMD/////////////////////////////////////////
void DefensePlan::markPosRefinePlayoff()
{
    Vector2D sol1, sol2, sol;
    Line2D tempLine(Vector2D(0,0), Vector2D(1,0));
    Circle2D _markAreaRestricted(wm->field->ourGoal(), markRadiusStrict);
    for(int i = 0; i < markPoses.count(); i++)
    {
        tempLine.assign(wm->field->ourGoal(), markPoses[i]);
        if((wm->field->ourGoal() - markPoses[i]).length() < markRadius)
        {
            _markAreaRestricted.intersection(tempLine, &sol1, &sol2);

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

    }



}

bool DefensePlan::checkIndirectAreaPass(Vector2D opp){
    double indirectAvoidRadius = 0.5 + 0.1;
    Circle2D indirectAvoidCircle(wm->ball->pos, indirectAvoidRadius);

    if (indirectAvoidCircle.contains(PassBlockRatio(segmentperpass, opp).first()) && !knowledge->transientFlag)
        return 1;
    else
    {
        return 0;
    }
}

bool DefensePlan::checkIndirectAreaShoot(Vector2D opp) {
    //if that point be in Indirect Area we will return 1


    //Circle2D MarkAreaStrict(wm->field->ourGoal(), markRadiusStrict);
    double indirectAvoidRadius = 0.5 + 0.1;
    Circle2D indirectAvoidCircle(wm->ball->pos, indirectAvoidRadius);


    if (indirectAvoidCircle.contains(ShootBlockRatio(segmentpershoot, opp).first()) && !knowledge->transientFlag)
        return 1;
    else
    {
        return 0;
    }
}


QList<Vector2D> DefensePlan::indirectAvoidShoot(Vector2D opp) {
    Segment2D tempseg;
    tempseg.assign(opp, wm->field->ourGoal());
    double indirectAvoidRadius = 0.5 + .1;
    Circle2D indirectAvoidCircle(wm->ball->pos,indirectAvoidRadius);
    Vector2D sol1, sol2, sol;
    indirectAvoidCircle.intersection(tempseg, &sol1, &sol2);
    if((wm->field->ourGoal() - sol1).length() > (wm->field->ourGoal() - sol2).length())
    {
        sol = sol2;
    }
    else
    {
        sol = sol1;
    }
    QList<Vector2D> temp;
    temp.clear();
    temp.append(sol);temp.append(wm->ball->pos - wm->field->ourGoal());
    return temp;
}

QList<Vector2D> DefensePlan::indirectAvoidPass(Vector2D opp) {
    Segment2D tempseg;
    tempseg.assign(wm->ball->pos, opp);
    double indirectAvoidRadius = 0.5 + .1;
    Circle2D indirectAvoidCircle(wm->ball->pos,indirectAvoidRadius);
    Vector2D sol1, sol2, sol;
    indirectAvoidCircle.intersection(tempseg, &sol1, &sol2);
    if(sol1.valid())
    {
        sol = sol1;
    }
    else if(sol2.valid())
    {
        sol = sol2;
    }
    QList<Vector2D> temp;
    temp.clear();
    temp.append(sol);temp.append(wm->ball->pos - opp);
    return temp;
}

void DefensePlan::findPos(int _markAgentSize)
{
    bool playOn = knowledge->getGameMode() == CKnowledge::Start;
    bool playOff = ((knowledge->getGameState() == CKnowledge::TheirDirectKick)
                    || (knowledge->getGameState() == CKnowledge::TheirKickOff)
                    || (knowledge->getGameState() == CKnowledge::TheirIndirectKick)
                    );
    ///////////////// Man To Man AllTransiant Mode for Mark ////////////////////
    if(policy()->Mark_ManToManAllTransiant())
    {
        if(knowledge->transientFlag)
            segmentpershoot = 0.05;
        else
            segmentpershoot = policy()->Mark_ShootRatioBlock() / 100;
    }
    else{
        segmentpershoot = policy()->Mark_ShootRatioBlock() / 100;
        segmentperpass = (100 - policy()->Mark_PassRatioBlock()) / 100;
    }
    ///////////// Man To Man PlayOff Mode for Mark//////////////////////////////
    double xLimitForblockingPass = 1;
    if(policy()->Mark_PlayOffManToMan() || wm->ball->pos.x > xLimitForblockingPass){
        if(playOff){
            manToManMarkInPlayOffBlockPass(oppAgentsToMarkPos,_markAgentSize , policy()->Mark_PassRatioBlock() / 100);
        }
        else{
            tempFindPos(_markAgentSize);
        }
    }
    ////////////////// BlockShoot //////////////////////////////////////////////
    else{
        tempFindPos(_markAgentSize);
    }
}

Vector2D DefensePlan::posvel(CRobot* opp){
    if(opp->vel.length() > 0.5 && opp->vel.x < 0)
        return opp->pos + 0.5 * opp->vel;
    else
        return opp->pos;
}

void DefensePlan::findOppAgentsToMark(QList <Vector2D> _realDefTargets)
{

    obspos.clear();
    opppos.clear();
    for(int i=0;i<wm->opp.activeAgentsCount();i++)
    {

        obspos.append(wm->opp.active(i)->pos);
        opppos.append(wm->opp.active(i)->pos);

    }

    for(int j=0;j<wm->our.activeAgentsCount();j++)
    {
        obspos.append(wm->our.active(j)->pos);

    }
    //allrobot=obspos;
    oppAgentsToMark.clear();
    oppAgentsMarkedByDef.clear();
    oppAgentsToMark.append(knowledge->toBeMopps);
    oppAgentsToMarkPos.clear();
    Segment2D tempMarkSeg;
    Vector2D sol1,sol2;
    // oppAgentsToMark.first()->pos = Vector2D(1,1);
    if(!knowledge->transientFlag)
    {
        for(int i = 0 ; i < oppAgentsToMark.count() ;i ++ )
        {
            tempMarkSeg.assign(posvel(oppAgentsToMark[i]),wm->field->ourGoal());
            for(int j = 0 ; j <_realDefTargets.size() ; j++)
            {
                if(Circle2D(_realDefTargets[j], 0.05).intersection(tempMarkSeg,&sol1,&sol2))
                {
                    oppAgentsMarkedByDef.append(oppAgentsToMark[i]);
                    break;
                }
            }
        }
    }

    for(int i =0 ; i < oppAgentsMarkedByDef.count() ; i++)
    {
        oppAgentsToMark.removeOne(oppAgentsMarkedByDef[i]);
    }

    for(int i = 0; i < oppAgentsToMark.count(); i++)
    {
        if(posvel(oppAgentsToMark[i]).x > policy()->Mark_OppOmitLimitPlayoff()){
            oppAgentsToMark.removeOne(oppAgentsToMark[i]);
            i--;
        }
    }



    for(int i = 0; i<oppAgentsToMark.count(); i++)
    {
        draw(oppAgentsToMark[i]->pos);
        oppAgentsToMarkPos.append(posvel(oppAgentsToMark[i]));

    }


    // qSort(oppAgentsToMarkPos.begin(),oppAgentsToMarkPos.end(),CMarkPlan::sortBy);

}


QList<CRobot*> DefensePlan::sortdanger(const QList<CRobot*> oppagent)
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

QList<QPair<Vector2D, double> > DefensePlan::sortdangerpassplayoff(QList<Vector2D> oppposdanger) {
    double danger;
    /////////////// Polygon
    double radius = .1;
    double treshold = .2;

    Vector2D sol1,sol2,sol3;
    Vector2D _pos1 = wm->ball->pos;

    Vector2D _pos2 = wm->ball->pos + (10.0  * wm->ball->vel.norm() * knowledge->getRealBallVel());
    Line2D _path(_pos1,_pos2);
    Polygon2D _poly;
    Circle2D(_pos2,radius + treshold).
            intersection(_path.perpendicular(_pos2),&sol1,&sol2);


    _poly.addVertex(sol1);
    sol3 = sol1;
    _poly.addVertex(sol2);
    Circle2D(_pos1,CRobot::robot_radius_new + treshold).
            intersection(_path.perpendicular(_pos1),&sol1,&sol2);

    _poly.addVertex(sol2);
    _poly.addVertex(sol1);
    _poly.addVertex(sol3);

    draw(_poly,"cyan");



    double KAP = 1; //Angle parameter
    double KDBP = 1; //distancetoball
    double KDIP = 2; //distancetointersect

    double AngleP;
    double distanceToBallProjectionP;
    double distanceToIntersectP;


    double RangeofAngleP = 90;
    double RangeofdistanceToBallProjectionP = Segment2D(Vector2D(-1.0 * _FIELD_WIDTH / 2, -1.0 * _FIELD_HEIGHT /2 ), Vector2D(_FIELD_WIDTH / 2 , _FIELD_HEIGHT / 2)).length();
    double RangeofdistanceToIntersectP =  radius;
    double danger2;


    /////////////////////


    double KA=1; //Angle Coefficient
    double KDB=1;  //Distance To Ball
    double KDG=1;  //Distnce To Goal
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(),Vector2D(-1.0 * (_FIELD_WIDTH / 2 - _GOAL_RAD), 0), wm->field->ourGoalL()).degree();
    //draw(Vector2D(-1.0 * (_FIELD_WIDTH - _GOAL_WIDTH), 0), QColor(Qt::red));
    // double RangeofAngle2 = Vector2D::angleOf(wm->field->ou,Vector2D(0, -1.0 * (_FIELD_WIDTH - _GOAL_WIDTH)), wm->field->ourGoalL()).degree();

    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), Vector2D(-1.0 * _FIELD_WIDTH/2,-1.0 * _FIELD_HEIGHT /2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), wm->field->ourGoal()).length());

    //double RangeofTempDis = 2;
    double angle, distancetoball, distancetogoal,danger1;


    QPair<Vector2D, double> temp;
    QList<QPair<Vector2D, double> > output;
    double Polycontain;
    for(int i = 0; i<oppposdanger.count(); i++) {
        if(Polycontain == _poly.contains(oppposdanger[i]))
        {
            Polycontain = 1;
        }
        else
        {
            Polycontain = 0;
        }
        temp.first = oppposdanger[i];


        angle = Vector2D::angleOf(wm->field->ourGoalR(), oppposdanger[i], wm->field->ourGoalL() ).degree();
        distancetoball =  (oppposdanger[i] - wm->ball->pos).length();
        distancetogoal =  (oppposdanger[i] - wm->field->ourGoal()).length();

        ////poly

        AngleP = Vector2D::angleOf( oppposdanger[i], wm->ball->pos, _path.projection(oppposdanger[i]) ).degree();
        distanceToIntersectP = _path.dist(oppposdanger[i]); //distanse of opponent to the path
        distanceToBallProjectionP = _path.projection(oppposdanger[i]).length(); //distance of the ball to the projection of opponent to the path

        danger1 = (KA * fabs(angle) / RangeofAngle) + ( KDB *( 1 - (distancetoball / RangeofDistancetoBall)) ) + (KDG * (1 -(distancetogoal / RangeofDistancetoGoal)));
        danger2 = KAP * ( 1 - AngleP/RangeofAngleP) + KDBP * (1 - distanceToBallProjectionP/RangeofdistanceToBallProjectionP ) + KDIP * (1 - distanceToIntersectP / RangeofdistanceToIntersectP);
        if(wm->ball->vel.length() < .2)
            danger = danger1;
        else
            danger = 3 * Polycontain * danger2 + danger1;

        temp.second = danger;
        output.append(temp);
        //draw(QString("HMD danger=%1").arg(danger), oppposdanger[i] + Vector2D(0,0.3), QColor(Qt::red));




        //        draw(QString("mindistance%1").arg(mintempdis), oppposdanger[i] + Vector2D(0,0.5), QColor(Qt::blue));




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

QList<QPair<Vector2D, double> > DefensePlan::sortdangerpassplayon(QList<Vector2D> oppposdanger) {





    double KA=1; //Angle Coefficient
    double KDB=1;  //Distance To Ball
    double KDG=1;  //Distnce To Goal
    double RangeofAngle = Vector2D::angleOf(wm->field->ourGoalR(),Vector2D(-1.0 * (_FIELD_WIDTH / 2 - _GOAL_RAD), 0), wm->field->ourGoalL()).degree();
    //draw(Vector2D(-1.0 * (_FIELD_WIDTH - _GOAL_WIDTH), 0), QColor(Qt::red));
    // double RangeofAngle2 = Vector2D::angleOf(wm->field->ou,Vector2D(0, -1.0 * (_FIELD_WIDTH - _GOAL_WIDTH)), wm->field->ourGoalL()).degree();

    double RangeofDistancetoBall = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), Vector2D(-1.0 * _FIELD_WIDTH/2,-1.0 * _FIELD_HEIGHT /2)).length());

    double RangeofDistancetoGoal = fabs(Segment2D(Vector2D(_FIELD_WIDTH/2,_FIELD_HEIGHT /2), wm->field->ourGoal()).length());

    double RangeofTempDis = 2;
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
        //draw(QString("HMD danger=%1").arg(danger), oppposdanger[i] + Vector2D(0,0.3), QColor(Qt::red));


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


        //        draw(QString("mindistance%1").arg(mintempdis), oppposdanger[i] + Vector2D(0,0.5), QColor(Qt::blue));




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

    for(int i=0; i<output.count(); i++)
    {
        //draw(QString("HMD Danger New%1" ).arg(output[i].second),output[i].first + Vector2D(0,.2),QColor(Qt::red));
    }

    return output;
}


QList<Vector2D> DefensePlan::ShootBlockRatio(double ratio, Vector2D opp){
    QList<Vector2D> tempQlist;
    CDefPos test;
    tempQlist.clear();
    Segment2D tempSeg;
    tempSeg.assign(opp + (wm->field->ourGoal() - opp) * (-10), wm->field->ourGoal());
    Vector2D pos = opp + (wm->field->ourGoal() - opp) * ratio;
    if((wm->field->ourGoal() - pos).length() < markRadiusStrict)
    {
        tempQlist.append(test.getIntersectionWithPenaltyAreaDef(1.37,tempSeg));
        tempQlist.append(opp - wm->field->ourGoal());
        draw(tempSeg, "blue");
    }
    else
    {
        tempQlist.append(pos);
        tempQlist.append(opp - wm->field->ourGoal());
        draw(tempSeg, "blue");
    }

    return tempQlist;
}

QList<Vector2D> DefensePlan::PassBlockRatio(double ratio, Vector2D opp){
    Segment2D tempSeg;
    QList<Vector2D> tempQlist;
    tempQlist.clear();
    tempSeg.assign(wm->ball->pos, wm->ball->pos + (opp - wm->ball->pos) * 10);
    Vector2D pos = wm->ball->pos + (opp - wm->ball->pos) * ratio;
    CDefPos test;
    if((wm->field->ourGoal() - pos).length() < markRadiusStrict)
    {
        tempQlist.append(test.getIntersectionWithPenaltyAreaDef(2, tempSeg));

        tempQlist.append( wm->ball->pos - opp);
        draw(tempSeg, "red");
        debug(QString("this is in the penalty area, Block pass Mode"), D_HAMED);
    }
    else
    {
    tempQlist.append(pos);
    tempQlist.append( wm->ball->pos - opp);
    draw(tempSeg, "red");
    }
    return tempQlist;
}
bool DefensePlan::lookat(){
    if(knowledge->transientFlag && (policy()->Mark_ManToManAllTransiant() || policy()->Mark_ManToManSomeTransiant()))
        return 1;
    else
        return 0;
}
////////
double CDefPos::findBestRadius(int _numOfDefs)
{

    Vector2D BallPos = wm->ball->pos;
    Vector2D ourGoalL = wm->field->ourGoalL();
    Vector2D ourGoalR = wm->field->ourGoalR();
    Segment2D leftBarSeg(BallPos , ourGoalL);
    Segment2D rightBarSeg(BallPos , ourGoalR);
    Segment2D goalSeg(ourGoalL,ourGoalR);
    Segment2D downFieldLine(Vector2D(-_FIELD_WIDTH/2,-_FIELD_HEIGHT/2),Vector2D(-_FIELD_WIDTH/2,_FIELD_HEIGHT/2));
    double ballheight = BallPos.dist(downFieldLine.nearestPoint(BallPos));
    double vatarKazeb;
    Segment2D biggerBarSeg = (leftBarSeg.length() < rightBarSeg.length()) ? rightBarSeg : leftBarSeg;
    Segment2D shorterBarSeg = (leftBarSeg.length() > rightBarSeg.length()) ? rightBarSeg : leftBarSeg;
    vatarKazeb = goalSeg.intersection(shorterBarSeg).dist(biggerBarSeg.nearestPoint(goalSeg.intersection(shorterBarSeg)));
    Line2D mainLine(goalSeg.intersection(shorterBarSeg),biggerBarSeg.nearestPoint(goalSeg.intersection(shorterBarSeg)));
    if( vatarKazeb > (_numOfDefs) * 2 * CRobot::robot_radius_new )
        mainLine = Line2D(Vector2D(BallPos.x-((_numOfDefs + 0.3) * 1.5 * CRobot::robot_radius_new*ballheight/vatarKazeb),BallPos.y),Vector2D(BallPos.x-((_numOfDefs + 0.3) * 1.5 * CRobot::robot_radius_new*ballheight/vatarKazeb),BallPos.y-0.1));
    if(biggerBarSeg.intersection(mainLine).valid()) {
        return biggerBarSeg.intersection(mainLine).dist(wm->field->ourGoal()) + penaltyAreaOffset;
    }
    else {
        return -1;
    }
}



Vector2D DefensePlan::ballPrediction(bool _isGoalie)
{
    Vector2D BallPos = wm->ball->pos;
    Vector2D BallVel = wm->ball->vel * 0.5;
    Segment2D ballPosVel(BallPos,BallPos+(BallVel * 0.5));
    Vector2D predictedBall;
    Vector2D solu[2];
    Rect2D fieldRect(Vector2D(- _FIELD_WIDTH/2.0 , - _FIELD_HEIGHT/2.0)+Vector2D(-0.005,-0.005),Vector2D(_FIELD_WIDTH/2.0 , _FIELD_HEIGHT/2.0)+Vector2D(+0.005,+0.005));
    double dist2Ball = 1000;
    if(BallVel.x > 0 && BallPos.x > 0) {// in ejdeha code
        return BallPos;
    }
    if(wm->opp.activeAgentsCount() > 0) {
        for(int i=0 ; i < wm->opp.activeAgentsCount() ; i++){
            Circle2D oppCircle(wm->opp.active(i)->pos,0.1);
            if(oppCircle.intersection(ballPosVel,&solu[0],&solu[1]) > 0 && BallVel.length() > 0.5){ //in ejdeha code
                if(wm->opp.active(i)->pos.dist(BallPos) < dist2Ball){
                    dist2Ball = wm->opp.active(i)->pos.dist(BallPos);
                    predictedBall = wm->opp.active(i)->pos;
                }
            }
        }
    }
    if(dist2Ball != 1000) {
        draw(QString("Def Predicted Level 1"), Vector2D(0,2),"red");
        draw(predictedBall);
        return predictedBall;
    }
    else if(wm->field->isInField(BallPos + BallVel) && BallVel.length() > 0.5) {
        if((BallVel.x <= 0.2 || _isGoalie)) {
            predictedBall = BallPos + BallVel;
        }
        else{
            draw(QString("Def Predicted"), Vector2D(0,1),"red");

            predictedBall = BallPos + Vector2D(0,BallVel.y);
        }
        draw(predictedBall);
        return predictedBall;

    }
    else if(BallVel.length() > 0.5){// in ejdeha code
        fieldRect.intersection(ballPosVel, &solu[0], &solu[1]);
        if(BallVel.x <= 0 || _isGoalie) {
            predictedBall = (BallPos.dist(solu[0] ) < BallPos.dist(solu[1]) && solu[0].isValid()) ? (solu[0]):(solu[1]);//// in ejdeha code
        }
        else {
            draw(QString("Def Predicted Level 2"), Vector2D(0,2),"red");

            predictedBall = (BallPos.dist(solu[0]) < BallPos.dist(solu[1])) ? Vector2D(BallPos.x,solu[0].y):Vector2D(BallPos.x,solu[1].y);

        }

    }
    else {
        predictedBall = BallPos;
        draw(QString("Def follow"), Vector2D(0,2),"red");
    }
    draw(predictedBall);
    return predictedBall;
}
//////////changesfor robocup 2016////////////////
Vector2D DefensePlan::getIntersectionWithPenaltyAreaGk(Segment2D _seg)
{
    ////////////////////////////////////////////////
    Vector2D ins[2];
    Vector2D finter;
    Vector2D fOurGoal(- _FIELD_WIDTH/2.0 , 0.0);
    Circle2D c1(fOurGoal + Vector2D(0,-_GOAL_WIDTH/4),_GOAL_RAD - 0.07);
    Circle2D c2(fOurGoal + Vector2D(0,+_GOAL_WIDTH/4),_GOAL_RAD - 0.07);
    Rect2D r(fOurGoal + Vector2D(+0,-_GOAL_WIDTH/4),fOurGoal + Vector2D(+_GOAL_RAD - 0.07,+_GOAL_WIDTH/4));
    ////////////////////////////////////////////////
    r.intersection(_seg,&ins[0],&ins[1]);
    if(ins[0].valid() || ins[1].valid()) {
        finter = (ins[0].x > ins[1].x) ? ins[0] : ins[1];

        if(finter.x > -3.6) {
            draw(QString("rect"),Vector2D(-1,0),"red");
            draw(finter);
            return finter;
        }
    }
    c1.intersection(_seg,&ins[0],&ins[1]);
    if(((wm->field->isInField(ins[0])) && (ins[0].y <= -_GOAL_WIDTH/4)) || ((wm->field->isInField(ins[1]) && ins[1].y <= -_GOAL_WIDTH/4))) {
        finter = (ins[0].x > ins[1].x) ? ins[0] : ins[1];
        draw(QString("c1"),Vector2D(-1,0),"red");
        draw(finter);
        return finter;
    }
    c2.intersection(_seg,&ins[0],&ins[1]);
    if((wm->field->isInField(ins[0]) && ins[0].y >= +_GOAL_WIDTH/4) || (wm->field->isInField(ins[1]) && ins[1].y >= _GOAL_WIDTH/4)) {
        finter = (ins[0].x > ins[1].x) ? ins[0] : ins[1];
        draw(QString("c2"),Vector2D(-1,0),"red");
        draw(finter);
        return finter;
    }
}

////////////////////////////////////////////////
