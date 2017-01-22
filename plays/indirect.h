#ifndef INDIRECT_H
#define INDIRECT_H

#include "playoff.h"

class CIndirect : public CPlayOff {
public:
    CIndirect();
    virtual ~CIndirect();
    virtual QString whoami() {return "InDirect";}
};

#endif // INDIRECT_H
