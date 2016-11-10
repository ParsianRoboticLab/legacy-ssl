#ifndef KICKOFF_H
#define KICKOFF_H

#include "playoff.h"

class CKickoff : public CPlayOff
{
public:
    CKickoff();
    virtual QString whoami() {return "KickOff";}

};

#endif // KICKOFF_H
