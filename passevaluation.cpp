#include "passevaluation.h"

PassEvaluation passEval;

PassEvaluation::PassEvaluation()
{
    inprogress = false;
    receiving = -1;
    current.invalidate();
    passcount = totalpasscount = 0;
}

void PassEvaluation::createPass(int _passSender, int _passReceiver, Mode _mode)
{
    if (inprogress) {
        sendPass(false);
        receivedPass();
    }
    current.invalidate();
    current.chip = false;    
    current.mode = _mode;
    current.sender = _passSender;
    current.receiver = _passReceiver;
    current.senderPos = wm->our[_passSender]->pos;
    current.receiverPos = wm->our[_passReceiver]->pos;        
    inprogress = true;    
}

void PassEvaluation::sendPass(bool chip)
{
    if (inprogress)
    {        
        receiving = current.receiver;
        current.chip = chip;
        passcount ++;totalpasscount ++;
//        passlist.append(current);
        current.invalidate();
        inprogress = false;
    }
}

void PassEvaluation::receivedPass()
{
    receiving = -1;    
}

int PassEvaluation::receivingAgent()
{
    return receiving;
}

void PassEvaluation::sendingPass()
{
    if (inprogress)
        current.frames_elapsed_before_pass_sent++;
}

int PassEvaluation::passesSent()
{
    return passcount;
//    return passlist.length();
}

int PassEvaluation::totalPassesSent()
{
    return totalpasscount;
    //return passesSent() + totalpasslist.length();
}

QList<PassEvaluation::Pass> PassEvaluation::passes()
{
//    return passlist;
}

QList<PassEvaluation::Pass> PassEvaluation::totalPasses()
{
    QList<PassEvaluation::Pass> t;
//    t.append(totalpasslist);
//    t.append(passlist);
    return t;
}

void PassEvaluation::resetStatistics()
{
//    totalpasslist.append(passlist);
//    passlist.clear();
}

PassEvaluation::Pass PassEvaluation::pass()
{    
    return current;
}

void PassEvaluation::reset()
{
    current.invalidate();
    receiving = -1;
    inprogress = false;
}

bool PassEvaluation::isInvolved(int agent)
{
    return (current.sender==agent) || (current.receiver==agent);
}

bool PassEvaluation::isSender(int agent)
{
    return (current.sender==agent);
}

bool PassEvaluation::isReceiver(int agent)
{
    return (current.receiver==agent);
}

bool PassEvaluation::inProgress()
{
    return inprogress;
}

void PassEvaluation::Pass::invalidate()
{
    sender = -1;
    receiver = -1;
    frames_elapsed_before_pass_sent = 0;    
    mode = PassEvaluation::Simple;
    senderPos.invalidate();
    receiverPos.invalidate();    
    chip = false;
}

bool PassEvaluation::Pass::valid()
{
    return sender==-1;
}
