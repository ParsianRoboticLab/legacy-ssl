#ifndef MIXTEAMCHALLENGE_H
#define MIXTEAMCHALLENGE_H

#include <mainapplication.h>
#include "mixteamhandler.h"
#include "mixteamreader.h"

void CMainApplication::MixTeamChallenge()
{
    knowledge->updateGameState();
    CMixTeamHandler *mstr = new CMixTeamHandler();
    mstr->master();

    CMixTeamHandler *slv = new CMixTeamHandler();
    slv->slave();
    qDebug() << "****" << knowledge->ssize;

}

#endif // MIXTEAMCHALLENGE_H
