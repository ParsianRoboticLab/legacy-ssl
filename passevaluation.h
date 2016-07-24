#ifndef PASSEVALUATION_H
#define PASSEVALUATION_H

#include "worldmodel.h"

class PassEvaluation
{    
public:    
    enum Mode {
        Simple       = 0,
        OneTouch     = 1,
        Through      = 2
    };
    struct Pass {
        Vector2D senderPos;
        Vector2D receiverPos;
        int sender;
        int receiver;
        int frames_elapsed_before_pass_sent;            
        bool chip;
        Mode mode;
        void invalidate();
        bool valid();
    };
    PassEvaluation();
    void createPass(int _passSender, int _passReceiver, Mode _mode);
    void sendPass(bool chip=false);
    void receivedPass();
    int  receivingAgent();    
    void sendingPass();        
    int  passesSent();
    int  totalPassesSent();
    QList<Pass> passes();
    QList<Pass> totalPasses();    
    Pass pass();
    void reset();
    bool isInvolved(int agent);
    bool isSender(int agent);
    bool isReceiver(int agent);    
    bool inProgress();
    void resetStatistics();
private:        
    //QList<Pass> passlist, totalpasslist;
    int passcount;
    int totalpasscount;
    Pass current;
    bool inprogress;
    int receiving;
};

extern PassEvaluation passEval;

#endif // PASSEVALUATION_H
