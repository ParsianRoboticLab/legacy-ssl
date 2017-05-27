#include "defenseplan.h"
#include <cmath>
#include <sstream>

using namespace std;

#define LOG(key, value) debug(QString("%1:: %2").arg(key).arg(value), D_MOHAMMED);

#define CHIP_POWER 1023
#define LONG_CHIP_POWER 1023
#define KICK_POWER 1000
///////////////// AHZ is writing, have you my voice? ... ;) //////////////////

float getDegree(Vector2D pos1, Vector2D origin, Vector2D pos3){
    //// get the angle of between two vector that
    //// is made up by this 3 points.

    Vector2D v1 = pos1 - origin;
    Vector2D v2 = pos3 - origin;
    return (v1.th() - v2.th()).degree();
}
bool DefensePlan::isIndirectArea(Vector2D aPoint){
    //// check that a point is in the circle around the ball
    //// with 50cm radius or not.

    bool localFlag = Circle2D(wm->ball->pos , ballCircleR).contains(aPoint) ?   1 : 0;
    return localFlag;
}
Line2D DefensePlan::getBisectorLine(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint){
    //// get the bisector line of an angle
    //// that is made up by this 3 points.

    Line2D bisectorLine (originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    return bisectorLine;
}
Segment2D DefensePlan::getBisectorSegment(Vector2D firstPoint , Vector2D originPoint , Vector2D thirdPoint){
    //// get the bisector segment of an angle
    //// that is made up by this 3 points.

    Line2D bisectorLine (originPoint , AngleDeg::bisect((firstPoint - originPoint).th() , (thirdPoint - originPoint).th()));
    Segment2D bisectorSegment(originPoint , Segment2D (thirdPoint , firstPoint).intersection(bisectorLine));
    return bisectorSegment;
}
void DefensePlan::manToManMarkInPlayOffBlockPass(QList<Vector2D> opponentAgentsToBeMarkPossition , int ourMarkAgentsSize , double proportionOfDistance){
    //// This function blocking the lines that are between ball &&
    //// opponent agents by a variable ratio along these lines.
    //// This is one of the mark plan for defending more flexible


    ////////////////////////// Variables of this function //////////////////////
    bool playOn = knowledge->getGameMode() == CKnowledge::Start;
    bool playOff = ((knowledge->getGameState() == CKnowledge::TheirDirectKick)/*|| (knowledge->getGameState() == CKnowledge::TheirKickOff)*/|| (knowledge->getGameState() == CKnowledge::TheirIndirectKick));
    int i;
    int j;
    Vector2D ourCenterOfGoalPossition = wm->field->ourGoal();
    Vector2D sol1 , sol2;
    Vector2D sol3 , sol4;
    Vector2D sol5 , sol6;
    Vector2D sol7 , sol8;
    QList <Vector2D> sol;
    double opponentAgentsCircleR = 0.2;
    QList<int> omittedOpp;
    QList<Circle2D> opponentAgentsCircle;
    QList<Circle2D> opponentAgentsToBeMarkCircle;
    QList<Circle2D> tempOpponentAgentsToBeMarkedCircle;
    QList<Vector2D> ourMarkAgentsPossition;
    QList<QPair<Vector2D,double> > sortDangerAgentsToBeMarkBlockPassPlayOff;
    QList<QPair<Vector2D,double> > tempSortDangerAgentsToBeBlockPassPlayOff;
    Circle2D goalCircle(ourCenterOfGoalPossition , 1.43);
    Circle2D penaltyArea(wm->field->ourGoal(),1.28);
    //////////////////// Clear QLists for update the states ////////////////////
    stopMode = knowledge->isStop();
    ourMarkAgentsPossition.clear();
    markPoses.clear();
    markAngs.clear();
    markRoles.clear();
    /////////////////////////// Added by AHZ for Intelligent Mark //////////////
    if((playOn && !knowledge->transientFlag) || knowledge->isStop()){
        sol.clear();
        AHZCount = 0;
    }
    if(playOff){
        tenLastOpponentDirection.append(wm->opp[knowledge->nearestOppToBall]->dir);
        if(AHZCount > 10){
            tenLastOpponentDirection.removeFirst();
        }
        AHZCount++;
    }
    if(LastTS != knowledge->transientFlag && LastTS == 0){
        if(AHZCount < 10){
            for(int i = tenLastOpponentDirection.size() - 1 ; i >= tenLastOpponentDirection.size() - (AHZCount - 1) ; i--){
                sumOfLastOpponentDirection += tenLastOpponentDirection.at(i);
            }
        }
        else{
            for(int i = tenLastOpponentDirection.size() - 1 ; i >= tenLastOpponentDirection.size() - 9 ; i--){
                sumOfLastOpponentDirection += tenLastOpponentDirection.at(i);
            }
        }
        if(AHZCount < 10){
            opponentPasserDirection = sumOfLastOpponentDirection / AHZCount;
        }
        else{
            opponentPasserDirection = sumOfLastOpponentDirection / 10;
        }
        opponentPasserPossition = wm->opp[knowledge->nearestOppToBall]->pos;
        tenLastOpponentDirection.clear();
        sumOfLastOpponentDirection = Vector2D(0,0);
    }
    if(policy()->Mark_IntelligentMarkPrediction()){
        if(knowledge->transientFlag){
            sol.append(wm->field->AHZOurPAreaIntersect(Segment2D(opponentPasserPossition , opponentPasserPossition + (10 * opponentPasserDirection))));
            if(sol.size()){
                sol.append(wm->field->AHZOurPAreaIntersect(Segment2D(opponentPasserPossition , opponentPasserPossition + (10 * opponentPasserDirection))));
                draw(Segment2D(opponentPasserPossition , opponentPasserPossition + (10 * opponentPasserDirection)) , "black");
                markPoses.append(Segment2D(sol.first() , opponentPasserPossition).length() > Segment2D(sol.last() , opponentPasserPossition).length() ? sol.first() : sol.last());
                markRoles.append(QString("predictBlocker"));
                markAngs.append(wm->field->center() - wm->field->ourGoal());
                ourMarkAgentsSize--;
            }
        }
    }
    LastTS = knowledge->transientFlag;
    ////////////////////////////////////////////////////////////////////////////
    debug(QString("Mark Agents Count : %1").arg(ourMarkAgentsSize) , D_SEPEHR , QColor(Qt::red));
    debug(QString("Opponent Agents to be mark count : %1").arg(opponentAgentsToBeMarkPossition.size()) , D_SEPEHR , QColor(Qt::red));
    ///// Ommit opponent Agent(s) that is(are) ball owner //////////////////////
    for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
        if(isIndirectArea(opponentAgentsToBeMarkPossition.at(i))){
            omittedOpp.append(i);
        }
    }
    for(i = 0 ; i < omittedOpp.size() ; i++){
        opponentAgentsToBeMarkPossition.removeAt(omittedOpp.at(i));
    }
    ///////// Make Cirlcles around opponent agents /////////////////////////////
    for(i = 0 ; i < opponentAgentsToBeMarkPossition.size(); i++){
        opponentAgentsToBeMarkCircle.append(Circle2D(opponentAgentsToBeMarkPossition.at(i) , opponentAgentsCircleR));
        draw(opponentAgentsToBeMarkCircle.at(i) , "Cyan");
    }
    ///////////////// Block Pass Plan ////////////////////////////////////
    if(opponentAgentsToBeMarkPossition.size() == ourMarkAgentsSize){
        for(i = 0 ; i < ourMarkAgentsSize ; i++){
            markRoles.append(QString("passBlocker"));
            //////////// Don't Enter penalty area, mark agents!!! :) ///////////
            if(penaltyArea.intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)) , &sol7,  &sol8)){
                if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                    penaltyArea.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 ,Segment2D(sol5 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol5:sol6 ,proportionOfDistance));
                }
                else{
                    goalCircle.intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                    markPoses.append(Segment2D(sol1 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol2 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol1 : sol2);
                }
            }
            else{
                opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4, proportionOfDistance));
            }
            markAngs.append(wm->ball->pos - markPoses.at(i));
            draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                           Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , QColor(Qt::red));
            draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
            draw(goalCircle,QColor(Qt::black));
            debug(QString("Man To Man Mark In PlayOff Mode / BlockPass / our = opp") , D_SEPEHR);
        }
    }
    else if(opponentAgentsToBeMarkPossition.size() < ourMarkAgentsSize){
        if(opponentAgentsToBeMarkPossition.size() == 0){
            for(i = 0 ; i < ourMarkAgentsSize ; i++){
                if(i % 2){
                    markPoses.append(Vector2D(0 , i / 1.5));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
                else{
                    markPoses.append(Vector2D(0,-i  / 1.5));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
            }
        }
        else if(opponentAgentsToBeMarkPossition.size() == 1){
            for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                markRoles.append(QString("passBlocker"));
                //////////// Don't Enter penalty area, mark agents!!! :) ///////////
                if(penaltyArea.intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)) , &sol7,  &sol8)){
                    if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                        penaltyArea.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 ,Segment2D(sol5 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol5:sol6 ,proportionOfDistance));
                    }
                    else{
                        penaltyArea.intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(sol1 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol2 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol1 : sol2);
                    }
                }
                else{
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4, proportionOfDistance));
                }
                markAngs.append(wm->ball->pos - markPoses.at(i));
                draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4), QColor(Qt::red));
                draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
            }
            for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    penaltyArea.intersection(Segment2D(opponentAgentsToBeMarkPossition.at(i) , wm->field->ourGoal()), &sol3 , &sol4);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3:sol4 ,policy()->Mark_ShootRatioBlock() / 100));
                }
                else{
                    penaltyArea.intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                    markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                }
                markAngs.append(opponentAgentsToBeMarkPossition.at(i) - wm->field->ourGoal());
                markRoles.append(QString("shotBlocker"));
            }
            for(i = 0 ; i < ourMarkAgentsSize - markPoses.size() + 1 ; i++){
                if(i % 2){
                    markPoses.append(Vector2D(0 , i / 1.5));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
                else{
                    markPoses.append(Vector2D(0,-i  / 1.5));
                    markAngs.append(Vector2D(0,0));
                    markRoles.append(QString("shotBlocker"));
                }
            }
        }
        else{
            for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                markRoles.append(QString("passBlocker"));
                //////////// Don't Enter penalty area, mark agents!!! :) ///////////
                if(penaltyArea.intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)) , &sol7,  &sol8)){
                    if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                        penaltyArea.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4) , &sol5 , &sol6);
                        markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 ,Segment2D(sol5 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol6 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol5:sol6 ,proportionOfDistance));
                    }
                    else{
                        penaltyArea.intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(sol1 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol2 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol1 : sol2);
                    }
                }
                else{
                    opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , opponentAgentsToBeMarkPossition.at(i)), &sol3 , &sol4);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4, proportionOfDistance));
                }
                markAngs.append(wm->ball->pos - markPoses.at(i));
                draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3 : sol4), QColor(Qt::red));
                draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
                debug(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp < our") , D_SEPEHR);
            }
            ////////////// With Extra mark agents ch ghalati bokonim ? :) //////////
            if(ourMarkAgentsSize - markPoses.size() == opponentAgentsToBeMarkPossition.size()){
                for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                    draw(opponentAgentsToBeMarkCircle.at(i),QColor(Qt::cyan));
                }
                for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                    if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        penaltyArea.intersection(Segment2D(opponentAgentsToBeMarkPossition.at(i) , wm->field->ourGoal()), &sol3 , &sol4);
                        markPoses.append(getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3:sol4 ,policy()->Mark_ShootRatioBlock() / 100));
                    }
                    else{
                        penaltyArea.intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                    }
                    markAngs.append(opponentAgentsToBeMarkPossition.at(i) - wm->field->ourGoal());
                    markRoles.append(QString("shotBlocker"));
                }
            }
            else if(ourMarkAgentsSize - markPoses.size() < opponentAgentsToBeMarkPossition.size()){
                tempSortDangerAgentsToBeBlockPassPlayOff = sortdangerpassplayoff(opponentAgentsToBeMarkPossition);
                for(i = 0 ; i < tempSortDangerAgentsToBeBlockPassPlayOff.size() ; i++){
                    tempOpponentAgentsToBeMarkedCircle.append(Circle2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , opponentAgentsCircleR));
                    draw(tempOpponentAgentsToBeMarkedCircle.at(i),QColor(Qt::yellow));
                }
                for(i = 0 ; i < ourMarkAgentsSize - markPoses.size() ; i++){
                    if(!wm->field->isInOurPenaltyArea(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first)){
                        tempOpponentAgentsToBeMarkedCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first), &sol1 , &sol2);
                        penaltyArea.intersection(Segment2D(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first , wm->field->ourGoal()), &sol3 , &sol4);
                        markPoses.append(getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first).length() ? sol3:sol4 ,policy()->Mark_ShootRatioBlock() / 100));
                    }
                    else{
                        penaltyArea.intersection(Line2D(wm->field->ourGoal() , tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first) , &sol1 , &sol2);
                        markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                    }
                    markAngs.append(tempSortDangerAgentsToBeBlockPassPlayOff.at(i).first - wm->field->ourGoal());
                    markRoles.append(QString("shotBlocker"));
                }
            }
            else if(ourMarkAgentsSize - markPoses.size() > opponentAgentsToBeMarkPossition.size()){
                for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                    draw(opponentAgentsToBeMarkCircle.at(i),QColor(Qt::cyan));
                }
                for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                    if(!wm->field->isInOurPenaltyArea(opponentAgentsToBeMarkPossition.at(i))){
                        opponentAgentsToBeMarkCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)), &sol1 , &sol2);
                        penaltyArea.intersection(Segment2D(opponentAgentsToBeMarkPossition.at(i) , wm->field->ourGoal()), &sol3 , &sol4);
                        markPoses.append(getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsToBeMarkPossition.at(i)).length() < Segment2D(sol4 , opponentAgentsToBeMarkPossition.at(i)).length() ? sol3:sol4 ,policy()->Mark_ShootRatioBlock() / 100));
                    }
                    else{
                        penaltyArea.intersection(Line2D(wm->field->ourGoal() , opponentAgentsToBeMarkPossition.at(i)) , &sol1 , &sol2);
                        markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                    }
                    markAngs.append(opponentAgentsToBeMarkPossition.at(i) - wm->field->ourGoal());
                    markRoles.append(QString("shotBlocker"));
                }
                for(i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
                    opponentAgentsCircle.append(Circle2D(wm->opp.active(i)->pos , opponentAgentsCircleR));
                }
                for(i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
                    for(j = 0 ; j < opponentAgentsToBeMarkPossition.size() ; j++){
                        if(opponentAgentsToBeMarkPossition.at(j) == wm->opp.active(i)->pos){
                            opponentAgentsCircle.removeAt(i);
                        }
                    }
                }
                for(i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
                    if(Circle2D(wm->ball->pos , ballCircleR).contains(wm->opp.active(i)->pos)){
                        opponentAgentsCircle.removeAt(i);
                    }
                }
                if(opponentAgentsToBeMarkPossition.size() == 0){
                    for(i = 0 ; i < opponentAgentsToBeMarkPossition.size() ; i++){
                        if(!wm->field->isInOurPenaltyArea(opponentAgentsCircle.at(i).center())){
                            opponentAgentsCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()), &sol1 , &sol2);
                            penaltyArea.intersection(Segment2D(opponentAgentsCircle.at(i).center() , wm->field->ourGoal()), &sol3 , &sol4);
                            markPoses.append(getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsCircle.at(i).center()).length() < Segment2D(sol4 , opponentAgentsCircle.at(i).center()).length() ? sol3:sol4 ,policy()->Mark_ShootRatioBlock() / 100));
                        }
                        else{
                            penaltyArea.intersection(Line2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()) , &sol1 , &sol2);
                            markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                        }
                        markAngs.append(opponentAgentsCircle.at(i).center() - wm->field->ourGoal());
                        markRoles.append(QString("shotBlocker"));
                    }
                }
                else{
                    for(i = 0 ; i < ourMarkAgentsSize - markPoses.size()+1 ; i++){
                        if(!wm->field->isInOurPenaltyArea(opponentAgentsCircle.at(i).center())){
                            opponentAgentsCircle.at(i).intersection(Segment2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()), &sol1 , &sol2);
                            penaltyArea.intersection(Segment2D(opponentAgentsCircle.at(i).center() , wm->field->ourGoal()), &sol3 , &sol4);
                            markPoses.append(getPointInDirection(Segment2D(sol1 , wm->field->ourGoal()).length() < Segment2D(sol2 , wm->field->ourGoal()).length() ? sol1 : sol2 ,Segment2D(sol3 , opponentAgentsCircle.at(i).center()).length() < Segment2D(sol4 , opponentAgentsCircle.at(i).center()).length() ? sol3:sol4 ,policy()->Mark_ShootRatioBlock() / 100));
                        }
                        else{
                            penaltyArea.intersection(Line2D(wm->field->ourGoal() , opponentAgentsCircle.at(i).center()) , &sol1 , &sol2);
                            markPoses.append(Segment2D(wm->field->center() , sol1).length() < Segment2D(wm->field->center() , sol2).length() ? sol1 : sol2);
                        }
                        markAngs.append(opponentAgentsCircle.at(i).center() - wm->field->ourGoal());
                        markRoles.append(QString("shotBlocker"));
                    }
                }
            }
        }
    }
    else if(opponentAgentsToBeMarkPossition.size() > ourMarkAgentsSize){
        sortDangerAgentsToBeMarkBlockPassPlayOff = sortdangerpassplayoff(opponentAgentsToBeMarkPossition);
        for(i = 0 ; i < ourMarkAgentsSize; i++){
            //////////// Don't Enter penalty area, mark agents!!! :) ///////////
            if(penaltyArea.intersection(Segment2D(wm->ball->pos , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first) , &sol7,  &sol8)){
                if(!wm->field->isInOurPenaltyArea(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first)){
                    Circle2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , opponentAgentsCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol3 , &sol4);
                    penaltyArea.intersection(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol3 : sol4) , &sol5 , &sol6);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2 ,Segment2D(sol5 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol6 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol5:sol6 ,proportionOfDistance));
                }
                else{
                    Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                    penaltyArea.intersection(Segment2D(wm->ball->pos , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first) , &sol5 , &sol6);
                    markPoses.append(getPointInDirection(Segment2D(sol1 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol2 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol1 : sol2 ,Segment2D(sol5 , wm->ball->pos).length() < Segment2D(sol6 , wm->ball->pos).length() ? sol5 :sol6 ,proportionOfDistance));
                }
            }
            else{
                Circle2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , opponentAgentsCircleR).intersection(Segment2D(sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first , wm->ball->pos) , &sol1 , &sol2);
                Circle2D(wm->ball->pos , ballCircleR).intersection(Segment2D(wm->ball->pos , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first), &sol3 , &sol4);
                markPoses.append(getPointInDirection(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,Segment2D(sol3 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol3 : sol4, proportionOfDistance));
            }
            markAngs.append(wm->ball->pos - markPoses.at(i));
            markRoles.append(QString("passBlocker"));
            draw(Segment2D(Segment2D(sol1 , wm->ball->pos).length() < Segment2D(sol2 , wm->ball->pos).length() ? sol1 : sol2,
                           Segment2D(sol3 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() < Segment2D(sol4 , sortDangerAgentsToBeMarkBlockPassPlayOff.at(i).first).length() ? sol3 : sol4)
                 , QColor(Qt::red));
            draw(Circle2D(wm->ball->pos , ballCircleR),QColor(Qt::black));
            debug(QString("Man To Man Mark In PlayOn Mode / BlockPass / opp > our") , D_SEPEHR);
        }

    }
    //////////////// Draw Possition of Mark Agents //////////////////////////
    for(i = 0 ; i < markPoses.size() ; i++){
        draw(markPoses.at(i) ,1, "white");
        draw(markRoles.at(i) , markPoses.at(i) - Vector2D(0,0.4) , "white");
    }
}
Vector2D DefensePlan::getPointInDirection(Vector2D firstPoint , Vector2D secondPoint , double proportion){
    //// This function gets a point along the lines that
    //// is made up by 2 points.The position of this point varies
    //// in according to the "ratio" that is given to the function.

    firstPoint = firstPoint + (secondPoint - firstPoint).norm() * proportion * (Segment2D(secondPoint , firstPoint).length());
    return firstPoint;
}
void DefensePlan::manToManMarkInPlayOffBlockShot(int _markAgentSize){
    //// This function blocking the lines that are between center of our goal &&
    //// opponent agents by a variable ratio along these lines.
    //// This is one of the mark plan for defending more flexible

    bool playOn = knowledge->getGameMode() == CKnowledge::Start;
    bool playOff = ((knowledge->getGameState() == CKnowledge::TheirDirectKick)/*|| (knowledge->getGameState() == CKnowledge::TheirKickOff)*/|| (knowledge->getGameState() == CKnowledge::TheirIndirectKick));
    bool intelligentMarkType = policy()->Mark_IntelligentMarkType();
    int count;
    QList <Vector2D> sol;
    oppmarkedpos.clear();
    markPoses.clear();
    markRoles.clear();
    markAngs.clear();
    /////////////////////////// Added by AHZ for Intelligent Mark //////////////
    if((playOn && !knowledge->transientFlag) || knowledge->isStop()){
        sol.clear();
        AHZCount = 0;
    }
    if(playOff){
        tenLastOpponentDirection.append(wm->opp[knowledge->nearestOppToBall]->dir);
        if(AHZCount > 10){
            tenLastOpponentDirection.removeFirst();
        }
        AHZCount++;
    }
    if(LastTS != knowledge->transientFlag && LastTS == 0){
        if(AHZCount < 10){
            for(int i = tenLastOpponentDirection.size() - 1 ; i >= tenLastOpponentDirection.size() - (AHZCount - 1) ; i--){
                sumOfLastOpponentDirection += tenLastOpponentDirection.at(i);
            }
        }
        else{
            for(int i = tenLastOpponentDirection.size() - 1 ; i >= tenLastOpponentDirection.size() - 9 ; i--){
                sumOfLastOpponentDirection += tenLastOpponentDirection.at(i);
            }
        }
        if(AHZCount < 10){
            opponentPasserDirection = sumOfLastOpponentDirection / AHZCount;
        }
        else{
            opponentPasserDirection = sumOfLastOpponentDirection / 10;
        }
        opponentPasserPossition = wm->opp[knowledge->nearestOppToBall]->pos;
        tenLastOpponentDirection.clear();
        sumOfLastOpponentDirection = Vector2D(0,0);
    }
    if(policy()->Mark_IntelligentMarkPrediction()){
        if(knowledge->transientFlag){
            sol.append(wm->field->AHZOurPAreaIntersect(Segment2D(opponentPasserPossition , opponentPasserPossition + (10 * opponentPasserDirection))));
            if(sol.size()){
                sol.append(wm->field->AHZOurPAreaIntersect(Segment2D(opponentPasserPossition , opponentPasserPossition + (10 * opponentPasserDirection))));
                draw(Segment2D(opponentPasserPossition , opponentPasserPossition + (10 * opponentPasserDirection)) , "black");
                markPoses.append(Segment2D(sol.first() , opponentPasserPossition).length() > Segment2D(sol.last() , opponentPasserPossition).length() ? sol.first() : sol.last());
                markRoles.append(QString("predictBlocker"));
                markAngs.append(wm->field->center() - wm->field->ourGoal());
                _markAgentSize--;
            }
        }
    }
    if(intelligentMarkType){
        if(knowledge->transientFlag){
            sol.append(wm->field->AHZOurPAreaIntersect(Segment2D(opponentPasserPossition , opponentPasserPossition + (10 * opponentPasserDirection))));
            if(sol.size()){
                segmentpershoot = 0.9;
            }
            else{
                segmentpershoot = 0.05;
            }
        }

    }
    LastTS = knowledge->transientFlag;
    ////////////////////////////////////
    if(_markAgentSize == oppAgentsToMarkPos.count())
    {
        ///shoot block
        for(int i=0; i < oppAgentsToMarkPos.count(); i++)
        {
            markRoles.append(QString("shotBlocker"));
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
    else if(_markAgentSize > oppAgentsToMarkPos.count() )
    {
        // Blocking direct shoot
        for(int i =0;i < oppAgentsToMarkPos.count();i++)
        {
            markRoles.append(QString("shotBlocker"));
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
        QList<QPair<Vector2D, double> > tempsorted = sortdangerpassplayoff(oppAgentsToMarkPos);
        for(int i=0;i<min(_markAgentSize - oppAgentsToMarkPos.count(), oppAgentsToMarkPos.count());i++)
        {
            markRoles.append(QString("passBlocker"));
            if(!checkIndirectAreaPass(tempsorted[i].first))
            {
                markPoses.append(PassBlockRatio(segmentperpass, tempsorted[i].first).first());
                markAngs.append(PassBlockRatio(segmentperpass, tempsorted[i].first).last());
            }
            else
            {
                markPoses.append(indirectAvoidPass(tempsorted[i].first).first());
                markAngs.append(indirectAvoidPass(tempsorted[i].first).last());
            }
        }
        //extra robot we have
        if(_markAgentSize > markPoses.count())
        {
            count = 0;
            for(int i=markPoses.count() ; i<_markAgentSize; i++)
            {

                if(markPoses.count() < _markAgentSize)
                {
                    markRoles.append(QString("shotBlocker"));
                    markPoses.append(Vector2D(0,  count *  4.5/ 6  * pow(-1, count) ));
                    markAngs.append(Vector2D(1,0));
                }
                count++;
            }

        }


    }
    else if(_markAgentSize < oppAgentsToMarkPos.count())
    {
        if(playOff || knowledge->transientFlag)
        {
            QList<QPair<Vector2D, double> > tempsorted = sortdangerpassplayoff(oppAgentsToMarkPos);
            for(int i = 0; i<_markAgentSize; i++)
            {
                markRoles.append(QString("shotBlocker"));
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
                markRoles.append(QString("shotBlocker"));
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
    for(int i = 0 ; i < markPoses.size() ; i++){
        draw(markRoles.at(i) , markPoses.at(i) - Vector2D(0,0.4) , "white");
    }
    for(int i =0;i<markPoses.count(); i++)
    {
        if(i < _markAgentSize) {
            draw(Circle2D(markPoses[i], 0.1), QColor(Qt::white), true);
            debug(QString("%1 : x : %2, y : %3").arg(i).arg(markPoses[i].x).arg(markPoses[i].y),D_MAHI);
        }
    }
}
void DefensePlan::setGoalKeeperState()
{
    //// In this function,we determine the specific states that goalkeeper must
    //// have a logical behavior by good conditions.In other word we have some
    //// exceptions mode for goalkeeper.These modes are :
    //// 1- ball is behind the goalkeeper,
    //// 2- One touch mode(goalkeeper diving mode) ,
    //// 3- Goalkeeper is in penalty area prediction ,
    //// 4- Throw out the ball from penalty area (Clear mode) ,
    //// 5- Strict following the ball ,
    //// 6- Following the ball ,
    //// 7- Ball is out of the field.

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
            else if(playOn && ((knowledge->getRealBallVel() < 0.5) || goalieClearMode) && penaltyArea.contains(BallPos)) {
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
            else{
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
void DefensePlan::setGaolKeeperTargetPoint(){
    //// This function determine the target point that goalkeeper must go to it.
    //// For producing the target point, we certainly consider the states that
    //// is result from the "setGoalKeeperState" function.  :)

    ///////////////////////////// Variables of this function ////////////////////////////////
    Vector2D ballPos;
    Vector2D ballVel;
    Vector2D predictedBall;
    Vector2D goaliePos;
    Vector2D Solutions[2];
    Vector2D goalKeeperTargetOffSet = Vector2D(0.2 , 0.0);
    bool playOnMode = knowledge->getGameMode() == CKnowledge::Start;
    stopMode = knowledge->isStop();
    ////////////////////////// Added Danger Mode && edited by AHZ //////////////////////////
    dangerForGoalieClear = false;
    dangerForGoalieClearByOurAgents = false;
    dangerForGoalieClearByOppAgents = false;
    isCrowdedInFrontOfPenaltyAreaByOppAgents = false;
    isCrowdedInFrontOfPenaltyAreaByOurAgents = false;
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
        savedClearPos = findBestPointForChipTarget(savedClearDist,1);
        penaltyArea.intersection(Line2D(wm->ball->pos , wm->field->ourGoal()),&Solutions[0] , &Solutions[1]);
        Circle2D dangerCircle = Circle2D(Solutions[0].dist(wm->ball->pos) < Solutions[1].dist(wm->ball->pos) ? Solutions[0] : Solutions[1], 0.40);
        Circle2D dangerCircle1 = Circle2D(Solutions[0].dist(wm->ball->pos) < Solutions[1].dist(wm->ball->pos) ? Solutions[0] : Solutions[1], 0.13);
        draw(dangerCircle , "yellow");
        draw(dangerCircle1 , "yellow");
        for(int i = 0; i < wm->our.activeAgentsCount(); i++){
            if(wm->our.active(i)->id != knowledge->goalie->id()){
                draw(wm->our.active(i)->pos + wm->our.active(i)->vel , 0 , "blue");
            }
        }
        if(ballIsOutOfField){
            dangerForGoalieClear = false;
            draw(QString("Ball Is Out Of Field"), Vector2D(0,1),"red");
            goalieTarget = wm->field->ourGoal() + goalKeeperTargetOffSet;
            return;
        }
        if(stopMode){
            dangerForGoalieClear = false;
            debug(QString("stopMode"),D_SEPEHR);
            //////////////////////////// AHZ ////////////////////////
            ballPos = wm->ball->pos;
            goalieTarget = wm->field->ourGoal() + goalKeeperTargetOffSet;
            //////////////////////////////////////////////////////
            return;
        }
        else if(ballBehindGoalie){
            draw(QString("Ball Is Behind The Goalie"), Vector2D(0,1),"red");
            return;
            ////////////Handle this state in executeGoalie() Func//////
        }
        else if(goalieOneTouch){
            ////////////////////////////when nearest point to goalie position on ball line is
            ///////////////////////////far from ball position in comparison with horizental line beside goalie
            if(ballLine.nearestPoint(goaliePos).x < ballLine.intersection(robotPrGoalLine).x) {
                goalieTarget = ballLine.nearestPoint(goaliePos);
                goalieTarget = (oneTouchCoef+1)*goalieTarget - (oneTouchCoef)*goaliePos;
                draw(QString("OneTouch To Nearest Point"), Vector2D(0,1),"red");
            }
            else{
                goalieTarget = ballLine.intersection(robotPrGoalLine);
                goalieTarget = (oneTouchCoef+1)*goalieTarget - (oneTouchCoef)*goaliePos;
                draw(QString("OneTouch To Side Point"), Vector2D(0,1),"red");
            }
            return;
        }
        else if(goalieInPenaltyAreaPrediction && penaltyArea.contains(ballPos)){
            if(knowledge->getRealBallVel() > 2 && wm->opp.activeAgentsCount() > 0 && predictMostDangrousOppToBall() != 100){
                goalieTarget = followBall(wm->opp[predictMostDangrousOppToBall()]->pos);
                draw(QString("Pass ball through penalty area using predict"), Vector2D(0,1),"red");
            }
            else {
                penaltyArea.intersection(ballLine,&Solutions[0],&Solutions[1]);
                goalieTarget = (Solutions[0].valid() && wm->field->isInField(Solutions[0])) ? followBall(Solutions[0]) : followBall(Solutions[1]);
                draw(QString("Pass ball through penalty area"), Vector2D(0,1),"red");
            }
            return;
        }
        else if(goalieClearMode && (playOnMode)){
            ////////////// Danger Mode for inside of the penalty area///////////
            savedClearPos = findBestPointForChipTarget(savedClearDist,1);
            if(wm->our.activeAgentsCount() > 0 || wm->opp.activeAgentsCount() > 0){
                for(int i = 0; i < wm->our.activeAgentsCount() ; i++){
                    if(wm->our.active(i)->id != knowledge->goalie->id()){
                        if(dangerCircle.contains(wm->our.active(i)->pos)){
                            isCrowdedInFrontOfPenaltyAreaByOurAgents = true;
                        }
                    }
                }
                for(int i = 0 ; i < wm->opp.activeAgentsCount() ; i++){
                    if(dangerCircle.contains(wm->opp.active(i)->pos)){
                        isCrowdedInFrontOfPenaltyAreaByOppAgents = true;
                    }
                }
            }
            if(isCrowdedInFrontOfPenaltyAreaByOurAgents){
                debug("Crowded" , D_AHZ , "red");
                if(dangerCircle1.contains(wm->ball->pos)){
                    dangerForGoalieClearByOurAgents = true;
                }
            }
            if(isCrowdedInFrontOfPenaltyAreaByOppAgents){
                if(dangerCircle1.contains(wm->ball->pos)){
                    dangerForGoalieClearByOppAgents = true;
                }
            }
            if(dangerForGoalieClearByOurAgents || dangerForGoalieClearByOppAgents){
                dangerForGoalieClear = true;
            }
            if(dangerForGoalieClear){
                if(dangerForGoalieClearByOppAgents){
                    goalieTarget =  getPointInDirection(wm->ball->pos , wm->field->ourGoal() ,0.2);
                }
                else if(dangerForGoalieClearByOurAgents){
                    debug("danger" , D_AHZ , "green");
                    penaltyArea.intersection(Line2D(wm->ball->pos , wm->field->ourGoal()),&Solutions[0] , &Solutions[1]);
                    goalieTarget = Solutions[0].dist(wm->ball->pos) < Solutions[1].dist(wm->ball->pos) ? Solutions[0] : Solutions[1];
                }
            }
            /////////////// End of Danger Mode ///////////////////////////////
            else if(ourLeftPole.contains(ballPos) || ourRightPole.contains(ballPos)){
                besidePoleFlag = true;
            }
            else{
                draw(QString("Clear"), Vector2D(0,1),"red");
            }
            return;
        }
        else{
            ////////////// Danger Mode for out of the penalty area /////////////
            if(wm->our.activeAgentsCount() > 0 || wm->opp.activeAgentsCount() > 0){
                for(int i = 0 ; i < wm->opp.activeAgentsCount()  ; i++){
                    if(dangerCircle.contains(wm->opp.active(i)->pos)){
                        isCrowdedInFrontOfPenaltyAreaByOppAgents = true;
                    }
                }
                for(int i = 0; i < wm->our.activeAgentsCount() ; i++){
                    if(wm->our.active(i)->id != knowledge->goalie->id()){
                        if(dangerCircle.contains(wm->our.active(i)->pos)){
                            isCrowdedInFrontOfPenaltyAreaByOurAgents = true;
                        }
                    }
                }
            }
            if(isCrowdedInFrontOfPenaltyAreaByOurAgents){
                if(dangerCircle.contains(wm->ball->pos)){
                    dangerForGoalieClear = true;
                }
            }
            if(isCrowdedInFrontOfPenaltyAreaByOppAgents){
                if(dangerCircle.contains(wm->ball->pos)){
                    dangerForGoalieClearByOppAgents = true;
                }
            }
            if(dangerForGoalieClearByOurAgents || dangerForGoalieClearByOppAgents){
                dangerForGoalieClear = true;
            }
            if(dangerForGoalieClear){
                debug("danger" , D_AHZ , QColor(Qt::blue));
                goalieTarget = getPointInDirection(wm->ball->pos , wm->field->ourGoal() , 0.1);
                if(!isInThePenaltyArea(goalieTarget)){
                    penaltyArea.intersection(Line2D(wm->ball->pos , wm->field->ourGoal()),&Solutions[0] , &Solutions[1]);
                    goalieTarget = Solutions[0].dist(wm->ball->pos) < Solutions[1].dist(wm->ball->pos) ? Solutions[0] : Solutions[1];
                }
            }
            //////////////// End of Danger Mode ////////////////////////////////
            else{
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
}
void DefensePlan::assignSkill(CAgent *_agent , CSkill *_skill){
    //// For run any skill,for example: kick , we must initialize
    //// some main variables that are effective to run a skill.
    //// This function is called anywhere that we need to run a skill.

    _agent->skill = _skill;
    _agent->skillName = _skill->getName();
    _skill->setAgent(_agent);

}
void DefensePlan::initGoalKeeper(CAgent *_goalieAgent){
    //// This function determines the goalkeeper agent. Actually this function
    //// is used in Coach.cpp && in "decideDefense"function to determine the
    //// goalkeeper agent.

    goalieAgent = _goalieAgent;
    agents.clear();
    if(_goalieAgent){
        agents.append(_goalieAgent);
    }
}
void DefensePlan::initDefense(const QList <CAgent*> &_defenseAgents){
    //// Just like the "initGoalKeeper" function , for inializing the defense
    //// agent , we have a function like this. :)

    defenseAgents.clear();
    defenseAgents.append(_defenseAgents);
    agents.append(_defenseAgents);
}
DefensePlan::DefensePlan()
{
    //// Constructor function of DefensePlan class

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
    markRadius = 1.6;
    markRadiusStrict = 1.39;
    segmentpershoot = policy()->Mark_ShootRatioBlock() / 100.0;
    segmentperpass = (100  - policy()->Mark_PassRatioBlock()) / 100.0;
    LastTS = true;
    dir  = Vector2D(1,0);
    MantoManAllTransientFlag =  policy()->Mark_ManToManAllTransiant();
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
    lastClearID = -1;
    lastTouchTheGoalie = -1;
    lastStateOffPlay = -1;
    oneToucher = 0;
    firstDefenseKickLine = -1;
    secondDefenseKickLine = 0;
    goalieKickThreshold = 70;
    /////////// AHZ //////////////
    lastMarkRoles.append(markRoles);
    changeInMarkPlanFlag = false;
    sumOfLastOpponentDirection = Vector2D(0,0);
    sumOfLastOpponentPosition = Vector2D(0,0);
    lastOpponentAgentsToBeMarkSize = 0;
    ////////////////////////////////
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
void DefensePlan::preCalculate(){
    //// This function initializes some variables from GUI &&
    //// sets some properties of kick && chip skill.
    //// Actually this function is used in "execute()" function , before any
    //// work that will do.

    stopMode = knowledge->isStop();
    if(goalieAgent){
        knowledge->goalie = goalieAgent;
    }
    announceClearing(false);
    for(int i = 0; i < count() ; i++){
        agent(i)->idle = false;
    }
    isItPossibleToClear = true;
    if(policy()->Defense_NoClear()){
        isItPossibleToClear = false;
    }
    for(int i = 0; i < count(); i++)
    {
        float dTheta = (wm->ball->pos - agent(i)->pos()).th().degree();
        if(knowledge->isStart()
                && (wm->ball->pos.dist(agent(i)->pos()) < 1.0)
                && (fabs(dTheta) < goalieKickThreshold)){

            agent(i)->setKick(agent(i)->kickSpeedValue(7.2 , false));
            ////////// Edited by AHZ ////////////////////////
            agent(i)->setChip(LONG_CHIP_POWER);
        }
        else{
            agent(i)->setKick(0);
            /////////// Edited by AHZ //////////////////
            agent(i)->setChip(0);
        }
    }
    for(int i = 0 ; i < _MAX_NUM_PLAYERS ; i++)
    {
        if(stopMode){
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
void DefensePlan::matchingDefPos(int _defenseNum){
    //// This Function matches the points that is produced by other functions to
    //// our agents we have in defense plan. Then we run the "GotoPointAvoid"
    //// skill on the agents.

    QList <CAgent*> ourAgents;
    QList <Vector2D> matchPoints;
    QList <int> matchResult;
    stopMode = knowledge->isStop();
    ourAgents.clear();
    ourAgents.append(defenseAgents);
    if(defExeptions.active){
        ///////////// Edited By AHZ ////////////////////////////////////////
        if(defExeptions.exepAgentId != -1){
            ourAgents.removeOne(knowledge->getAgent(defExeptions.exepAgentId));
        }
    }
    matchPoints.clear();
    for(int i = 0 ; i < _defenseNum ; i++) {
        draw(tempDefPos.pos[i],0,QColor(Qt::blue));
        matchPoints.append(tempDefPos.pos[i]);

    }
    findOppAgentsToMark(matchPoints);
    findPos(decideNumOfMarks(defPosDecision.overDef));
    matchPoints.append(markPoses);
    draw(QString(" %1 %2").arg(matchPoints.count()).arg(_defenseNum),Vector2D(-2,2),"red");
    draw(QString("  %1").arg(ourAgents.count()),Vector2D(2,2),"red");

    knowledge->Matching(ourAgents,matchPoints,matchResult);

    if(matchPoints.count() == ourAgents.count()){
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

            if(knowledge->getGameState() == CKnowledge::TheirIndirectKick)
            {
                gpa[ourAgents[i]->id()]->setNoAvoid(false);
                gpa[ourAgents[i]->id()]->setSlowMode(false);
                gpa[ourAgents[i]->id()]->setAvoidPenaltyArea(false);
                gpa[ourAgents[i]->id()]->setAvoidBall(true);
                gpa[ourAgents[i]->id()]->setBallObstacleRadius(0.5);

            }
            ////////////////// Addes by AHZ for Defense Agents /////////////////
            else if(stopMode){
                gpa[ourAgents[i]->id()]->setSlowMode(true);
                gpa[ourAgents[i]->id()]->setADiveMode(false);
            }
            //////////// Go To Point Avoid for defense agents //////////////////
            if(i < _defenseNum){
                gpa[ourAgents[i]->id()]->init(matchPoints[matchResult[i]] , matchPoints[matchResult[i]] - wm->field->ourGoal());
            }
            ///////// Go To Point Avoid for mark agents ////////////////////
            else{
                ////////// Added By AHZ /////////////////////
                //// for 4far game ///////
                gpa[ourAgents[i]->id()]->init(matchPoints[matchResult[i]] , markAngs.at(i - _defenseNum));
            }
        }

    }
}
void DefensePlan::execute()
{
    ///// All of the goalKeeper && defense functions are linked in this function.
    ///// First of all, we determine the behavior of goalKeeper.
    /////(first in penalty mode then other mode)
    ///// Now,we identify the number of defense agents && then this number is
    ///// sent to "matchingDefPos()" function to matching between the produced
    ///// points && our agents in defense plan.

    debug(QString("defense agents size %1").arg(defenseAgents.size()), D_HAMED);
    ////////////initialize////////////////
    initVars();
    preCalculate();
    debug(QString("valid point: %1").arg(isValidPoint(wm->ball->pos)) , D_AHZ);
    ballPosHistory.prepend(Vector2D(wm->ball->pos.x, wm->ball->pos.y));
    if(ballPosHistory.count() > 7)
        ballPosHistory.removeLast();
    //////////////////////////////////////
    bool playOn = knowledge->getGameMode() == CKnowledge::Start;
    //   draw(QString("gID : %1").arg(knowledge->goalie->id()),Vector2D(0,0));
    if (knowledge->getGameState() == CKnowledge::TheirPenaltyKick) //Penalty Mode
    {
        if( goalieAgent != NULL ){
            draw(QString("Penalty") , Vector2D(1,2) , "white");
            penaltyMode();
        }
        else{
            draw(QString("No Goalie!") , Vector2D(1,2) , "white");
        }
        return;
    }
    else {
        if(knowledge->goalie != NULL){
            setGoalKeeperState();
            setGaolKeeperTargetPoint();
            executeGoalie();
        }
        if(defenseAgents.size() > 0){
            if(wm->our.activeAgentsCount() < 7){
                debug(QString("Active agents are %1").arg(wm->our.activeAgentsCount()),D_HAMED);
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
                else{
                    defenseCount = defenseAgents.size();
                }
                if(defenseCount > 0){
                    int realDefSize;
                    defPosDecision = defPos.getDefPositions(wm->ball->pos, defenseCount, 1.5, 2.5);
                    realDefSize = defenseCount - decideNumOfMarks(defPosDecision.overDef);
                    tempDefPos = defPos.getDefPositions(ballPrediction(false), realDefSize, 1.5, 2.5);
                    matchingDefPos(realDefSize);
                }
            }
            else{
                draw("Vision Problem", Vector2D(0,0),"red");
            }
        }
    }
    return;

}
void DefensePlan::penaltyMode(){
    //// By this function goalKeeper is able to move according to the direction
    //// of the opponent agents that will shot to our goal in pentalty mode.

    Vector2D ballPos = wm->ball->pos;
    const float goalLineExtra = 0.03;
    const double xDiff = 0.10;
    Line2D newLine(wm->field->ourGoalL() + Vector2D(+xDiff,+goalLineExtra),wm->field->ourGoalR()+Vector2D(+xDiff,-goalLineExtra));
    const double epsilon = 0.12;
    Vector2D target(-2.93, 0.0);
    Line2D ballRay(ballPos, ballPos + (wm->opp[knowledge->nearestOppToBall]->dir));
    Vector2D intersectionPoint = newLine.intersection(ballRay);
    if(intersectionPoint.valid()){
        target = intersectionPoint;
        draw(target , 0 , "black");
    }
    else{
        target.y = 0.0;
    }
    target.y = min(max(target.y, wm->field->ourGoalR().y + epsilon), wm->field->ourGoalL().y - epsilon + 0.03);
    Vector2D targetDir(10, 10);
    targetDir.setDir(AngleDeg(60));
    targetDir.setLength(1);
    assignSkill(goalieAgent , gpa[goalieAgent->id()]);
    gpa[goalieAgent->id()]->setSlowMode(false);
    gpa[goalieAgent->id()]->setADiveMode(true);
    gpa[goalieAgent->id()]->init(target , targetDir);
}

////////////////////// ??????????? ///////////
bool DefensePlan::isValidPoint(const Vector2D& point){
    float distThreshold = 0.10f;

    if(point.x < wm->field->fieldRect().left() - distThreshold)
        return false;
    if(point.x > wm->field->fieldRect().right() + distThreshold)
        return false;
    if(point.y < wm->field->fieldRect().bottom() - distThreshold)
        return false;
    if(point.y > wm->field->fieldRect().top() + distThreshold)
        return false;

    Rect2D rect(midGoal + Vector2D(0.0, defenseAreaLine.length() / 2.0), defenseAreaBottomCircle.radius(), defenseAreaLine.length());
    return  rect.contains(point) == false &&
            defenseAreaBottomCircle.contains(point) == false &&
            defenseAreaTopCircle.contains(point) == false;
}
void DefensePlan::announceClearing(bool state){
    if(state){
        knowledge->variables["clearing"] = "true";
    }
    else{
        knowledge->variables["clearing"] = "false";
    }
}
//////////////////////////////////////////////////////////////////////////////


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
void DefensePlan::setPointToKick()
{
    if(oneToucher < 0 || oneToucher >= defenseAgents.size()){
        oneToucher = 0;
    }
    Vector2D ballPos = wm->ball->pos;
    Line2D ballLine(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm());
    if(defenseAgents.size() > 0){
        Line2D perpBallLine(defenseAgents.at(oneToucher)->self()->getKickerPos() , defenseAgents.at(oneToucher)->self()->getKickerPos()+wm->ball->vel.norm());
        perpBallLine = perpBallLine.perpendicular(defenseAgents.at(oneToucher)->self()->getKickerPos());
        Vector2D intersect = perpBallLine.intersection(ballLine);
        if( intersect.valid() ){
            if(isValidPoint(intersect) ){
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
    if(fastestToBall.isFastestOurs){
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
bool DefensePlan::defenseClearOrNot(){
    Vector2D ballPos = wm->ball->pos;
    defenseClearIndex = -1;
    bool isGameStarted = knowledge->isStart();
    if(ballPos.dist(wm->field->ourGoal()) > 3.5) {
        distClearHysteresis = false;
    }
    else if (ballPos.dist(wm->field->ourGoal()) < 2.5) {
        distClearHysteresis = true;
    }
    bool ballVelOrDirection = true;
    if( wm->ball->vel.length() > 1.5 || isBallGoingToOppArea())
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
                if (defenseAgents[defenseClearIndex]->id() && false != wm->our.active(i)->id && wm->our.active(i)->id != knowledge->goalie->id() ){
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
    kickSkill->setKickSpeed(1023);
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
        kickSkill->setChip(true);

        //        savedClearPos = findBestPointForChipTarget(savedClearDist,0);
        //        kickSkill->setTarget(savedClearPos);
        //        kickSkill->getAgent()->chipDistanceValue(savedClearDist, false);

        //        draw(savedClearPos);
        //        kickSkill->setChip(true);
    }
    if ( (defenseAgents.at(defenseClearIndex)->pos().dist(wm->ball->pos) < 0.5) &&
         (fabs(Vector2D::angleBetween(Vector2D(1.0, 0.0),(wm->ball->pos - wm->field->ourGoal())).degree()) > 40) &&
         (wm->ball->pos.dist(wm->field->ourGoal()) > 1.4)
         ) //good conditions for swapping clearer agent with an idle attacker
    {
        knowledge->formationChanges["clearswap"] = defenseAgents.at(defenseClearIndex)->id();
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

    return _defenseCount;
}

bool DefensePlan::checkStillBeingInOneTouch(bool goalieFlag){
    if(goalieFlag) {

        if(knowledge->goalie != NULL)
        {
            Segment2D goalLine(wm->field->ourGoal()+Vector2D(0,1) , wm->field->ourGoal()+Vector2D(0,-1));
            Segment2D ballLine = Segment2D(wm->ball->pos , wm->ball->pos + wm->ball->vel.norm()*10);
            Vector2D goalLineIntersect = goalLine.intersection(ballLine);
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

void DefensePlan::fillDefencePositionsTo(Vector2D *poses)
{
    for (int i=0 ; i < defenseAgents.count() ; i++) {
        poses[i] = defensePoints[i];
    }
}

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
    Vector2D sol1 , sol2;
    Vector2D offsetGoalkeeperPosition = Vector2D(0.2 , 0.0);
    Segment2D goal2Ball;
    Vector2D target(wm->field->center());
    QList<Circle2D> defs;
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
        Segment2D goalLine(wm->field->ourGoal()+Vector2D(0,-0.8) , wm->field->ourGoal() + Vector2D(0,0.8));
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
        draw(AZBisecOpenSeg,"red");
        goal2Ball.assign(wm->field->ourGoal(),wm->ball->pos);
        dangerFlag = 0;
        /////changes for RoboCup 2016////////in one def, goalie dont move to nearest point///////
#ifdef THEIR_DIRECT_CENTER
        if(knowledge->getGameState() == CKnowledge::TheirIndirectKick) {
            target = AZBisecOpenSeg.intersection(goalLine) + Vector2D(0.1,0.0);
            return target;
        }
#endif
        if(knowledge->goalie->pos().dist(AZBisecOpenSeg.nearestPoint(knowledge->goalie->pos())) > 0.2 + thr && defenseAgents.size() > 1){
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
                ////////////// Added by AHZ ///////////////////////////////////
                if(Vector2D::angleOf(wm->ball->pos,wm->field->ourGoal(),wm->field->ourGoalL()).degree() < 10 + angleDegreeThrNotStop){
                    Circle2D(wm->field->ourGoal() , 1.28).intersection(Segment2D(wm->ball->pos , AZBisecOpenSeg.intersection(aimLessLine)) , &sol1 , &sol2);
                    target = getPointInDirection(AZBisecOpenSeg.intersection((aimLessLine)) , sol1.dist(wm->field->ourGoal()) < sol2.dist(wm->field->ourGoal()) ? sol1 : sol2, 0.1);
                    angleDegreeThrNotStop = 0;
                }
                else if(Vector2D::angleOf(wm->ball->pos,wm->field->ourGoal(),wm->field->ourGoalR()).degree() < 10 + angleDegreeThrNotStop){
                    Circle2D(wm->field->ourGoal() , 1.28).intersection(Segment2D(wm->ball->pos , AZBisecOpenSeg.intersection(aimLessLine)) , &sol1 , &sol2);
                    target = getPointInDirection(AZBisecOpenSeg.intersection((aimLessLine)) , sol1.dist(wm->field->ourGoal()) < sol2.dist(wm->field->ourGoal()) ? sol1 : sol2, 0.1);
                    angleDegreeThrNotStop = 0;
                }
                else{
                    angleDegreeThrNotStop = 3;
                    if(defenseCount == 2){
                        if(knowledge->getEmptyAngle(ballPos, wm->field->ourGoalL(), wm->field->ourGoalR(), defs, AZDangerPercent, AZBisecOpenAngle, AZBigestOpenAngle,true) > 6.0 + threshOld){
                            target = AZBisecOpenSeg.intersection((aimLessLine));
                            threshOld = 0.0;
                        }
                        else{
                            target = wm->field->ourGoal() + offsetGoalkeeperPosition;
                            threshOld = 2.0;
                        }
                    }
                    else if(defenseCount == 1){
                        Circle2D(wm->field->ourGoal() , 1.28).intersection(Segment2D(wm->ball->pos , AZBisecOpenSeg.intersection(aimLessLine)) , &sol1 , &sol2);
                        target = getPointInDirection(AZBisecOpenSeg.intersection((aimLessLine)) , sol1.dist(wm->field->ourGoal()) < sol2.dist(wm->field->ourGoal()) ? sol1 : sol2, 0.1);
                        draw(target , 1 , "green");
                    }
                }
                if(!isInThePenaltyArea(target)){
                    target = getIntersectionWithPenaltyAreaGk(AZBisecOpenSeg);
                }
                ////////////////////////////////////////////////////////////////
            }
            else{
                target = getIntersectionWithPenaltyAreaGk(AZBisecOpenSeg);
            }
        }
        if ((!wm->field->isInField(target) || target.x < -4.4) && defenseCount == 2){
            target = AZBisecOpenSeg.intersection(goalLine) + offsetGoalkeeperPosition;
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
    Vector2D target(wm->field->ourGoal());
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

void DefensePlan::checkDefenseExeptions(){
    if(defenseAgents.size() > 0){
        setPointToKick();
        doOneTouch = defenseOneTouchOrNot();
        doClear = defenseClearOrNot();
        bool forceBeingInOneTouch = checkStillBeingInOneTouch(false);
        if(forceBeingInOneTouch)
            oneTouchPointFlag = false;
        if(doOneTouch || forceBeingInOneTouch){
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

void DefensePlan::runDefenseExeptions(){
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

void DefensePlan::executeGoalie(){    
    stopMode = knowledge->isStop();
    if(goalieAgent != NULL){
        if(savedClearPos.valid() && clearCnt > 30) {
            savedClearPos.invalidate();
            clearCnt = 0;
        }
        else if(savedClearPos.valid() && clearCnt <= 30){
            clearCnt++;
        }
        else{
            clearCnt = 0;
        }
        // edited by AHZ
        // ommite the setChip(CHIP_POWER)
        Rect2D fieldRect(Vector2D(- _FIELD_WIDTH/2.0 , - _FIELD_HEIGHT/2.0)+Vector2D(-0.005,-0.005),Vector2D(_FIELD_WIDTH/2.0 , _FIELD_HEIGHT/2.0)+Vector2D(+0.005,+0.005));
        Line2D ballPrGoalLine(wm->ball->pos, Vector2D(wm->ball->pos.x,(wm->ball->pos.y + 0.01)));
        Vector2D solut[2];
        fieldRect.intersection(ballPrGoalLine, &solut[0], &solut[1]);
        assignSkill( goalieAgent , gpa[goalieAgent->id()]);
        if(stopMode){
            debug("Stop Mode" , D_AHZ, "green");
            gpa[goalieAgent->id()]->setADiveMode(false);
            gpa[goalieAgent->id()]->setSlowMode(true);
            gpa[goalieAgent->id()]->init(goalieTarget , wm->ball->pos - wm->field->ourGoal());
        }
        else if(ballIsOutOfField){
            debug("Ball is out of field" , D_AHZ, "green");
            gpa[goalieAgent->id()]->setADiveMode(false);
            gpa[goalieAgent->id()]->setSlowMode(true);
            goalieAgent->setChip(0);
            gpa[goalieAgent->id()]->init(goalieTarget , wm->ball->pos - wm->field->ourGoal());
        }
        else if(ballBehindGoalie){
            debug("ball behind goalie" , D_AHZ , "red");
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
        else if(besidePoleFlag){
            debug("beside ple flag" , D_AHZ , "red");
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
        else if(goalieClearMode && !dangerForGoalieClear){
            debug("Clear" , D_AHZ , "red");
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
            if(!isPathToOppGoalieClear()){
                if(!savedClearPos.valid())
                    savedClearPos = findBestPointForChipTarget(savedClearDist,1);
                kickSkill->setTarget(savedClearPos);
                kickSkill->getAgent()->chipDistanceValue(savedClearDist, false);
                kickSkill->setChip(true);
            }
        }
        else{
            assignSkill( goalieAgent , gpa[goalieAgent->id()]);
            if(goalieOneTouch){
                gpa[goalieAgent->id()]->setSlowMode(false);
                gpa[goalieAgent->id()]->setADiveMode(true);
                Vector2D targetDir(0,goalieAgent->pos().y);
                debug("one touch" , D_AHZ , QColor(Qt::blue));
                gpa[goalieAgent->id()]->init(goalieTarget, targetDir);
            }
            if(dangerForGoalieClear){
                // edited by AHZ
                gpa[goalieAgent->id()]->setSlowMode(false);
                gpa[goalieAgent->id()]->setADiveMode(false);
                goalieAgent->setChip(0);
                goalieAgent->setKick(0);
                chipGKCounter = 0;
                gpa[goalieAgent->id()]->init(goalieTarget,  wm->ball->pos - wm->field->ourGoal()); //edited by AHZ
            }
            else{
                gpa[goalieAgent->id()]->setSlowMode(false);
                gpa[goalieAgent->id()]->setADiveMode(false);
                debug("not danger" , D_AHZ , "green");
                goalieAgent->setChip(0);
                //goalieAgent->setChip(LONG_CHIP_POWER);
                //                if(chipGKCounter > 50) {
                //                    goalieAgent->setChip(CHIP_POWER);
                //                    chipGKCounter = 0;
                //                }
                //                else{
                //                    chipGKCounter++;
                //                }
                gpa[goalieAgent->id()]->init(goalieTarget, wm->ball->pos - wm->field->ourGoal());
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
            return defenseCount;//TO DO:
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

bool DefensePlan::checkIndirectAreaShoot(Vector2D opp){
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

QList<Vector2D> DefensePlan::indirectAvoidShoot(Vector2D opp){
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

QList<Vector2D> DefensePlan::indirectAvoidPass(Vector2D opp){
    Segment2D tempseg;
    tempseg.assign(wm->ball->pos, opp + 10 * (opp - wm->ball->pos));
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

void DefensePlan::inteliDecideMarkType(){
    Segment2D tempseg;
    CDefPos test;
    if(knowledge->transientFlag)
    {
        if(LastTS != knowledge->transientFlag){
            dir = wm->opp[knowledge->nearestOppToBall]->dir;
        }
        tempseg.assign(wm->ball->pos, wm->ball->pos + 10 * dir);
        draw(tempseg, QColor(Qt::green));
        if(test.getIntersectionWithPenaltyAreaDef(1.37,tempseg).isValid()){
            MantoManAllTransientFlag = false;
            segmentpershoot = 1;
        }
        else{
            MantoManAllTransientFlag = true;
            segmentpershoot = 0.2;
        }
    }
    else {
        segmentpershoot = policy()->Mark_ShootRatioBlock() / 100;
        segmentperpass = (100 - policy()->Mark_PassRatioBlock()) / 100;
    }
    LastTS = knowledge->transientFlag;
}

void DefensePlan ::findPos(int _markAgentSize){
    double xLimitForblockingPass = 0;
    bool playOn = knowledge->getGameMode() == CKnowledge::Start;
    bool playOff = ((knowledge->getGameState() == CKnowledge::TheirDirectKick)/*|| (knowledge->getGameState() == CKnowledge::TheirKickOff)*/|| (knowledge->getGameState() == CKnowledge::TheirIndirectKick));
    bool MantoManAllTransientFlag = policy()->Mark_ManToManAllTransiant();
    bool manToManMarkBlockPassFlag = policy()->Mark_PlayOffManToMan();
    stopMode = knowledge->isStop();
    ///////////////// Man To Man AllTransiant Mode for Mark ////////////////////
    if(MantoManAllTransientFlag)
    {
        if(knowledge->transientFlag){
            segmentpershoot = 0.1;
        }
        else
            segmentpershoot = policy()->Mark_ShootRatioBlock() / 100;
        segmentperpass = (100 - policy()->Mark_PassRatioBlock()) / 100;
    }
    else{
        segmentpershoot = policy()->Mark_ShootRatioBlock() / 100;
        segmentperpass = (100 - policy()->Mark_PassRatioBlock()) / 100;
    }
    //////////////// Determine the plan of mark from GUI ////////////////////
    if(manToManMarkBlockPassFlag){
        if(playOff){
            manToManMarkInPlayOffBlockPass(oppAgentsToMarkPos,_markAgentSize , policy()->Mark_PassRatioBlock() / 100);
        }
        /////////////// Added By AHZ for transient mode :) ////////////////////
        if(knowledge->transientFlag){
            manToManMarkInPlayOffBlockPass(oppAgentsToMarkPos,_markAgentSize , policy()->Mark_PassRatioBlock() / 100);
        }
        else if(playOn){
            changeInMarkPlanFlag = false;
            manToManMarkInPlayOffBlockShot(_markAgentSize);
        }
        else if(stopMode){
            changeInMarkPlanFlag = false;
            manToManMarkInPlayOffBlockShot(_markAgentSize);
        }
    }
    /////////////////////////////////////////////
    else if(wm->ball->pos.x > xLimitForblockingPass){
        limitBetweenAHZAndHMD = true;
        if(playOff){
            manToManMarkInPlayOffBlockPass(oppAgentsToMarkPos,_markAgentSize , policy()->Mark_PassRatioBlock() / 100);
        }
        /////////////// Added By AHZ for transient mode :) ////////////////////
        if(knowledge->transientFlag){
            if(limitBetweenHMDAndAHZ){
                manToManMarkInPlayOffBlockShot(_markAgentSize);
            }
            else{
                manToManMarkInPlayOffBlockPass(oppAgentsToMarkPos,_markAgentSize , policy()->Mark_PassRatioBlock() / 100);
            }
        }
        else if(playOn){
            limitBetweenHMDAndAHZ = false;
            manToManMarkInPlayOffBlockShot(_markAgentSize);
        }
        else if(stopMode){
            limitBetweenHMDAndAHZ = false;
            manToManMarkInPlayOffBlockShot(_markAgentSize);
        }
    }
    else {
        //changeInMarkPlanFlag = false;
        if(knowledge->transientFlag){
            if(limitBetweenAHZAndHMD){
                manToManMarkInPlayOffBlockPass(oppAgentsToMarkPos,_markAgentSize , policy()->Mark_PassRatioBlock() / 100);
            }
            else{
                limitBetweenAHZAndHMD = false;
                limitBetweenHMDAndAHZ = true;
                manToManMarkInPlayOffBlockShot(_markAgentSize);
            }
        }
        else{
            limitBetweenAHZAndHMD = false;
            limitBetweenHMDAndAHZ = true;
            manToManMarkInPlayOffBlockShot(_markAgentSize);
        }
    }
}

Vector2D DefensePlan::posvel(CRobot* opp, double VelReliabiity){
    CDefPos test;
    if(VelReliabiity == 0)
        return opp->pos;
    if(!opp->vel.length() > 0.5  || opp->vel.x > 0 || test.isInPenaltyAreaDef(1.37, opp->pos)){
        VelReliabiity = 0;
    }
    Segment2D tempseg;
    Vector2D temppos = opp->pos + VelReliabiity * opp->vel;
    tempseg.assign(opp->pos, opp->pos + VelReliabiity * opp->vel );
    draw(tempseg,QColor(Qt::yellow));
    Vector2D penaltyvec;
    penaltyvec.assign(test.getIntersectionWithPenaltyAreaDef(1.37,tempseg).x,test.getIntersectionWithPenaltyAreaDef(1.37,tempseg).y)  ;
    if(wm->field->isInField(penaltyvec) && penaltyvec.isValid() && tempseg.length() != 0){
        debug(QString("Intersection with penalty area by penaltyvec: %1,%2").arg(penaltyvec.x).arg(penaltyvec.y),D_HAMED);
        return penaltyvec;
    }
    else if((temppos).x < -4.4){
        debug(QString("Opp is out"),D_HAMED);
        return Vector2D(-4.4,(opp->pos + VelReliabiity * opp->vel).y) ;
    }
    else{
        debug(QString("normal mode"),D_HAMED);
        return opp->pos + VelReliabiity * opp->vel;
    }
}

void DefensePlan::findOppAgentsToMark(QList <Vector2D> _realDefTargets)
{   
    oppAgentsToMark.clear();
    oppAgentsMarkedByDef.clear();
    oppAgentsToMark.append(knowledge->toBeMopps);
    oppAgentsToMarkPos.clear();
    if(knowledge->getGameState() == CKnowledge::TheirKickOff)
    {
        for(int i = 0; i < oppAgentsToMark.count(); i++)
        {
            if(oppAgentsToMark[i]->pos.x > policy()->Mark_OppOmitLimitKickOff()){
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                // TODO: chage the transeint this flag
                /*if(oppAgentsToMark[i]->vel.length() > 1)
                    HMDtransient = 1;*/
                i--;
            }
        }
    }
    else{
        for(int i = 0; i < oppAgentsToMark.count(); i++)
        {
            if(oppAgentsToMark[i]->pos.x > policy()->Mark_OppOmitLimitPlayoff()){
                oppAgentsToMark.removeOne(oppAgentsToMark[i]);
                // TODO: chage the transeint this flag
                /*if(oppAgentsToMark[i]->vel.length() > 1)
                HMDtransient = 1;*/
                i--;
            }
        }
    }

    for(int i = 0; i<oppAgentsToMark.count(); i++)
    {
        draw(oppAgentsToMark[i]->pos);
        oppAgentsToMarkPos.append(posvel(oppAgentsToMark[i], 0.5));

    }

    knowledge->ToBeMark.clear();
    knowledge->ToBeMark.append(oppAgentsToMarkPos);
    debug(QString("OppAgenttoMark count %1").arg(oppAgentsToMarkPos.count()), D_HAMED);

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
    double treshold = 1;

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
    double KDB=0;  //Distance To Ball
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
        // debug(QString("angle: %1, rangeofangle: %2, distansetoball:%3, RangeofDistancetoBall:%4,distancetogoal:%5,rangeofdistansetogoal:%6").arg(angle).arg(RangeofAngle).arg(distancetoball).arg(RangeofDistancetoBall).arg(distancetogoal).arg(RangeofDistancetoGoal),D_HAMED);
        // debug(QString("angleP: %1, rangeofangleP: %2, distansetoballProjectionP:%3, RangeofDistancetoBallProjectionP:%4,distancetointersect:%5,rangeofdistansetointesrsect:%6").arg(AngleP).arg(RangeofAngleP).arg(distanceToBallProjectionP).arg(RangeofdistanceToBallProjectionP).arg(distanceToIntersectP).arg(RangeofdistanceToIntersectP),D_HAMED);
        if( knowledge->getRealBallVel() < .1)
            danger = danger1;
        else
            danger = 10* Polycontain * danger2 + danger1;

        temp.second = danger;
        output.append(temp);
        draw(QString("HMD danger=%1").arg(danger), oppposdanger[i] + Vector2D(0,0.3), QColor(Qt::red));
        //draw(_poly, QColor(Qt::blue));



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
Vector2D DefensePlan::getIntersectionWithPenaltyAreaGk(Segment2D _seg)
{
    ///////////////////////////////////////////////
    Vector2D ins[2];
    Vector2D finter;
    Vector2D fOurGoal(- _FIELD_WIDTH/2.0 , 0.0);
    Circle2D c1(fOurGoal + Vector2D(0,-_GOAL_WIDTH/4),_GOAL_RAD - 0.07);
    Circle2D c2(fOurGoal + Vector2D(0,+_GOAL_WIDTH/4),_GOAL_RAD - 0.07);
    Rect2D r(fOurGoal + Vector2D(+0,-_GOAL_WIDTH/4),fOurGoal + Vector2D(+_GOAL_RAD - 0.07,+_GOAL_WIDTH/4));
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
