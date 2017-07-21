#ifndef PlayMake_H
#define PlayMake_H

#include <role.h>
#include <behaviours/offensive.h>
#include <QMap>

#define PASS_BEHAVS 5

enum PlayMakerKickModes{FixedPass , FixedShoot , NoMode};

class CRolePlayMakeInfo : public CRoleInfo
{
public:
    CRolePlayMakeInfo(QString _roleName);
    CAgent* passReceiver(CAgent* self, int p, QList<int> passables);
    CAgent* bestPassReceiver(bool indirect);
    QList<int> oneToucher;
    double oneToucherDist2Ball;
    void reset(){}
};

class CRolePlayMake : public CRole
{
protected:
    bool goalKeeperForward=false;
    Hyst ballTop;
    Hyst ballLeft;
    int qqHist;
    int rn;
    CSkillKick* kick;
    CSkillSpinBack* spin;
    CSkillGotoBall* gotoball;
    CSkillKickOneTouch* onetouch;
    CSkillGotoPointAvoid* gotopoint;
    CSkillHitTheBall* hitTheBall;
    CSkillTurn* turn;
    int kickDecision;
    int onetouching;
    CAgent* lastAgent;
    int lastPassReceiver;
    bool startKicked;
    int gameMode;
    bool indirect, direct, kickoff;
    int penaltyRand;
    Vector2D penaltyTarget;
    bool firstKick=true;
    int penaltyCounter;
    //decision making
    QList<int> oppBlockers;
    double w,ang,coming;
    int nextPlayMaker;
    bool stopped;//, clear;
    Vector2D target;
    bool spinBack();
    void passShootNew();
    void stopBehindBall(bool penalty = false);
    int indirectPassRecverSelectedFrame;
    int lastFrameCrowded;
    bool spinside;
    bool forceRedecide;
    Vector2D initialPoint;
    CAgent* indirectRecver;
    //    CBehaviourSpinPass spinPass;
    QMap<int, double> passProbs;
    QList<CAgent*> passreceivers;
    //void checkPassSequences(QList<CAgent*> s);
    double maxPassProbability;
    QList<CAgent*> bestPassSequence;
    int ballavoidanceState;
    //CBehaviourPass* pass;
    CBehaviourKick* shoot;
    CBehaviourPass* fixedPass;
    CBehaviourPass* pass[PASS_BEHAVS];
    CBehaviourChipPass* chippass[PASS_BEHAVS];
    CBehaviourSpinPass* spinPass[PASS_BEHAVS];
    CBehaviourChipToGoal* chipToGoal;
    CBehaviourKickBetweenTheirDefenders *kickToDefense;
    CBehaviourKick* clear;
    double lastDecisionTime;
    int waitBeforePass;
    int orderRushInPlenalty;
    bool orderedRush;
    QTime changeDirPenaltyStrikerTime;
    bool timerStartFlag;

public:
    DEF_ROLE(CRolePlayMake)
    void executeOurDirect();
    void executeOurIndirect();
    void executeOurKickOff();
    void executeOurPenalty();
    void executeOurPenaltyShootout();
    void theirPenaltyPositioning();
    int choosePenaltyStrategy();
    void ShootoutSwitching(bool isChip);
    void firstKickInShootout(bool isChip);
    void kickInitialShootout();
    int getPenaltychipSpeed();
    double lastBounce();
    bool ShootPenalty();
    void executeDefault();
    void resetOffPlays();
    void resetPlayMake();
    bool canScoreGoal();
    void kickPass( int kickSpeed );
    enum KickPassMode{KickPassFirst , KickPassSecond};
    enum penaltyStrategy{pgoaheadShoot , pchipShoot , pshootDirect};

    KickPassMode kickPassMode;
    int kickPassCyclesWait;
    Vector2D finalTarget;

    QList<QPair<int, QMap<double, int> > > lastBounceProfileData;
    QTextStream out;
    QFile lastBounceDataFile;

    virtual void generateFromConfig(CAgent *a);
    virtual CSkillConfigWidget* generateConfigWidget(QWidget *parent);
    virtual void parse(QStringList params);

    SkillProperty(CRolePlayMake, Vector2D, PointToPass, pointToPass);
    SkillProperty(CRolePlayMake, Vector2D, PointToShoot, pointToShoot);
    SkillProperty(CRolePlayMake, int, KickMode , kickMode);
    SkillProperty(CRolePlayMake, int, CyclesToWait, cyclesToWait);
    SkillPropertyNoSet(CRolePlayMake, int, CyclesExecuted, cyclesExecuted);
    SkillProperty(CRolePlayMake, bool, Slow, slow);
    SkillProperty(CRolePlayMake, bool, ChipInPenaltyArea, chipPenaltyArea);
    SkillProperty(CRolePlayMake, bool, Chip, chip);
    SkillProperty(CRolePlayMake, bool, Through, through);
    SkillProperty(CRolePlayMake, bool, IndirectGoogooli, indirectGoogooli);
    SkillProperty(CRolePlayMake, bool, IndirectTiny, indirectTiny);
    SkillProperty(CRolePlayMake, bool, IndirectKhafan, indirectKhafan);
    SkillProperty(CRolePlayMake, bool, KickoffWing, kickoffWing);
    SkillProperty(CRolePlayMake, bool, Kickoffmode, kickoffmode);
    SkillProperty(CRolePlayMake, bool, NoStop, nostop);
    SkillProperty(CRolePlayMake, bool, ChipIndirect, chipIndirect);
    SkillProperty(CRolePlayMake, bool, NoKick, noKick);
    SkillProperty(CRolePlayMake, int, PassReceiver, passReceiver);
    SkillProperty(CRolePlayMake, bool, ManualPassReceive, manualPassReceive);
    SkillProperty(CRolePlayMake, bool, AllowOneTouch, allowonetouch);
    SkillProperty(CRolePlayMake, bool, AllowOnePass, allowonepass);
    SkillProperty(CRolePlayMake, QList<int>, Passables, passables);
    SkillProperty(CRolePlayMake, QList<int>, PassSequence, passsequence);
    SkillProperty(CRolePlayMake, bool, FollowSequence, followsequence);
    SkillProperty(CRolePlayMake, bool, ShadowPass, shadowPass);
    SkillProperty(CRolePlayMake, bool, KickToTheirDefense, kickToTheirDefense);
    SkillProperty(CRolePlayMake, bool, JustTurn, justTurn);
    SkillProperty(CRolePlayMake, QString, LocalAgentPassTarget, localAgentPassTarget);
    SkillProperty(CRolePlayMake, bool, Khers, khers);
    SkillProperty(CRolePlayMake, bool, LongChip, longchip);
    SkillProperty(CRolePlayMake, bool, ChipToOppGoal, chipToOppGoal);
    SkillProperty(CRolePlayMake, bool, ShadowyPoint, shadowyPoint);
    SkillProperty(CRolePlayMake, bool, SafeIndirect , safeIndirect);
    SkillProperty(CRolePlayMake, bool, SpinBool , spinBool);
    SkillProperty(CRolePlayMake, bool, KickPower, kickPower);
};


#endif // PlayMake_H
