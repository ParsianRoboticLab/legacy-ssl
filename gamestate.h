// game_state.h
//
// This class implements the transition system that defines the rules
// of the game.  The state changes based on the input from the referee
// and whether the ball has been kicked since the last referee
// command.  Calling the method transition() with these inputs will
// have the class maintain the state of the game.
//
// In addition, their are query methods for determining the current
// game state and what behavior is allowed in these game states.
//
// Created by:  Michael Bowling (mhb@cs.cmu.edu)
//
/* LICENSE: */

#ifndef __game_state_h__
#define __game_state_h__

// We want this to be usable by other teams, so below are the
// constants that this module makes use of.  If this is not being used
// within the CMDragons system then the #if 1 should be changed to a
// #if 0.
//
// These constants should match with those found in constants.h.
//

#if 0
#include <constants.h>
#else
#define TEAM_BLUE 0
#define TEAM_YELLOW 1
#endif

#include "commands.h"


class GameState {
public:
  static const int GAME_ON;
  static const int GAME_OFF;
  static const int HALTED;

  static const int KICKOFF;
  static const int PENALTY;
  static const int DIRECT;
  static const int INDIRECT;
  static const int RESTART;

  static const int BLUE;
  static const int YELLOW;

  static const int READY;
  static const int NOTREADY;

  //added
  static const int BALLPLACEMENT;
  static const int HALF_TIME;
  static const int PENALTY_SHOOTOUT;

  int state;
  int gametimes=0;

  // The set of possible states are:
  //
  // { GAME_ON, GAME_OFF, HALTED, NEUTRAL,
  //   { { KICKOFF, PENALTY, DIRECT, INDIRECT } |
  //     { BLUE, YELLOW } | { READY, NOTREADY } } }
  //

  int color;
  int yellowscore,bluescore;
public:

  GameState();

  void init(int _color);
  int get();
  void set(int _state);

  // This is the state machine transition function.  It takes the last
  // ref_command as input
  void transition(char ref_command);
  bool gameOn();
  bool restart();
  bool ourRestart();
  bool theirRestart();
  bool kickoff();
  bool ourKickoff();
  bool theirKickoff();
  bool penaltyKick();
  bool ourPenaltyKick();
  bool theirPenaltyKick();
  bool directKick();
  bool ourDirectKick();
  bool theirDirectKick();
  bool indirectKick();
  bool ourIndirectKick();
  bool theirIndirectKick();
  bool freeKick();
  bool ourFreeKick();
  bool theirFreeKick();
  bool canMove();
  bool allowedNearBall();
  bool canKickBall();
  int ourScore();
  int oppScore();
  //added
  bool ballPlacement();
  bool ourBallPlacement();
  bool theirBallPlacement();
  bool halfTimeLineUp();
  bool penalty_shootout();


};

#endif
