
#include "masterplay.h"

PositioningPlan CMasterPlay::position;
CRolePlayMake CMasterPlay::playMakeRole(NULL);
CMarkPlan CMasterPlay::markPlan;
CRoleBlock CMasterPlay::blockRole(NULL);

CMasterPlay::CMasterPlay(){
    executedCycles = 0;
    lockAgents = false;
    //defenseN = 2;
}

CMasterPlay::~CMasterPlay(){

}

void CMasterPlay::initMaster(){
    blockAgent = NULL;
    playMakeAgent = NULL;
    positionAgents.clear();
    if(knowledge->getGameState() != CKnowledge::Start)
        markAgents.clear();
    stopAgents.clear();
    masterStaticPoints.clear();
    staticInited = false;
    //defenseN = 2;
    formationName = "";


}

bool CMasterPlay::canScore(){
    QList<int> ourRelaxed, theirRelaxed;
    theirRelaxed.clear();
    ourRelaxed.clear();
    for( int i = 0 ; i < wm->our.activeAgentsCount() ; i++ )
        ourRelaxed.append(wm->our.activeAgentID(i));
    return 	getOpenness(wm->ball->pos,wm->field->oppGoalL(),wm->field->oppGoalR(),ourRelaxed,theirRelaxed) > .2;
}

int CMasterPlay::getDefenseNum()
{
    return policy()->Formation_Defense();
}

void CMasterPlay::setEditData(QMap<QString, EditData *> *_editData){
    editData = _editData;
}

void CMasterPlay::setAgentsID(QList<int> _agentsID){
    agentsID.clear();
    agentsID.append(_agentsID);
}

void CMasterPlay::setFormation(QString _formationName){
    formationName = _formationName;
}

void CMasterPlay::setStaticPoints(QList< holdingPoints > _staticPoints){
    staticInited = true;
    masterStaticPoints = _staticPoints;
}

void CMasterPlay::resetPositioning(){
    position.reset();
}

void CMasterPlay::resetPlayMaker(){
    playMakeRole.resetPlayMake();
    playMakeRole.parse(QStringList());
}


// This function is critical, any manipulation would
// result in bad bugs!
void CMasterPlay::appendRemainingsAgents(QList<CAgent *> &_list){

    QList <int> remainings;

    for( int i=0 ; i<agentsID.size(); i++ ){
        remainings.append(agentsID.at(i));
    }

    if( blockAgent ){
        remainings.removeOne(blockAgent->self()->id);
    }
    if( playMakeAgent ){
        remainings.removeOne(playMakeAgent->self()->id);
    }

    if( markAgents.size() ){
        for( int i=0 ; i<markAgents.size() ; i++ ){
            remainings.removeOne(markAgents.at(i)->self()->id);
        }
    }

    if( positionAgents.size() ){
        for( int i=0 ; i<positionAgents.size() ; i++ ){
            remainings.removeOne(positionAgents.at(i)->self()->id);
        }
    }

    for( int i=0 ; i<remainings.size() ; i++ ){
        _list.append(knowledge->getAgent(remainings.at(i)));
    }
}

void CMasterPlay::choosePlayMaker(){
    int playMakeID = -1;
    QList <CAgent *> playAgents;
    playAgents.clear();
    for( int i=0 ; i < agentsID.size() ; i++ ){
        playAgents.append(knowledge->getAgent(agentsID.at(i)));
    }


    for( int i=0 ; i<knowledge->agentsWithIntention.size() ; i++ ){
        int id = knowledge->agentsWithIntention.at(i);
        if( agentsID.contains(id) ){
            CAgent *agnt = knowledge->getAgent(id);
            if( agnt->intention->M_type == "playmake" ){
                playMakeID = id;
            }
        }
    }

    if( playMakeID == -1 ){
        playMakeID = knowledge->newFastestSelector(playAgents);
        if( playMakeID != -1 ){
            CAgent *agnt = knowledge->getAgent(playMakeID);
            if( agnt != NULL ){
                agnt->playMakeIntent.assign(playMakeID , knowledge->frameCount);
                agnt->intention = &agnt->playMakeIntent;
            }
        }
    }

    ///////////////////////////////////////

    ///////////////////////////////////////

    if( playMakeID != -1 ){
        playMakeAgent = knowledge->getAgent(playMakeID);
    }
}

void CMasterPlay::chooseBlocker(){

    int blockID = -1;
    QList <CAgent *> playAgents;
    playAgents.clear();
    for( int i=0 ; i<agentsID.size() ; i++ ){
        playAgents.append(knowledge->getAgent(agentsID.at(i)));
    }

    for( int i=0 ; i<knowledge->agentsWithIntention.size() ; i++ ){
        int id = knowledge->agentsWithIntention.at(i);
        if( agentsID.contains(id) ){
            CAgent *agnt = knowledge->getAgent(id);
            if( agnt->intention->M_type == "block" ){
                blockID = id;
            }
        }
    }

    if( blockID == -1 ){
        blockID = knowledge->newFastestSelector(playAgents);
        if( blockID != -1 ){
            CAgent *agnt = knowledge->getAgent(blockID);
            agnt->blockIntent.assign(blockID , knowledge->frameCount);
            agnt->intention = &agnt->blockIntent;
        }
    }

    if( blockID != -1 ){
        blockAgent = knowledge->getAgent(blockID);
    }
}


