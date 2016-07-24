#include "learning/policylearner.h"
#include <QDebug>

#include "logger.h"
#include "VarTypes.h"
#if 0
CPolicyLearner::CPolicyLearner(CCoach* c)
{

    showDebug = false;
    firstTime = true;

    //Stat Ball (sb) Stat Our (so) Stat oPp (sp)
    wm->statAnal->addStatVariable("sb");
    wm->statAnal->addRegionToVariable("sb", "reg1", wm->field->getRegion("ourdeffenseonethird"));
    wm->statAnal->addRegionToVariable("sb", "reg2", wm->field->getRegion("ourmidonethird"));
    wm->statAnal->addRegionToVariable("sb", "reg3", wm->field->getRegion("ourattackonethird"));
    for (int i = 0; i < _MAX_NUM_PLAYERS; i++)
    {
        wm->statAnal->addStatVariable(QString("so%1").arg(i));
        wm->statAnal->addRegionToVariable(QString("so%1").arg(i), "reg1", wm->field->getRegion("ourdeffenseonethird"));
        wm->statAnal->addRegionToVariable(QString("so%1").arg(i), "reg2", wm->field->getRegion("ourmidonethird"));
        wm->statAnal->addRegionToVariable(QString("so%1").arg(i), "reg3", wm->field->getRegion("ourattackonethird"));

        wm->statAnal->addStatVariable(QString("sp%1").arg(i));
        wm->statAnal->addRegionToVariable(QString("sp%1").arg(i), "reg1", wm->field->getRegion("ourdeffenseonethird"));
        wm->statAnal->addRegionToVariable(QString("sp%1").arg(i), "reg2", wm->field->getRegion("ourmidonethird"));
        wm->statAnal->addRegionToVariable(QString("sp%1").arg(i), "reg3", wm->field->getRegion("ourattackonethird"));
    }

    wm->statAnal->addStatVariable("noOp");
    wm->statAnal->addStatVariable("noOpDefs");

    this->coach  = c;

    ourLastBallOwner = oppLastBallOwner = ourBallOwner = oppBallOwner = -1;
    ballState = lastBallState = _FREE;
    ballOutPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);


    /* Our Corner Left & Right*/
    fieldSegments[0] = Segment2D(wm->field->ourGoalL(), wm->field->ourCornerL());
    fieldSegments[10] = Segment2D(wm->field->ourCornerR(), wm->field->ourGoalR());
    ourRewards[0] = ourRewards[10] = -8.0;
    oppRewards[0] = oppRewards[10] = -4.0;
    ourStr[0] = ourStr[10] = "Corner for Opponent";
    oppStr[0] = oppStr[10] = "Goal Kick For Us";

    /* Our One Third Left & Right */
    fieldSegments[1] = Segment2D(wm->field->ourCornerL(), wm->field->ourOneThirdL());
    fieldSegments[9] = Segment2D(wm->field->ourOneThirdR(), wm->field->ourCornerR());
    ourRewards[1] = ourRewards[9] = -1.0;
    oppRewards[1] = oppRewards[9] = -2.0;
    ourStr[1] = ourStr[9] = "Throw in for Opponent";
    oppStr[1] = oppStr[9] = "Throw in for Us";

    /* Midfield Left & Right */
    fieldSegments[2] = Segment2D(wm->field->ourOneThirdL(), wm->field->oppOneThirdL());
    fieldSegments[8] = Segment2D(wm->field->oppOneThirdR(), wm->field->ourOneThirdR());
    ourRewards[2] = ourRewards[8] =  -1.0;//0.0;
    oppRewards[2] = oppRewards[8] =  1.0;
    ourStr[2] = ourStr[8] = "Throw in for Opponent";
    oppStr[2] = oppStr[8] = "Throw in for Us";

    /* Opp One Third Left & Right */
    fieldSegments[3] = Segment2D(wm->field->oppOneThirdL(), wm->field->oppCornerL());
    fieldSegments[7] = Segment2D(wm->field->oppCornerR(), wm->field->oppOneThirdR());
    ourRewards[3] = ourRewards[7] =  -1.0;//1.0;
    oppRewards[3] = oppRewards[7] =  2.0;
    ourStr[3] = ourStr[7] = "Throw in for Opponent";
    oppStr[3] = oppStr[7] = "Throw in for Us";

    /* Opp Corner Left& Right */
    fieldSegments[4] = Segment2D(wm->field->oppCornerL(), wm->field->oppGoalL());
    fieldSegments[6] = Segment2D(wm->field->oppGoalR(), wm->field->oppCornerR());
    ourRewards[4] = ourRewards[6] =  4.0;//4.0;
    oppRewards[4] = oppRewards[6] =  8.0;//8.0;
    ourStr[4] = ourStr[6] = "Goal Kick For Opponent";
    oppStr[4] = oppStr[6] = "Corner for Us";


    /* Opp Goal */
    fieldSegments[5] = Segment2D(wm->field->oppGoalL(), wm->field->oppGoalR()); /* */
    ourRewards[5] =  20.0;
    oppRewards[5] =  15.0;
    ourStr[5] = "Goal for Us";
    oppStr[5] = "Own Goal by Opponent";

    /* Our Goal */
    fieldSegments[11] = Segment2D(wm->field->ourGoalR(), wm->field->ourGoalL()); /* */
    ourRewards[11] =  -20.0;
    oppRewards[11] =  -15.0;
    ourStr[11] = "Own Goal by Us";
    oppStr[11] = "Goal for Opponent";

    changeOwnerUsReward = 3.0;
    changeOwnerOppReward = -3.0;

    isOwnerChanged = false;


    /* RL */

    learner = new MTileSarsaLearner();

    // Def : Our Defence

    //  Ball Histogram in Our Def
    //learner->addStateVariable("bhDef", 0.0, 1.0, 8);
    //  Ball Histogram in Mid
    //learner->addStateVariable("bhMid", 0.0, 1.0, 8);
    //  Our Robots Histogram in Our Def
    learner->addStateVariable("oDef", 0.0, 1.0, 5);
    //  Our Robots Histogram in Mid
    learner->addStateVariable("oMid", 0.0, 1.0, 5);
    //  Opp Robots Histogram in Our Def
    learner->addStateVariable("pDef", 0.0, 1.0, 5);
    //  Opp Robots Histogram in Mid
    learner->addStateVariable("pMid", 0.0, 1.0, 5);
    //  Current Pass/Shoot threshold
    //learner->addStateVariable("psTres", 0.0, 1.0, 8);
    //  Current OneTouch threshold
    //learner->addStateVariable("otTres", 0.0, 1.0, 8);
    //  Normalized Number of opponents
    learner->addStateVariable("noOp", 0.0, 1.0, 4);
    //learner->addStateVariable("noOpDefs", 0.0, 1.0, 4);
    //learner->addStateVariable("noUs", 0.0, 1.0, 4);
    //learner->addStateVariable("noUsAtts", 0.0, 1.0, 4);
    learner->addStateVariable("bx", -1.0, 1.0, 5);
    learner->addStateVariable("by",  0.0, 1.0, 3);

    learner->initAction("acc", 0, 26, 27);
    //learner->initLearner(8, 0.9, 0.3, 0.1, 0.1, 0.5, 0.01);

    learner->initLearner(8, 0.9, 0.3, 0.5, 0.8, 0.5, 0.01);

    newEpisode = true;
    newRun = true;
    newPhase = false;
    run = 0;
    episode = 0;
    step = 0;
    stepLength = 0;

    isLearning = true;
    totalLearningSteps = 0.0;
    totalExploitingSteps = 0.0;
    totalLearningSuccess = 0.0;
    totalExploitingSuccess = 0.0;
    totalLearningEpisodes = 0.0;
    totalExploitingEpisodes = 0.0;
    sumOfRewards = 0.0;

    wait = 50;
    randBallX = randBallY = 0.0;


    seedPoints.clear();
    seedPoints.append(Vector2D(2.9, 1.9));
    seedPoints.append(Vector2D(1.5, 1.9));
    seedPoints.append(Vector2D(0.0, 1.9));
    seedPoints.append(Vector2D(0.0, 0.0));
    seedPoints.append(Vector2D(0.0, -1.9));
    seedPoints.append(Vector2D(1.5, -1.9));
    seedPoints.append(Vector2D(2.9, -1.9));
    seedIndex = 4;

}

