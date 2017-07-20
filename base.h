
#ifndef BASE_H
#define BASE_H


#define REVNUM "$Rev: 2484 $"

// TODO : remove this macro
#define KK_PLAYON
/* Ennumerations */
#include "geom.h"

enum SKILLINITVAL{
    SKILLGOTOPOINT = 0,
    SKILLGOTOPOINTAVOID = 1,
    SKILLKICK = 2,
    SKILLKICKONETOUCH = 3,
    SKILLRECEIVEPASS = 4
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
    SKICK,
    SCHIP,
    PSHOOT,
    PPASS,
    PONETOUCH
};

namespace DynamicEnums {

enum DynamicMode {
    NoMode,
    CounterAttack,
    DefenseClear,
    NoPlanExeption,
    HighProb,
    Fast,
    Critical,
    NotWeHaveBall,
    Plan,
    BallInOurField,
    NoPositionAgent,
    BallInOppJaw
};

enum DynamicSkill {
    NoSkill,
    //PlayMake Skills
    Pass,
    CatchBall,
    Shot,
    Keep,
    Chip,
    //Positioning
    Ready,
    OneTouch,
    Move,
    Dribble
};

enum DynamicRegion {
    NoMatter,
    Near,
    Forward,
    Far,
    Goal,
    Reflect,
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

#define _NUM_PLAYERS    12
#define _MAX_NUM_PLAYERS 12


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



#define _STADIUM_WIDTH   14.80
#define _STADIUM_HEIGHT  10.40

/* COM Port Constants */
#define _SIM_COM_PORT   "/dev/ttyS6"
#define _REAL_COM_PORT  "/dev/ttyUSB0"


/* Skills */
#define _DEFENCE_DIST       0.110


/* Field */
#define _FIELD_WIDTH            13.40
#define _FIELD_HEIGHT           9.00
#define _FIELD_MARGIN_HEIGHT    0.700
#define _FIELD_MARGIN_WIDTH     0.700
#define _FIELD_PENALTY          1.000
#define _GOAL_WIDTH             1.000
#define _GOAL_RAD               1.000
#define _AHZ_GOAL_RAD           1.150
#define _GOAL_DEPTH             0.250
#define _PENALTY_WIDTH          2.500
#define _CENTER_CIRCLE_RAD      0.500
#define _PENALTY_AREA_CIRCLE_X  -4.20
#define _PENALTY_AREA_CIRCLE_RAD  1.350
#define _PARSIAN_BOUNDRY         4.40
#define _MAX_DIST               sqrt(_FIELD_WIDTH * _FIELD_WIDTH + _FIELD_HEIGHT * _FIELD_HEIGHT)

#define _MIXTEAM_FIELD_WIDTH            13.4
#define _MIXTEAM_FIELD_HEIGHT           9.00

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
