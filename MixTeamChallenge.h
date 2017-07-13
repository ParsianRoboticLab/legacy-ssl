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
    slv->slave();


//    slv->execute();


    qDebug() << "****" << knowledge->ssize;
//    qDebug() << "^^^^" << knowledge->activesInField.size();
    qDebug() << "&&&&" << knowledge->getActiveAgents().size();
    qDebug() << "^^^^" << knowledge->activesInField.size();
    debug(QString("GID: %1").arg(knowledge->mixGoaleID), D_ATOUSA);
}

#endif // MIXTEAMCHALLENGE_H