CPolicyLearner::~CPolicyLearner()
{

}

void CPolicyLearner::randomizeBall(bool seed)
{
    if (seed)
    {
        // Update Seed
        //randBallX = (drand48() * (_FIELD_WIDTH / 2.0)) - (_FIELD_WIDTH / 4.0);
        //randBallX = (drand48() * 4.0) - (2.0);
        //randBallY = (drand48() * (_FIELD_HEIGHT / 1.0)) - (_FIELD_HEIGHT / 2.0);
        randBallX = seedPoints.at(seedIndex).x;
        randBallY = seedPoints.at(seedIndex).y;
        seedIndex++;
        if (seedIndex >= seedPoints.size())
        {
            seedIndex = 0;
        }
    }
    else
    {
        wm->ball->setReplace(Vector2D(randBallX, randBallY), Vector2D(0.0, 0.0));
    }
}

void CPolicyLearner::resetBall()
{
    wm->ball->setReplace(Vector2D(0.0, 0.0), Vector2D(0.0, 0.0));
}

void CPolicyLearner::randomizeAgents()
{
    int ac;

    ac = wm->our.activeAgentsCount();
    for (int i = 0; i < ac; i++)
    {
		wm->our.active(i)->setReplace(Vector2D(-1.5, (i - floor(ac / 2)) * 0.5 ), 0.0);
    }

    ac = wm->opp.activeAgentsCount();
    for (int i = 0; i < ac; i++)
    {
        wm->opp.active(i)->setReplace(Vector2D(1.5, (i - floor(ac / 2)) * 0.5 ), 0.0);
    }
}

