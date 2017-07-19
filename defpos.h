#ifndef DEFPOS_H
#define DEFPOS_H

#include "geom.h"
#include "worldmodel.h"

struct kk2Angles {
    double angle1;
    double angle2;
};

struct kkDefPos {
    int size;
    double overDef;
    Vector2D pos[5];
};


class CDefPos {
public:
    CDefPos();
    kkDefPos getDefPositions(Vector2D _ballPos, int _size, double _limit1, double _limit2);
    //HMD
    Vector2D getIntersectionWithPenaltyAreaDef(double _tempBestRadius , Segment2D _seg, bool isMixTeam);
    bool isInPenaltyAreaDef(double _tempBestRadius , Vector2D vec);
    //HMD Finish
    double nearRadius[2];
    bool isNearPenaltyArea;

    Vector2D getXYByAngle(double _angle, double _radius);
    double getRobotAngle(double _radius);
    kk2Angles getIntersections(Vector2D _ballPos, double _radius);
    double findBestRadius(int _numOfDefs);
private:
    kkDefPos tempDefPos;
    double getAngleByXY(Vector2D _point);
    double oneDefThr;
    double penaltyAreaOffset;
    double penaltyAreaRadius;
    Circle2D penaltyAreaCircle;
};


#endif // DEFPOS_H
