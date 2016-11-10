#ifndef DIRECT_H
#define DIRECT_H

#include "playoff.h"

class CDirect : public CPlayOff
{
public:
    CDirect();
    virtual QString whoami() {return "Direct";}

};

#endif // DIRECT_H