void CPolicyLearner::updateVars()
{

    if (wm->ball->pos != Vector2D(Vector2D::ERROR_VALUE,Vector2D::ERROR_VALUE))
    {
        wm->statAnal->updateVar("sb", wm->ball->pos);
    }

    for (int i = 0; i < wm->our.activeAgentsCount(); i++)
    {
        int index = wm->our.activeAgentID(i);
        wm->statAnal->updateVar(QString("so%1").arg(index), wm->our[index]->pos);
    }

    for (int i = 0; i < wm->opp.activeAgentsCount(); i++)
    {
        int index = wm->opp.activeAgentID(i);
        wm->statAnal->updateVar(QString("sp%1").arg(index), wm->opp[index]->pos);

    }

    /*
    for (int r = 0; r < (2 * _MAX_NUM_PLAYERS) + 1; r++)
    {
        QString name;
        if (r < _MAX_NUM_PLAYERS)
        {
            name = QString("so%1").arg(r);
        }
        else if ((r >= _MAX_NUM_PLAYERS) && (r < (2 *_MAX_NUM_PLAYERS)))
        {
            name = QString("sp%1").arg(r - _MAX_NUM_PLAYERS);
        }
        else
        {
            name = QString("sb");
        }

        double reg1 = wm->statAnal->getVar(name).regions.at(0).stat.count;
        double reg2 = wm->statAnal->getVar(name).regions.at(1).stat.count;
        double reg3 = wm->statAnal->getVar(name).regions.at(2).stat.count;

        draw(QString("%1 | %2 %3 %4").arg(name).arg(reg1).arg(reg2).arg(reg3), Vector2D(-1.0, (-2.5 + r * 0.2)), QColor("black"));
    }
    */



    double sumBall = wm->statAnal->getVar("sb").stat.count;
    if (sumBall == 0.0)
    {
        qWarning() << "Sum of Ball in all regions is 0.0";
        ballDef = ballMid = 0.0;
    }
    else
    {
        ballDef = wm->statAnal->getVar("sb").regions.at(0).stat.count / sumBall;
        ballMid = wm->statAnal->getVar("sb").regions.at(1).stat.count / sumBall;
    }

    double sumOur = 0.0;
    double sumOpp = 0.0;

    for (int i = 0; i < _MAX_NUM_PLAYERS; i++)
    {
        sumOur += wm->statAnal->getVar(QString("so%1").arg(i)).stat.count;
        sumOpp += wm->statAnal->getVar(QString("sp%1").arg(i)).stat.count;
    }

    if (sumOur == 0.0)
    {
        qWarning() << "Sum of Our Team Robots in all regions is 0.0";
        ourTeamDef = ourTeamMid = 0.0;
    }
    else
    {
        double reg0sum = 0.0;
        double reg1sum = 0.0;
        for (int i = 0; i < _MAX_NUM_PLAYERS; i++)
        {
            reg0sum += wm->statAnal->getVar(QString("so%1").arg(i)).regions.at(0).stat.count;
            reg1sum += wm->statAnal->getVar(QString("so%1").arg(i)).regions.at(1).stat.count;
        }
        ourTeamDef = reg0sum / sumOur;
        ourTeamMid = reg1sum / sumOur;
    }

    if (sumOpp == 0.0)
    {
        qWarning() << "Sum of Opp Team Robots in all regions is 0.0";
        oppTeamOff = oppTeamMid = 0.0;
    }
    else
    {
        double reg0sum = 0.0;
        double reg1sum = 0.0;
        for (int i = 0; i < _MAX_NUM_PLAYERS; i++)
        {
            reg0sum += wm->statAnal->getVar(QString("sp%1").arg(i)).regions.at(0).stat.count;
            reg1sum += wm->statAnal->getVar(QString("sp%1").arg(i)).regions.at(1).stat.count;
        }
        oppTeamOff = reg0sum / sumOpp;
        oppTeamMid = reg1sum / sumOpp;
    }

    draw(QString("Our(%1 %2 %3) Opp(%4 %5 %6) Ball(%7 %8 %9")
         .arg(ourTeamDef,0,'f',1)
         .arg(ourTeamMid,0,'f',1)
         .arg(1.0-ourTeamDef-ourTeamMid,0,'f',1)
         .arg(oppTeamOff,0,'f',1)
         .arg(oppTeamMid,0,'f',1)
         .arg(1.0-oppTeamOff-oppTeamMid,0,'f',1)
         .arg(ballDef,0,'f',1)
         .arg(ballMid,0,'f',1)
         .arg(1.0 -ballDef-ballMid,0,'f',1)
         , Vector2D(-3.0, -2.6), QColor("red"));

    ourLastBallOwner = ourBallOwner;
    oppLastBallOwner = oppBallOwner;

    knowledge->findBallOwners();

    ourBallOwner = knowledge->ballOwner;
    oppBallOwner = knowledge->oppBallOwner;

    if ((ourLastBallOwner != ourBallOwner) || (oppLastBallOwner != oppBallOwner))
    {
        lastBallState = ballState;

        if ((ourBallOwner == -1) && (oppBallOwner == -1))
        {
            ballState  = CPolicyLearner::_FREE;
        }
        else if ((ourBallOwner > -1) && (oppBallOwner == -1))
        {
            ballState = CPolicyLearner::_OURS;
        }
        else if ((ourBallOwner == -1) && (oppBallOwner > -1))
        {
            ballState = CPolicyLearner::_THEIRS;
        }
    }

    draw(QString("Our: %1 OurLast: %2 Opp: %3 OppLast: %4 STATE : %5")
         .arg(ourBallOwner)
         .arg(ourLastBallOwner)
         .arg(oppBallOwner)
         .arg(oppLastBallOwner)
         .arg(ballState)
         , Vector2D(-3.0, -2.1), QColor("red"));

    experimentState = CPolicyLearner::_GAMEON;

    Vector2D lastBallInFieldPos;
    if ((!wm->field->fieldRect().contains(wm->ball->pos)) && (wm->ball->hist.count() > 20))
    {
        for (int i = (wm->ball->hist.count() - 1); i >= 0; i--)
        {
            if (wm->field->fieldRect().contains(wm->ball->hist.at(i).pos))
            {
                lastBallInFieldPos = wm->ball->hist.at(i).pos;
                break;
            }
        }
        //ballLine(wm->ball->pos, lastBallInFieldPos);
        ballLine.assign(wm->ball->pos, lastBallInFieldPos);
        Vector2D sol1, sol2;
        int nSol = wm->field->fieldRect().intersection(ballLine, &sol1, &sol2);
        if (nSol == 0)
        {
            qWarning() << "Ball is out, however intersection point not found!";
            ballOutPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
        }
        else if (nSol == 1)
        {
            if (sol1 != Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE))
            {
                ballOutPos = sol1;
            }
            else
            {
                ballOutPos = sol2;
            }
            //wm->ball->setReplace(Vector2D(0.0, 0.0), Vector2D(0.0, 0.0));
            experimentState = CPolicyLearner::_OUTOFPLAY;
        }
        else if (nSol == 2)
        {
            //qWarning() << "Ball is out, however intersection points are two!";
            ballOutPos = Vector2D(Vector2D::ERROR_VALUE, Vector2D::ERROR_VALUE);
        }


    }

    isOwnerChanged = false;
    if (experimentState == CPolicyLearner::_GAMEON)
    {
        if (
                ((ballState == CPolicyLearner::_OURS) && (lastBallState == CPolicyLearner::_THEIRS)) ||
                ((ballState == CPolicyLearner::_THEIRS) && (lastBallState == CPolicyLearner::_OURS))
           )
        {
            //isOwnerChanged = true;
        }
    }

    if (ballOutPos.valid())
    {
        draw(ballOutPos, 0, QColor("red"));
    }





    //policy()->v_PlayMaker_KickThreshold->setDouble()

    // Opps
    numOfOpps = wm->opp.activeAgentsCount() / 5.0;


