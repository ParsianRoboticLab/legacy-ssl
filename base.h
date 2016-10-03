#ifndef BASE_H
#define BASE_H


#define REVNUM "$Rev: 2484 $"

#define LARGE_FIELD

#define KK_PLAYON
#define PARSIANWORKSHOP
//#define SIMULATION_MODE
//#define GAME_MODE

//#define NAVIGATION_CHALLENGE
//IMP the above line must be comamented

/* Ennumerations */
//#define velProblem1
//#define velProblem2
#include "geom.h"
enum SKILLINITVAL{
    SKILLGOTOPOINT = 0,
    SKILLGOTOPOINTAVOID = 1,
    SKILLKICK = 2,
    SKILLKICKONETOUCH = 3,
    SKILLRECEIVEPASS = 4
};

enum kkSkill {
    SkillGotopoint = 0,
    SkillGotopointAvoid = 1,
    SkillKick = 2,
    SkillReceivePass = 3,
    SkillOneTouch = 4,
    SkillMark = 5
};

////////////////////////////////////<MAHI>
enum robotExtraDetail {
    MNOTHING,
    MGOALKIPPER,
    MDEFENCE,
    MOFFENCE,
    MPASSER,
    MSHOOTER,
    MONETOUCHER,
    MRECIVER,
    MWELLMOTION
};

enum ProfileMode {
    PKICK,
    PCHIP,
    PSHOOT,
    PPASS,
    PONETOUCH
};

namespace DynamicEnums {

enum DynamicMode {
    NoMode,
    DefenseClear,
    NoPlanExeption,
    HighProb,
    Fast,
    Critical,
    NotWeHaveBall,
    Plan
};

enum DynamicSkill {
    NoSkill,
    Ready,
    Pass,
    Mark,
    CatchBall,
    Shot,
    Move,
    Keep,
    Chip
};

enum DynamicRole {
    NoRole,
    PlayMaker,
    Position
};

enum DynamicRegion {
    NoMatter,
    Near,
    Forward,
    Far,
    Goal,
    Best,
    Supporter
};

}


////////////////////////////////////</MAHI>


enum EQuiescentMode
{
    _SIMULATION_MODE,
    _REAL_MODE
};


enum EActionType
{
    _FORWARD,
    _BACKWARD,
    _LEFT,
    _RIGHT,
    _TURN_CW,
    _TURN_CCW,
    _STOP_NAVIGATION,
    _STOP_OTHER,
        _KICK,
        _CHIP,
    _ROLLER,
    _STOP_ALL,
    _BEEP
};


enum EDefaultPlayMode
{
    _OFFEND_MODE,
    _DEFEND_MODE,
    _FREE_MODE
};

enum ETeamSideType
{
    _SIDE_RIGHT,
    _SIDE_LEFT
};

enum ETeamColorType
{
    _COLOR_BLUE,
    _COLOR_YELLOW
};

struct robotAttr {
    int index;
    int agent;
    int skillNum;
    bool isAng;
};

enum POffSkills {
    NoSkill = 0,
    PassSkill = 1,
    ReceivePassSkill = 2,
    ShotToGoalSkill = 3,
    ChipToGoalSkill = 4,
    OneTouchSkill = 5,
    MoveSkill = 6,
    ReceivePassIASkill = 7
};

struct playOffRobot {
    Vector2D pos;
    AngleDeg angle;
    double tolerance;
    int targetIndex;
    int targetAgent;
    POffSkills skill[3];
    int skillData[3][2];
    int skillSize;
};

struct POInitPos {
    Vector2D ball;
    Vector2D Agent[6];
};

/////////////////forceStart & playOff
struct kkValue {
    int IDs[6];
    double value;
    int agentSize;
};

struct kkRobot {
    int id;
    Vector2D pos;
    Vector2D vel;
    Vector2D dir;
};


/* Structures */

