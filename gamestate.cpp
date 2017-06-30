#include "gamestate.h"

//setup constants
const int GameState::GAME_ON =  (1 << 0);
const int GameState::GAME_OFF = (1 << 1);
const int GameState::HALTED =   (1 << 2);

const int GameState::KICKOFF =  (1 << 3);
const int GameState::PENALTY =  (1 << 4);
const int GameState::DIRECT =   (1 << 5);
const int GameState::INDIRECT = (1 << 6);
//added
const int GameState::BALLPLACEMENT = (1 << 12);
const int GameState::HALF_TIME=(1 << 13);


const int GameState::RESTART = (KICKOFF | PENALTY | DIRECT | INDIRECT | BALLPLACEMENT);

const int GameState::BLUE =     (1 << 8);
const int GameState::YELLOW =   (1 << 9);

const int GameState::READY =    (1 << 10);
const int GameState::NOTREADY = (1 << 11);


GameState::GameState()
{
  color = BLUE; state = GAME_OFF;
  yellowscore=bluescore=0;
}

int GameState::ourScore()
{
    if (color == BLUE) return bluescore;
    return yellowscore;
}

int GameState::oppScore()
{
    if (color == YELLOW) return bluescore;
    return yellowscore;
}

void GameState::init(int _color) { color = (_color == TEAM_BLUE) ? BLUE : YELLOW; }

int GameState::get() { return state; }
void GameState::set(int _state) { state = _state; }

// This is the state machine transition function.  It takes the last
// ref_command as input
void GameState::transition(char ref_command) {
    if (ref_command == COMM_GOAL_BLUE) bluescore++;
    if (ref_command == COMM_SUBGOAL_BLUE) bluescore--;
    if (ref_command == COMM_GOAL_YELLOW) yellowscore++;
    if (ref_command == COMM_SUBGOAL_YELLOW) yellowscore--;
if (ref_command == COMM_HALT) {
  state = HALTED; return; }

if (ref_command == COMM_STOP) {
  state = GAME_OFF; return; }

if (ref_command == COMM_START) {
  state = GAME_ON; return; }

if (ref_command == COMM_READY && state & NOTREADY) {
  state &= ~NOTREADY; state |= READY; return; }

if(ref_command == COMM_TIMEOUT_YELLOW
        || ref_command == COMM_TIMEOUT_BLUE
        || ref_command == COMM_HALF_TIME) {
    state=HALF_TIME; return; }

if (state & READY) {
  state = GAME_ON; return; }

if (state == GAME_OFF) {
  switch (ref_command) {
  case COMM_KICKOFF_BLUE:
    state = KICKOFF | BLUE | NOTREADY; return;
  case COMM_KICKOFF_YELLOW:
    state = KICKOFF | YELLOW | NOTREADY; return;

  case COMM_PENALTY_BLUE:
    state = PENALTY | BLUE | NOTREADY; return;
  case COMM_PENALTY_YELLOW:
    state = PENALTY | YELLOW | NOTREADY; return;

  case COMM_DIRECT_BLUE:
    state = DIRECT | BLUE | READY; return;
  case COMM_DIRECT_YELLOW:
    state = DIRECT | YELLOW | READY; return;

  case COMM_INDIRECT_BLUE:
    state = INDIRECT | BLUE | READY; return;
  case COMM_INDIRECT_YELLOW:
    state = INDIRECT | YELLOW | READY; return;

//added
  case COMM_BALLPLACEMENT_BLUE:
    state = BALLPLACEMENT | BLUE | READY; return;
  case COMM_BALLPLACEMENT_YELLOW:
    state = BALLPLACEMENT | YELLOW | READY; return;


  default: break;
  }
}
}

bool GameState::gameOn() { return (state == GAME_ON); }

bool GameState::restart() { return (state & RESTART); }
bool GameState::ourRestart() { return restart() && (state & color); }
bool GameState::theirRestart() { return restart() && ! (state & color); }

bool GameState::kickoff() { return (state & KICKOFF); }
bool GameState::ourKickoff() { return kickoff() && (state & color); }
bool GameState::theirKickoff() { return kickoff() && ! (state & color); }

bool GameState::penaltyKick() { return (state & PENALTY); }
bool GameState::ourPenaltyKick() { return penaltyKick() && (state & color); }
bool GameState::theirPenaltyKick() { return penaltyKick() && ! (state & color); }

bool GameState::directKick() { return (state & DIRECT); }
bool GameState::ourDirectKick() { return directKick() && (state & color); }
bool GameState::theirDirectKick() { return directKick() && ! (state & color); }

bool GameState::indirectKick() { return (state & INDIRECT); }
bool GameState::ourIndirectKick() { return indirectKick() && (state & color) ;}
bool GameState::theirIndirectKick() { return indirectKick() && ! (state & color); }

bool GameState::freeKick() { return directKick() || indirectKick(); }
bool GameState::ourFreeKick() { return ourDirectKick() || ourIndirectKick(); }
bool GameState::theirFreeKick() { return theirDirectKick() || theirIndirectKick(); }

//added
bool GameState::ballPlacement() { return (state & BALLPLACEMENT); }
bool GameState::ourBallPlacement() { return ballPlacement() && (state & color); }
bool GameState::theirBallPlacement() { return ballPlacement() && ! (state & color); }
bool GameState::halfTimeLineUp(){return state & HALF_TIME;}


bool GameState::canMove() { return (state != HALTED); }

bool GameState::allowedNearBall() {
return gameOn() || (state & color); }

bool GameState::canKickBall() {
return gameOn() || (ourRestart() && (state & READY)); }



