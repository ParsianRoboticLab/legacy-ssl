#ifndef MIXTEAMCHALLENGE_H
#define MIXTEAMCHALLENGE_H

#include <mainapplication.h>
#include <mixteamhandler.h>

void CMainApplication::MixTeamChallenge()
{
    knowledge->updateGameState();
    CMixTeamHandler *mstr = new CMixTeamHandler();
    mstr->master();

    CMixTeamHandler *slv = new CMixTeamHandler();
    slv->slave();
}

#endif // MIXTEAMCHALLENGE_H
