#ifndef POLICYLEARNER_H
#define POLICYLEARNER_H
#if 0

#include "worldmodel.h"
#include "coach.h"
#include "knowledge.h"

#include "statisticalanalysis.h"

#include "../../../mani-thesis/mlearner/mtilesarsalearner.h"
#include "../../../mani-thesis/mlearner/mtileqlearner.h"
#include "../../../mani-thesis/mlearner/mfuzzyaclearner.h"
#include "../../../mani-thesis/mlearner/mfuzzyqlearner.h"



class CPolicyLearner
{

enum ballStateSet {
    _OURS = 0,
    _THEIRS = 1,
    _FREE = 2
};

enum gameStateSet {
    _GAMEON = 0,
    _OUTOFPLAY = 1,
};

private:
    CCoach* coach;
    /* Def : Mid : Off Regions are relative to our team (LEFT) */
    double ourTeamDef;
    double ourTeamMid;
    double oppTeamOff;
    double oppTeamMid;
    double ballDef;
    double ballMid;
    /* -1 to +1 * Relative to our team */
    double normalizedResult;
    double numOfOpps;
    double numOfUs;
    double numOfOppDefenders;
    double numOfAttackers;
    double numOfDeffenders;
    double shootThreshold;
    double oneTouchThreshold;

    int ourBallOwner;
    int oppBallOwner;
    int ourLastBallOwner;
    int oppLastBallOwner;

    Vector2D ballOutPos;
    ballStateSet ballState;
    ballStateSet lastBallState;
    gameStateSet experimentState;

    Segment2D fieldSegments[12];
    double ourRewards[12];
    double oppRewards[12];
    double changeOwnerUsReward;
    double changeOwnerOppReward;
    QString ourStr[12];
    QString oppStr[12];

    bool isOwnerChanged;
    Segment2D ballLine;
    int segment;
    bool ourAction;

    int wait;

    /* RL */

    MTileSarsaLearner* learner;
    //MTileQLearner* learner;
    //MFuzzyACLearner* learner;

    bool doStep;
    long int stepLength;
    bool newEpisode;
    bool newRun;
    bool newPhase;
    bool isLearning;
    int run;
    int phase;
    int episode;
    long int step;
    long int sumSteps;
    double sumRewardInEpisode;
    double sumGoalsInEpisode;
    long int rate;
    double astep;
    double arate;
    double totalLearningSteps;
    double totalExploitingSteps;
    double totalLearningSuccess;
    double totalExploitingSuccess;
    double totalLearningEpisodes;
    double totalExploitingEpisodes;
    double sumOfRewards;
    int sCount;
    int maxsCount;
    bool sucRuns;

    bool firstTime;
    bool showDebug;
    double randBallX;
    double randBallY;
    int seedIndex;
    QList<Vector2D> seedPoints;
public:
    CPolicyLearner(CCoach* c);
    ~CPolicyLearner();

    void updateVars();
    void update();

    void resetBall();
    void randomizeBall(bool seed);
    void randomizeAgents();
};

#endif
#endif // POLICYLEARNER_H