bool CMasterPlay::canOneTouch(QList<CAgent *> posAgents, CAgent *playMake){
    for( int i=0 ; i<posAgents.size() ; i++ ){
        QList<int> ourRelaxIDS,oppRelaxIDS;
        int positioner = posAgents.at(i)->self()->id;
        ourRelaxIDS.push_back(positioner);
        Vector2D pos = posAgents.at(i)->pos() + posAgents.at(i)->dir().setLengthVector(CRobot::center_from_kicker_new);
        if( getOpenness( pos, wm->field->oppGoalL(), wm->field->oppGoalR(), ourRelaxIDS, oppRelaxIDS ) > 0.8 )
            return true;
    }
    return false;
}

double CMasterPlay::getOpenness(Vector2D from, Vector2D p1, Vector2D p2, QList<int> ourRelaxedIDs, QList<int> oppRelaxedIDs)
{
    std::priority_queue < QPair< edgeMode , double > , vector< QPair< edgeMode , double > > , Comparar > blockedLines;
    double least = ( p1 - from ).th().degree();
    double most  = ( p2 - from ).th().degree();
    if( least > most )
    {
        swap(least,most);
    }

    for (int i=0;i<wm->our.activeAgentsCount();i++)
    {
        if (!ourRelaxedIDs.contains(wm->our.active(i)->id))
        {
            double obsDeg = ( wm->our.active(i)->pos - from ).th().degree();
            double obsDist = wm->our.active(i)->pos.dist(from);
            double obsAng = asin(wm->our.active(i)->robot_radius_new / obsDist) * _RAD2DEG;
            double lowerbound = obsDeg-obsAng;
            double upperbound = obsDeg+obsAng;

            if( lowerbound > most || upperbound < least )
                continue;

            if( upperbound > most )
                upperbound = most;

            if( lowerbound < least )
                lowerbound = least;

            blockedLines.push( qMakePair(TOP,upperbound) );
            blockedLines.push( qMakePair(BOT,lowerbound) );
        }
    }
    for (int i=0;i<wm->opp.activeAgentsCount();i++)
    {
        if (!oppRelaxedIDs.contains(wm->opp.active(i)->id))
        {
            double obsDeg = ( wm->opp.active(i)->pos - from ).th().degree();
            double obsDist = wm->opp.active(i)->pos.dist(from);
            double obsAng = asin(wm->opp.active(i)->robot_radius_new / obsDist) * _RAD2DEG;
            double lowerbound = obsDeg-obsAng;
            double upperbound = obsDeg+obsAng;

            if( lowerbound > most || upperbound < least )
                continue;

            if( upperbound > most )
                upperbound = most;

            if( lowerbound < least )
                lowerbound = least;

            blockedLines.push( qMakePair(TOP,upperbound) );
            blockedLines.push( qMakePair(BOT,lowerbound) );
        }
    }

    return 1.0 - coveredArea( blockedLines ) / ( most - least );
}

double CMasterPlay::coveredArea( std::priority_queue < QPair< edgeMode , double > , vector< QPair< edgeMode , double > > , Comparar >& obstacles )
{
    if( obstacles.size() <= 1 )
        return 0.0;

    QPair< edgeMode , double > lastest = obstacles.top();
    obstacles.pop();
    QPair< edgeMode , double > second_lastest = obstacles.top();

    if( lastest.first == TOP && second_lastest.first == TOP )
        return ( lastest.second - second_lastest.second ) + coveredArea( obstacles );

    else if( lastest.first == BOT && second_lastest.first == BOT )
        return ( lastest.second - second_lastest.second ) + coveredArea( obstacles );

    else if( lastest.first == TOP && second_lastest.first == BOT )
        return ( lastest.second - second_lastest.second ) + coveredArea( obstacles );

    else
        return coveredArea( obstacles );
}

void CMasterPlay::execute() {

    switch(agentsID.count()) {
    case 0:
        execute_0();
        break;
    case 1:
        execute_1();
        break;
    case 2:
        execute_2();
        break;
    case 3:
        execute_3();
        break;
    case 4:
        execute_4();
        break;
    case 5:
        execute_5();
        break;
    case 6:
        execute_6();
        break;
    default:
        debug(QString("MasterPlay agentsID invalid size: %1!").arg(agentsID.count()) , D_ERROR);
    }
    execPlay();
}

void CMasterPlay::execPlay(){

    if( blockAgent ){
        blockRole.parse();
        blockRole.assign(blockAgent);
        blockRole.setStop(true);
    }

    if( playMakeAgent ){
        if(knowledge-> getGameState() != CKnowledge::OurDirectKick && knowledge-> getGameState() != CKnowledge::OurIndirectKick  )
        {
            knowledge->setPlayMaker(playMakeAgent);
            playMakeRole.assign(playMakeAgent);
        }
        else
        {
            knowledge->setPlayMaker(playMakeAgent);

        }
    }


    if( markAgents.size() ){
        markPlan.init(markAgents);
        markPlan.execute();
    }

    if( knowledge->getGameState() != CKnowledge::Start )
    {
        if( positionAgents.size() ){
            ///////// added to prevent Segmentatino fault! //////////
            if( editData->count(formationName) == 0 ){
                debug(QString("Invalid Formation Name: %1").arg(formationName) , D_ERROR);
                formationName = "Stop6";
            }
            /////////////////////////////////////////////////////////
            position.init(positionAgents , (*editData)[formationName],formationName);
            if( staticInited ){
                position.staticInit(masterStaticPoints);
            }
            position.execute();
        }
    }
}
