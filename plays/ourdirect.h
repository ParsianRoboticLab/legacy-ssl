#ifndef OURDIRECT_H
#define OURDIRECT_H

#include "masterplay.h"

class COurDirect;
typedef void (COurDirect::*directPlayFunction)(int symmetry);
typedef bool (COurDirect::*directPlayCondition)(int symmetry);

class COurDirect : public CMasterPlay{
public:
	COurDirect();
	~COurDirect();
	void execute_0();
	void execute_1();
	void execute_2();
	void execute_3();
	void execute_4();
	void execute_5();
	void execute_6();
	void init(QList <int> _agents , QMap<QString , EditData*> *_editData);

	void (COurDirect::*lastPlayExeced)(int);

	PLAY(theirOneThirdFlankAreaNoAdd3,TheirOneThirdFlankAreaNoAdd3)
	PLAY(theirOneThirdFlankAreaSimple3,TheirOneThirdFlankAreaSimple3)
	PLAY(theirOneThirdCenterArea3,TheirOneThirdCenterArea3)
	PLAY(theirOneThirdFlankAreaAddDefense3,TheirOneThirdFlankAreaAddDefense3)
        PLAY(theirOneThirdFlankAreaTwoDummies3,TheirOneThirdFlankAreaTwoDummies3)
        PLAY(theirOneThirdFlankAreaShootThemInTheAss3,TheirOneThirdFlankAreaShootThemInTheAss3)

	PLAY(middleFlankAreaOffensive3,MiddleFlankAreaOffensive3)
	PLAY(middleFlankAreaDefensive3,MiddleFlankAreaDefensive3)

	PLAY(middleCenterAreaOffensive3,MiddleCenterAreaOffensive3)
	PLAY(middleCenterAreaDefensive3,MiddleCenterAreaDefensive3)

	PLAY(ourOneThirdAreaOffensive3,OurOneThirdAreaOffensive3)
	PLAY(ourOneThirdAreaDefensive3,OurOneThirdAreaDefensive3)

	PLAY(ourOneThirdAreaOffensive2,OurOneThirdAreaOffensive2)
	PLAY(ourOneThirdAreaDefensive2,OurOneThirdAreaDefensive2)
	PLAY(ourOneThirdAreaOnlyTwoAgents,OurOneThirdAreaOnlyTwoAgents)

	PLAY(middleFlankAreaOffensive2,MiddleFlankAreaOffensive2)
	PLAY(middleFlankAreaDefensive2,MiddleFlankAreaDefensive2)
	PLAY(middleFlankAreaOnlyTwoAgents,MiddleFlankAreaOnlyTwoAgents)

	PLAY(middleCenterAreaOffensive2,MiddleCenterAreaOffensive2)
	PLAY(middleCenterAreaDefensive2,MiddleCenterAreaDefensive2)
	PLAY(middleCenterAreaOnlyTwoAgents,MiddleCenterAreaOnlyTwoAgents)

	PLAY(theirOneThirdFlankArea2,TheirOneThirdFlankArea2)
        PLAY(theirOneThirdFlankAreaShootThemInTheAss2,TheirOneThirdFlankAreaShootThemInTheAss2)
	PLAY(theirOneThirdCenterArea2,TheirOneThirdCenterArea2)

	PLAY(middleCenterAreaDefensive1,MiddleCenterAreaDefensive1)

        PLAY(newMiddleCenterAreaOffensive3,NewMiddleCenterAreaOffensive3)
        PLAY(falsePlay,falsePlay)

	Vector2D pointForPass, onTheBallLine;
	int oneToucherID;
	Vector2D whoIsBetterForPass(int symmetry);
private:
	int cyclesToExecute;

	QList<QPair<directPlayFunction,directPlayCondition> > defPlaysCan;
	QList<QPair<directPlayFunction,directPlayCondition> > offPlaysCan;

	QList<directPlayFunction> defPlays;
	QList<directPlayFunction> offPlays;

	void reset();

	void passOrNot();
	void swapNearestDefAndMid(Vector2D p, int midIdx);
	void addNearestRobot2Mid(Vector2D p);
	void setPosIntentions();
	int getAgent(int id);
	int getNearestAgent(Vector2D p);
        bool canPassWithoutChip();
	QList<directPlayFunction> getValidPlays(QList<QPair<directPlayFunction,directPlayCondition> > candidates);

	QList<Vector2D> staticVec[MAX_POSITIONERS];
	QList<int> cyclesToWait[MAX_POSITIONERS];
	QList<double> escapeRadius[MAX_POSITIONERS];
	QList<positioningType> facePoints[MAX_POSITIONERS];
	QList<holdingPoints> staticPoints;
	int waitBeforeExecution;
};


#endif // OURDIRECT_H
