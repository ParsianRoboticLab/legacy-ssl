#ifndef MIXTEAMCHALLENGE_H
#define MIXTEAMCHALLENGE_H

#include <mainapplication.h>
#include "mixteamhandler.h"
#include "mixteamreader.h"

void CMainApplication::MixTeamChallenge()
{
    knowledge->updateGameState();
    static CMixTeamHandler *mstr = new CMixTeamHandler();
    mstr->master();

    static CMixTeamHandler *slv = new CMixTeamHandler();

    //if we are master, attribute is true and if we are slave, attribute is true
    slv->slave(true);

    static CMixTeamCoach* mixCoach = new CMixTeamCoach();
    mixCoach->testDefense();

//    slv->execute();

    debug(QString("GID: %1").arg(knowledge->mixGoaleID), D_ATOUSA);
}

#endif // MIXTEAMCHALLENGE_H