//    int _g, _d;
//    knowledge->findOppDefenders(_d, _g);

//    numOfOppDefenders = (_d /*+ _g*/) / wm->opp.activeAgentsCount();

    // Ours
    numOfUs = wm->our.activeAgentsCount() / 5.0;
    numOfAttackers = double(knowledge->getNumOfAttackers()) / double(wm->our.activeAgentsCount());
    numOfDeffenders = double(knowledge->getNumOfDeffenders()) / double(wm->our.activeAgentsCount());

    shootThreshold = policy()->PlayMaker_KickThreshold();
    oneTouchThreshold = policy()->PlayMaker_OneTouchKickThreshold();

    draw(QString("NO: %1 NOD: %2 NA: %3 ND : %4 ST: %5 OT: %6 Epsilon: %7")
         .arg(numOfOpps)
         .arg(numOfOppDefenders)
         .arg(numOfAttackers)
         .arg(numOfDeffenders)
         .arg(shootThreshold)
         .arg(oneTouchThreshold)
         .arg(learner->getEpsilon())
         , Vector2D(-3.0, -2.3), QColor("red"));

    /*
    policy()->v_Formation_Manual->setBool(true);
    policy()->v_Formation_Goalie->setDouble(1.0);
    policy()->v_Formation_Attack->setDouble(2.0);
    policy()->v_Formation_Defense->setDouble(2.0);
    */

}