// http://small-size.informatik.uni-bremen.de/referee:protocol
struct GameStatePacket
{
    char cmd;                      // current referee command
    unsigned char cmd_counter;     // increments each time new command is set
    unsigned char goals_blue;      // current score for blue team
    unsigned char goals_yellow;    // current score for yellow team
    unsigned short time_remaining; // seconds remaining for current game stage (network byte order)
};


/* Defines */

#define _NUM_PLAYERS    16
#define _MAX_NUM_PLAYERS 16


/* Mathematical */
#define _PI         3.14159265358979323
#define	_RAD2DEG    (180.0/_PI)
#define _DEG2RAD    (_PI/180.0)

/* Motion Constants */
//#define LAST_PROTOCOL
#define newProtocol
#ifdef LAST_PROTOCOL
    #define _BIT_RESOLUTION             31
    #define _PACKET_SIZE                5
#else
        #define _BIT_RESOLUTION             127
        #define _PACKET_SIZE                14

        #define _NEW_PACKET_SIZE            14
#define _NewProtocolRobot 4
#endif
#define _BANG_BANG_NONLINEAR_SWITCH 0.300


#ifndef LARGE_FIELD
/* Monitor Constants (m)*/
#define _STADIUM_WIDTH   7.400
#define _STADIUM_HEIGHT  5.400

#else

#define _STADIUM_WIDTH   10.90
#define _STADIUM_HEIGHT  7.70
#endif

/* COM Port Constants */
#define _SIM_COM_PORT   "/dev/ttyS6"
#define _REAL_COM_PORT  "/dev/ttyS0"


/* Skills */
#define _DEFENCE_DIST       0.110


#ifndef LARGE_FIELD

/* Field */                 //brasil
#define _FIELD_WIDTH            6.05
#define _FIELD_HEIGHT           4.050
#define _FIELD_MARGIN_HEIGHT    0.250
#define _FIELD_MARGIN_WIDTH     0.500
#define _FIELD_PENALTY          0.750
#define _GOAL_WIDTH             0.700
#define _GOAL_RAD               0.800
#define _GOAL_DEPTH             0.200
#define _PENALTY_WIDTH          1.950
#define _CENTER_CIRCLE_RAD      0.500
#define _PENALTY_AREA_CIRCLE_X  -3.20
#define _PENALTY_AREA_CIRCLE_RAD  1.100
#define _MAX_DIST               sqrt(_FIELD_WIDTH * _FIELD_WIDTH + _FIELD_HEIGHT * _FIELD_HEIGHT)

#else

/* Field */
#define _FIELD_WIDTH            9.0
#define _FIELD_HEIGHT           6.00
#define _FIELD_MARGIN_HEIGHT    0.675
#define _FIELD_MARGIN_WIDTH     0.675
#define _FIELD_PENALTY          1.000
#define _GOAL_WIDTH             1.000
#define _GOAL_RAD               1.000
#define _GOAL_DEPTH             0.250
#define _PENALTY_WIDTH          2.500
#define _CENTER_CIRCLE_RAD      0.500
#define _PENALTY_AREA_CIRCLE_X  -4.20
#define _PENALTY_AREA_CIRCLE_RAD  1.350
#define _PARSIAN_BOUNDRY         4.40
#define _MAX_DIST               sqrt(_FIELD_WIDTH * _FIELD_WIDTH + _FIELD_HEIGHT * _FIELD_HEIGHT)

#endif

/* Common Includes */
#include "geom.h"

#define Property(type,name,local) \
        public: inline type& get##name() {return local;} \
        public: inline void set##name(type val) {local = val;} \
        protected: type local

#define PropertyGet(type,name,local) \
        public: inline type& get##name() {return local;} \
        protected: type local
#define InitVal(val) val = _##val

#define foragents(i) for (int i=0;i<_NUM_PLAYERS;i++)
#define for_visible_agents(AGENTS, i) for (int i=0;i<_NUM_PLAYERS;i++) if (AGENTS[i]->isVisible())

#endif // BASE_H

/*

TODO

* Referee: http://www.qtcentre.org/forum/archive/index.php/t-3347.html

*/
