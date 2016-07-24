#ifndef TECHNICALCHALLENGE_H
#define TECHNICALCHALLENGE_H

#include "mainapplication.h"
#include <joystick.h>
#include <QColor>
#include <callibration.h>
#include <defensepositioning.h>

class CPassChallengeAtt{
public:
	CPassChallengeAtt(CSoccer *);
	~CPassChallengeAtt();
	void resetEverything();
	bool canSendPass(int , int);
	void sendPass(int , int , int);
	void recievePass(int , int , bool);
	void position(int , int);
	void exectue();

    CSkillGotoPoint* gp[4];
	CSkillKick* kick[4];
	Vector2D tars[10];
	QList <Rect2D> rects;
	int lastPos[4];
	int kickSpeed;
	int mode;
        QTime timeOut;
        bool timeCantSendPass;
	bool forceReceivePass;
	bool gotoPoint[4];
	bool moves[4];
    int CID[4];
	int robotMaxVel , forceMode;
	QList <Vector2D> ballPosHist;
	CSoccer *soccer;
};

/////////////////////////////////////////////////////////////////////////////

class CPassChallengeDef{
public:
	CPassChallengeDef(CSoccer *);
	~CPassChallengeDef();
	void exectue();

    CSkillGotoPoint *gp[4];
	Vector2D tars[4][2];
	bool flags[4] , start;
	int cnt , num;
	int CID[4];
	int robotSpeed;
	CSoccer *soccer;
};

/////////////////////////////////////////////////////////////////////////////

class CKhersChallengeAtt{
public:
	CKhersChallengeAtt( CSoccer *);
	~CKhersChallengeAtt();
	void execute();
    int determinePassSender();
	int determinePassReceiver();
    void sendPass(int , int);

    CSkillGotoPoint *gp[4];
	Vector2D tars[4];
	int passSender , passReceiver;
	QList <Rect2D> rects;
	bool gotoPoint[4];
	CSoccer *soccer;
};

/////////////////////////////////////////////////////////////////////////////

class CKhersChallengeDef{
public:
	CKhersChallengeDef( CSoccer *);
	~CKhersChallengeDef();
	void execute();

	CSoccer *soccer;
};

//////////////////////////////////////////////////////////////////////////////

#endif // TECHNICALCHALLENGE_H