void CPolicyLearner::update()
{

    updateVars();


    bool endless = false;
    stepLength++;
    if ((stepLength > 2200) && (!doStep) && (wm->ball->vel.length() < 0.5))
    {
        stepLength = 0;
        //randomizeBall();
        doStep = true;
        endless = true;
        if (showDebug) debug("Step length is too much, reset ball, doStep!", D_EXPERIMENT | D_MANI, QColor("cyan"));
    }


    if (wait > 0)
    {
        wm->gs->set(GameState::GAME_OFF);
        //randomizeBall(false);
        randomizeAgents();
        wait--;
        return;
    }

    bool found = false;
    segment = -1;
    double* rewardArray;
    QString* textArray;


    wm->gs->set(GameState::GAME_ON);
    if ((experimentState == CPolicyLearner::_OUTOFPLAY) && (ballOutPos.valid()))
    {
        //wm->ball->setReplace(ballOutPos * 0.95, Vector2D(0.0,0.0));
        wm->gs->set(GameState::HALTED);
        //qDebug() << "Ball Out Pos : " << ballOutPos.x << ballOutPos.y;
        ourAction = ((ballState == _OURS) || ((ballState == _FREE) && (lastBallState == _OURS)));
        //qDebug() << "Ball State:" << ballState << " Last Ball State:" << lastBallState;
        rewardArray = (ourAction) ? ourRewards : oppRewards;
        textArray = (ourAction) ? ourStr : oppStr;

        found = true;

        segment = 0;
        double min = fieldSegments[segment].dist(ballOutPos);

        for (int i = 1; i < 12; i++)
        {
            double d = fieldSegments[i].dist(ballOutPos);
            if (d < min)
            {
                found = true;
                min = d;
                segment = i;
            }
        }

    }

    if ((found) || (false/*isOwnerChanged*/))
    {
        doStep = true;
        stepLength = 0;
    }

    if (step > 6)
    {
        newEpisode = true;
        sumSteps += step;
        astep += step;
    }

    if (newEpisode)
    {
        if (showDebug) debug(QString("Starting Episode %1").arg(episode), D_EXPERIMENT | D_MANI);
        newEpisode = false;
        episode++;
        step = 0;


        /* Reset */

//        wm->statAnal->randomizeVar("sb");
//        for (int index = 0; index < _MAX_NUM_PLAYERS; index++)
//        {
//            wm->statAnal->randomizeVar(QString("so%1").arg(index));
//            wm->statAnal->randomizeVar(QString("sp%1").arg(index));
//        }

//        conf()->v_Coach_kickThreshold->setDouble(drand48());
//        policy()->v_PlayMaker_OneTouchKickThreshold->setDouble(drand48());



//        conf()->v_Coach_kickThreshold->setDouble(drand48());
//        policy()->v_PlayMaker_OneTouchKickThreshold->setDouble(drand48());

        //TODO: Randomize Number of Opponents

        doStep = false;
//        updateVars(); //Again
        learner->newEpisode();

        //wait = 50;

//        if (episode > 1)
//        {
//            debug(QString("%1 Sum Rewards in last Episode : %2,%3").arg(episode).arg(sumRewardInEpisode).arg(sumGoalsInEpisode), D_EXPERIMENT | D_MANI, QColor("red"));
//            //learner->saveLearnerToFile(QString("./learning/save/test1-%1.ml").arg(episode).toAscii(), true);
//        }

        learner->setEpsilon(learner->getEpsilon() * 0.9);

        sumRewardInEpisode = 0.0;
        sumGoalsInEpisode = 0.0;
    }

    // RL Step
    if (doStep)
    {
        if (found)
        {
            if (showDebug) debug(QString("Ball is out, %1").arg(textArray[segment]), D_EXPERIMENT | D_MANI, QColor("blue"));
        }
        if (isOwnerChanged)
        {
            if (showDebug) debug("Ball Owner Changed.", D_EXPERIMENT | D_MANI, QColor("blue"));
        }

        step++;

        double reward;

        reward = 0;
//        if ((isOwnerChanged) && (ballState == _OURS))
//        {
//            reward = changeOwnerUsReward;
//        }
//        else if ((isOwnerChanged) && (ballState == _THEIRS))
//        {
//            reward = changeOwnerOppReward;
//        }
//        else if (found)

        if (endless)
        {
            reward = -1.0;
            qDebug() << "endless";
            resetBall();
        }
        if (found)
        {
            reward = rewardArray[segment];

            if ((reward == ourRewards[5]) || (reward == oppRewards[5])) {
                sumGoalsInEpisode++;
                newEpisode = true;
            }
        }

        if (showDebug) debug(QString("Reward : %1").arg(reward), D_EXPERIMENT | D_MANI);
        sumRewardInEpisode = sumRewardInEpisode + reward;

        if (isLearning)
        {
            if (firstTime == false)
            {
                learner->step(reward, false, false);
                learner->afterStep();
            }
            else
            {
                firstTime = false;
            }
        }

//        if (found)
//        {
//            randomizeBall();
//        }



        if (!firstTime)
            debug(QString("%1,%2")/*.arg(episode)*/.arg(reward).arg(newEpisode ? 1.0 : 0.0), D_EXPERIMENT | D_MANI, QColor("red"));

        if (found)
        {


//            debug(QString("Segment: %1").arg(segment), D_EXPERIMENT | D_MANI);
            if (segment == 4)
            {
                wm->ball->setReplace((ourAction) ? Vector2D(0.0, 0.0) : Vector2D(2.8, 1.9), Vector2D(0.0, 0.0));
            }
            else if (segment == 6)
            {
                wm->ball->setReplace((ourAction) ? Vector2D(0.0, 0.0) : Vector2D(2.8, -1.9), Vector2D(0.0, 0.0));
            }
            else if ((segment == 5) || (segment == 11) || (segment == 0) || (segment == 10))
            {
                resetBall();
            }
            else // 1 2 3 7 8 9
            {
                double _bx = ballOutPos.x;
                double _by = (ballOutPos.y > 0.0) ? 1.9 : -1.9;
                wm->ball->setReplace(Vector2D(_bx, _by), Vector2D(0.0, 0.0));
            }

            wm->ball->hist.clear();
        }
        //randomizeBall(true);
        randBallX = wm->ball->getReplPos().x;
        randBallY = wm->ball->getReplPos().y;
//        debug(QString("Ball : %1 %2").arg(randBallX).arg(randBallY), D_EXPERIMENT | D_MANI);


//        learner->setStateVariableValue("bhDef", ballDef);
//        learner->setStateVariableValue("bhMid", ballMid);
        learner->setStateVariableValue("oDef", ourTeamDef);
        learner->setStateVariableValue("oMid", ourTeamMid);
        learner->setStateVariableValue("pDef", oppTeamOff);
        learner->setStateVariableValue("pMid", oppTeamMid);
//        learner->setStateVariableValue("psTres", shootThreshold);
//        learner->setStateVariableValue("otTres", oneTouchThreshold);
        learner->setStateVariableValue("noOp", numOfOpps);
//        learner->setStateVariableValue("noOpDefs", numOfOppDefenders);
//        learner->setStateVariableValue("noUs", numOfUs);
//        learner->setStateVariableValue("noUsAtts", numOfAttackers);
        learner->setStateVariableValue("bx", randBallX / _FIELD_WIDTH);
        learner->setStateVariableValue("by", fabs(randBallY) / (_FIELD_HEIGHT / 2.0));



        if (showDebug) debug(QString("Current State Vars: %1 %2 %3 %4 %5 %6 %7")
              .arg(ourTeamDef)
              .arg(ourTeamMid)
              .arg(oppTeamOff)
              .arg(oppTeamMid)
              //.arg(shootThreshold)
              //.arg(oneTouchThreshold)
              .arg(numOfOpps)
              .arg(randBallX / _FIELD_WIDTH)
              .arg( fabs(randBallY) / (_FIELD_HEIGHT / 2.0))
              , D_EXPERIMENT | D_MANI);



        int acc = learner->getActionSingetonValue();

        int accFormation = acc % 3;
        int accShoot = ((int) floor(acc / 3.0)) % 3;
        int accOneTouch = floor(floor(acc / 3.0) / 3.0);

        switch (accFormation)
        {
        case 0:
            policy()->v_Formation_Manual->setBool(true);
            policy()->v_Formation_Goalie->setDouble(1.0);
            policy()->v_Formation_Defense->setDouble(2.0);
            policy()->v_Formation_Attack->setDouble(2.0);
            if (showDebug) debug("Formation : 1-2-2", D_EXPERIMENT | D_MANI, QColor("green"));
            break;
        case 1:
            policy()->v_Formation_Manual->setBool(true);
            policy()->v_Formation_Goalie->setDouble(0.0);
            policy()->v_Formation_Defense->setDouble(2.0);
            policy()->v_Formation_Attack->setDouble(3.0);
            if (showDebug) debug("Formation : 0-2-3", D_EXPERIMENT | D_MANI, QColor("green"));
            break;
        case 2:
            policy()->v_Formation_Manual->setBool(true);
            policy()->v_Formation_Goalie->setDouble(1.0);
            policy()->v_Formation_Defense->setDouble(1.0);
            policy()->v_Formation_Attack->setDouble(3.0);
            if (showDebug) debug("Formation : 1-1-3", D_EXPERIMENT | D_MANI, QColor("green"));
            break;
        }

        switch (accShoot)
        {
        case 0:
            policy()->v_PlayMaker_KickThreshold->setDouble(0.1);
            if (showDebug) debug("Set kick threshold : 0.1", D_EXPERIMENT | D_MANI, QColor("green"));
            break;
        case 1:
            policy()->v_PlayMaker_KickThreshold->setDouble(0.5);
            if (showDebug) debug("Set kick threshold : 0.5", D_EXPERIMENT | D_MANI, QColor("green"));
            break;
        case 2:
            policy()->v_PlayMaker_KickThreshold->setDouble(0.9);
            if (showDebug) debug("Set kick threshold : 0.9", D_EXPERIMENT | D_MANI, QColor("green"));
            break;
        }

        switch (accOneTouch)
        {
        case 0:
            policy()->v_PlayMaker_OneTouchKickThreshold->setDouble(0.1);
            if (showDebug) debug("Set onetouch threshold : 0.1", D_EXPERIMENT | D_MANI, QColor("green"));
            break;
        case 1:
            policy()->v_PlayMaker_OneTouchKickThreshold->setDouble(0.5);
            if (showDebug) debug("Set onetouch threshold : 0.5", D_EXPERIMENT | D_MANI, QColor("green"));
            break;
        case 2:
            policy()->v_PlayMaker_OneTouchKickThreshold->setDouble(0.9);
            if (showDebug) debug("Set onetouch threshold : 0.9", D_EXPERIMENT | D_MANI, QColor("green"));
            break;
        }

        if (isLearning) learner->beforeStep();


        //CARE

//        wm->statAnal->resetVar("sb");
//        for (int index = 0; index < _MAX_NUM_PLAYERS; index++)
//        {
//            wm->statAnal->resetVar(QString("so%1").arg(index));
//            wm->statAnal->resetVar(QString("sp%1").arg(index));
//        }


        doStep = false;
        wait = 50;

        randomizeAgents();

    }

}


#endif
