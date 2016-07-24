#ifndef CONDITIONS_H
#define CONDITIONS_H

#include <QString>
#include <QList>
#include <QMap>

#define DEF_CONDITION(condition) \
    class condition : public CGameCondition \
    { \
    public: \
        condition(); \
        virtual bool check(QStringList params); \
    }
#define CONDITION(condition, Name) \
    bool condition##_registered = CGameConditions::registerCondition(new condition()); \
    condition::condition() : CGameCondition() {name=Name;} \
    bool condition::check(QStringList params)


class CGameCondition
{
protected:
    QString name;
    bool lastResult;
    QMap<QString, bool> lastResults;
public:
    QString getName();
    CGameCondition();
    virtual bool check(QStringList params);
};

    class CGameConditions
{
public:
    static QList<CGameCondition*> conditions;
    static bool registerCondition(CGameCondition* condition);
    static bool check(QString condition, QStringList params);
};

DEF_CONDITION(CConditionAlways);
DEF_CONDITION(CConditionDone);
DEF_CONDITION(CConditionNever);
DEF_CONDITION(CConditionOurDirect);
DEF_CONDITION(CConditionOurIndirect);
DEF_CONDITION(CConditionOurKickOff);
DEF_CONDITION(CConditionOurPenalty);
DEF_CONDITION(CConditionTheirDirect);
DEF_CONDITION(CConditionTheirIndirect);
DEF_CONDITION(CConditionTheirKickOff);
DEF_CONDITION(CConditionTheirPenalty);
DEF_CONDITION(CConditionStop);
DEF_CONDITION(CConditionStart);
DEF_CONDITION(CConditionTechnical);
DEF_CONDITION(CConditionHalt);
DEF_CONDITION(CConditionTimeOut);
DEF_CONDITION(CConditionBallInOurField);
DEF_CONDITION(CConditionBallInTopField);
DEF_CONDITION(CConditionBallMoved);
DEF_CONDITION(CConditionBallInside);
DEF_CONDITION(CConditionRectIsClear);
DEF_CONDITION(CConditionCount);
DEF_CONDITION(CConditionIndirectType);
DEF_CONDITION(CConditionNoPlaymaker);
DEF_CONDITION(CConditionBallOurs);
DEF_CONDITION(CConditionOppDifendersCount);
DEF_CONDITION(CConditionExecuting);
DEF_CONDITION(CConditionTimeIn);
DEF_CONDITION(CConditionPlayMakerInside);
DEF_CONDITION(CConditionWarmup);
DEF_CONDITION(CConditionExists);
DEF_CONDITION(CConditionCheck);
DEF_CONDITION(CConditionOppCorner);
DEF_CONDITION(CConditionOurCorner);
DEF_CONDITION(CConditionBallFromTop);
DEF_CONDITION(CConditionBallFromBottom);
DEF_CONDITION(CConditionTheyAreKhafan);
DEF_CONDITION(CConditionTheyAreShirje);
DEF_CONDITION(CConditionAgentSwitched);
DEF_CONDITION(CConditionTheyHaveBlocker);


#endif // CONDITIONS_H
