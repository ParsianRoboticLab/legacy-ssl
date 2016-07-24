#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <agent.h>

#define MAX_TRAJECTORY_STEPS 1000

struct ObjectPosVel
{
        float xPos, yPos, rotation;
  float xVel, yVel, rotVel;
};

ObjectPosVel objectPosVel(Vector2D pos, Vector2D dir, Vector2D vel, float rotVel);
class ObjectPath
{

private:
  ObjectPosVel path[MAX_TRAJECTORY_STEPS];
  int count;

public:

  ObjectPath() { count = 0; }
  void clearPath();

  int getNumSteps() const { return count; }

  bool get(int step, ObjectPosVel & nextStep);

  bool set(int step, const ObjectPosVel & nextStep);
  bool set(int step, const float xPos, const float yPos, const float rotation,
              const float xVel, const float yVel, const float rotVel);

  bool addStep(const ObjectPosVel & nextStep);
  bool addStep(const float xPos, const float yPos, const float rotation,
              const float xVel, const float yVel, const float rotVel);

  bool writeToFile(const char * fname);
};

float distance(float x1, float y1, float x2, float y2);

//=================================================
// TIMING FUNCTIONS

void expectedPathTime(  ObjectPosVel start,
                        ObjectPosVel final,
                        float frameRate,
                        float maxAccel,
                        float maxVelocity,
                        float& pathTime
                      );

void expectedFastPathTime(  ObjectPosVel start,
                            ObjectPosVel final,
                            float frameRate,
                            float maxAccel,
                            float maxVelocity,
                            float& pathTime
                          );

//=================================================
// WALL COLLISION DETECTION FUNCTION

// Returns true if the zero-final-velocity path
// would ever exit the bounding conditions
bool detectWallCollision( ObjectPosVel start,
                          ObjectPosVel final,
                          float frameRate,
                          float maxAccel,
                          float maxVelocity,
                          float minX,
                          float maxX,
                          float minY,
                          float maxY
                      );

//=================================================
// TRAJECTORY GENERATION FUNCTIONS

void zeroFinalVelocityPath( ObjectPosVel start,
                            ObjectPosVel final,
                            float frameRate,
                            float maxAccel,
                            float maxThetaAccel,
                            float maxVelocity,
                            float maxThetaVelocity,
                            ObjectPath& thePath,
                            int numberOfSteps = -1
                          );

void fastPath( ObjectPosVel start,
               ObjectPosVel final,
               float frameRate,
               float maxAccel,
               float maxVelocity,
               ObjectPath& thePath,
               int numberOfSteps);

//=================================================
// TRAJECTORY GENERATION FUNCTIONS

void zeroFinalVelocityTheta(ObjectPosVel start,
                            ObjectPosVel final,
                            float frameRate,
                            float maxThetaAccel,
                            float maxThetaVelocity,
                            float & nextAngularVelocity
                          );



//=================================================
// PROPORTIONAL GAIN FUNCTIONS

void proportionalGainPath(ObjectPosVel start,
                          ObjectPosVel final,
                          float frameRate,
                          float translationalGain,
                          float angularGain,
                          float maxVelocity,
                          float maxThetaVelocity,
                          ObjectPath& thePath
                         );


void proportionalGainTheta(ObjectPosVel start,
                           ObjectPosVel final,
                           float frameRate,
                           float angularGain,
                           float maxThetaVelocity,
                           float & nextAngularVelocity
                          );




class Navigation
{
private:
    Vector2D targetPos, targetDir;
    CAgent* agent;
    int pathGenerated;
    int frame;
    int currentPos;
    double pathError;
    ObjectPath path;
public:
    Navigation();
    void setAgent(CAgent* _agent);
    void setTarget(Vector2D pos, Vector2D dir);
    double getTime();
    void run();
};

#endif // NAVIGATION_H
