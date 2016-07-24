#ifndef SHOTPROFILE_H
#define SHOTPROFILE_H

#include <QList>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QTime>

#include <worldmodel.h>
#include <agent.h>
#include "base.h"
#include "agent.h"


struct SShotSpeed{
    int a , v;
};

struct SAgent{
    struct SShotSpeed speed[15];
};

struct SProfileShot{
    SAgent agent[12];
};

class CShotProfile
{
public:
    CShotProfile();
    ~CShotProfile();
    void execute( CAgent ** , int id , int speed , double sampleDist);
    QList <double> TValues, RValues;
    QTime timeStart;
    SProfileShot  profileShot;
    Vector2D BallPos;
    Property(bool , EndFlag , endFlag );
    Property(bool , FirstTimeBallPos , firstTimeBallPos );
    Property(int , BallStopCntr , ballStopCntr );
    Property(bool , Finish , finish);
    Property(bool , KickOrChip , kickOrChip );
};

#endif // SHOTPROFILE_H
