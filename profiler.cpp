#include "profiler.h"
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <agent.h>
#include <mathtools.h>

CProfiler::CProfiler()
{
    id = 1;
}

double CProfiler::getTime()
{
    timeval tim;
    gettimeofday(&tim, NULL);
    return tim.tv_sec+(tim.tv_usec/1000000.0);
}

long CProfiler::getKKTime()
{
    timeval tim;
    gettimeofday(&tim, NULL);
    return tim.tv_sec*100+tim.tv_usec/10000;
}

int CProfiler::putProbe(QString name)
{
    CTimeProbe t;
    t.time = getTime();
    t.id = id;
    t.name = name;
    probes.append(t);

    id ++;
    return id-1;
}

double CProfiler::elapesed()
{
    return getTime() - probes.last().time;
}

double CProfiler::takeProbe(int id)
{    
    for (QLinkedList<CTimeProbe>::iterator i=probes.begin();i!=probes.end();i++)
    {
        if ((*i).id==id)
        {
            CTimeProbe t = *i;
            probes.erase(i);
            t.time = getTime() - t.time;
            return t.time;
        }
    }
    return -1;
}

QString CProfiler::getProbeName(int i)
{
    if (i<probes.count() && i>=0)
    {
        int k = 0;
        for (QLinkedList<CTimeProbe>::iterator j=probes.begin();j!=probes.end();j++)
        {
            if (k==i) return (*j).name;
            k++;
        }
    }
    else return QString("Wrong Probe");
}

int CProfiler::getProbeID(int i)
{
    if (i==-1) return probes.last().time;
    if (i<probes.count() && i>=0)
    {
        int k = 0;
        for (QLinkedList<CTimeProbe>::iterator j=probes.begin();j!=probes.end();j++)
        {
            if (k==i) return (*j).id;
            k++;
        }
    }
    else return -1;
}

double CProfiler::getProbeTime(int i)
{
    if (i==-1) return probes.last().time;
    if (i<probes.count() && i>=0)
    {
        int k = 0;
        for (QLinkedList<CTimeProbe>::iterator j=probes.begin();j!=probes.end();j++)
        {
            if (k==i) return (*j).time;
            k++;
        }
    }
    else return -1;
}

int CProfiler::probeCount()
{
    return probes.count();
}

CMotionProfile::CMotionProfile(CAgent* _agent):
    agent(_agent)
{
}

void CMotionProfile::init(int k)
{
    id = k;
    mProfile.clear();
    profTimer.restart();
    profTimerStarted = false;
    fMP = new QFile(QString("./motion/MotionProfile-%1:%2:%3.CSV").arg(QTime::currentTime().hour()).arg(QTime::currentTime().minute()).arg(QTime::currentTime().second()));
    if (fMP->open(QFile::WriteOnly | QFile::Truncate)) {
        sMP = new QTextStream(fMP);
    }else{
        sMP = new QTextStream();
    }
    (*sMP)<<"TimeFromStart,PositionX,PositionY,Angle,VelocityX,VelocityY,W,ForwardVel,NormalVel\n";
}

void CMotionProfile::store()
{
    if(!profTimerStarted)
    {
        profTimer.start();
        profTimerStarted = true;
    }

    mProfile.append( motionProfile( wm->our[id]->pos, wm->our[id]->vel, profTimer.elapsed(), agent->vforward, agent->vnormal, wm->our[id]->dir, agent->angularVel()));
    //    bool dPath = conf()->MotionProfile_drawpath();
    //    bool dVel = conf()->MotionProfile_drawvel();
    //    bool dDir = conf()->MotionProfile_drawdir();
    //    bool drw = dPath || dVel || dDir;
    //    if ( drw){
    //        for ( int i = 0; i < mProfile.count(); i++){
    //            if ( dPath)
    //                draw(Circle2D( mProfile.at(i).position, 0.02), 0, 360, QColor("lightBlue"), 1);
    //            if ( dVel)
    //                draw(Segment2D(mProfile.at(i).position, mProfile.at(i).position+mProfile.at(i).velocity), "red");
    //        }
    //    }
    //	mProfile.last().velocity.rotate(-1*mProfile.last().position.th().degree());
    if ( !mProfile.isEmpty())
        (*sMP)<< mProfile.last().time <<","<< mProfile.last().position.x <<","<< mProfile.last().position.y <<"," << mProfile.last().direction.dir().degree() << ","<< mProfile.last().velocity.x << "," << mProfile.last().velocity.y << ","<< mProfile.last().angularVel << "," <<mProfile.last().forwardVel << "," << mProfile.last().normalVel <<"\n";
    sMP->flush();
}

void CMotionProfile::myStore(QList <double> list){
    if(!profTimerStarted)
    {
        profTimer.start();
        profTimerStarted = true;
    }

    if( list.size() ){
        (*sMP) << profTimer.elapsed() << ",";
        for( int i=0 ; i<list.size() ; i++ )
            (*sMP) << list.at(i) << ",";
        (*sMP) << "\n";
    }

    sMP->flush();
}
/////////////////////ADDED BY MAHI ;D
CNewProfiler::CNewProfiler() {
    refresh();
}

void CNewProfiler::refresh() {
    for(size_t i = 0; i < 16;i++) {
        robotsProfile[i].refresh();
    }
}

bool CNewProfiler::save(SaveFormat _saveFormat, const QString& _dir) {
    QFile saveFile(_saveFormat == JSON
                   ? _dir
                   : QString("mahiProfiler.dat"));
    if(!saveFile.open(QIODevice::WriteOnly)) {
        debug("Couldn't Open save file.", D_MAHI);
        return false;
    }

    QVariantMap tempDataBase;
    write(tempDataBase);
    QJson::Serializer serializer;
    serializer.setIndentMode(QJson::IndentFull);
    QByteArray output = serializer.serialize(tempDataBase,&isParsedOk);

    saveFile.write(output);
    return true;
}

bool CNewProfiler::load(SaveFormat _loadFormat, const QString& _dir) {
    QFile loadFile(_loadFormat == JSON
                   ? _dir
                   : QString("MahiProfiler.dat"));

    if (!loadFile.open(QIODevice::ReadOnly)) {
        debug(QString("Couldn't open Profiler file."),D_MAHI);
        return false;
    }

    QByteArray input = loadFile.readAll();
    QJson::Parser parser;
    QVariantMap dataBase = parser.parse(input,&isParsedOk).toMap();
    read(dataBase);


}

QString CNewProfiler::getFile(const QString& _dir) {
    QFile loadFile(_dir);

    if(!loadFile.open(QIODevice::ReadOnly)) {
        debug(QString("Couldn't open Profiler file."),D_MAHI);
        return QString();
    }

    return loadFile.readAll();
}

void CNewProfiler::setFile(const QByteArray &_data, const QString &_dir) {
    QFile saveFile(_dir);

    if(!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
    }

    saveFile.write(_data);
    saveFile.close();
}

void CNewProfiler::read(QVariantMap &variantMap) {
    QVariantList tempVariantList;
    QVariantMap tempRobot;
    tempVariantList = variantMap.value(QString("PROFILER")).toList();

    for(int i = 0;i < tempVariantList.size();i++) {
        tempRobot = tempVariantList.at(i).toMap();
        tempRobot = tempRobot.value(QString("robot_%1").arg(i)).toMap();
        robotsProfile[i].read(tempRobot);
    }

}


void CNewProfiler::write(QVariantMap &dataBase) {

    QVariantMap tempRobotProfile,tempRobot;
    QVariantList tempVariantList;
    for(int i=0;i < 16;i++) {
        robotsProfile[i].write(tempRobotProfile);
        tempRobot.insert(QString("robot_%1").arg(i), tempRobotProfile);
        tempVariantList.append(tempRobot);
        tempRobot.clear();
        tempRobotProfile.clear();
    }
    dataBase.insert("PROFILER",tempVariantList);
}


void CNewProfiler::insertRecord(ProfileMode mode, int inputSpeed, double RealSpeed,int robotId) {
    if(mode == PCHIP) {
        robotsProfile[robotId].chipMap.insert(inputSpeed, RealSpeed);
    }
    else if(mode == PKICK) {
        robotsProfile[robotId].finalKickMap.insert(inputSpeed, RealSpeed);
    }
    else if(mode == SCHIP){
        robotsProfile[robotId].SpinChipMap.insert(inputSpeed, RealSpeed);
    }
    else if(mode == SKICK){
        robotsProfile[robotId].finalSpinKickMap.insert(inputSpeed, RealSpeed);
    }
}

void CNewProfiler::insertRecord(ProfileMode mode, int inputSpeed, QList<double> RealSpeed,int robotId) {
    if(mode == PKICK) {
        robotsProfile[robotId].kickMap.insert(inputSpeed , RealSpeed);
    }
    else if(mode == SKICK){
        robotsProfile[robotId].SpinKickMap.insert(inputSpeed , RealSpeed);
    }
}

void CNewProfiler::insertRecord(SRecordArgs _args) {
    insertRecord(_args.mode, _args.inputSpeed, _args.RealSpeed, _args.robotId);
}

////////////////////////////////////
////////////////RECORD//////////////
////////////////////////////////////
void CNewProfiler::mahiRecord(int _firstAgentID, int _secondAgentID, ProfileMode _mode) {

    if(isFirstTime) {
        chooseKicker(_firstAgentID,_secondAgentID);
        isFirstTime = false;
    }
    else {

        switch(_mode) {
        case PCHIP:
            kick   (PCHIP);
            record (PCHIP);
            receive(PCHIP);
            break;
        case PPASS:
        case PKICK:
        case PONETOUCH:
        case PSHOOT:
            kick   (PKICK);
            record (PKICK);
            receive(PKICK);
            break;

        }
    }
}

void CNewProfiler::kick(ProfileMode _mode) {
    if(_mode == PCHIP) {

    }
    else {

    }
}

void CNewProfiler::receive(ProfileMode _mode) {
    if(_mode == PCHIP) {

    }
    else {

    }
}

void CNewProfiler::record(ProfileMode _mode) {
    if(_mode == PCHIP) {

    }
    else {

    }
}

void CNewProfiler::chooseKicker(int _firstAgentID, int _secondAgentID) {
    Vector2D tempBallPos = wm->ball->pos;
    if(tempBallPos.dist(wm->our.active(_firstAgentID)->pos) <
            tempBallPos.dist(wm->our.active(_secondAgentID)->pos)) {
        kickerID = _firstAgentID;
        receiverID = _secondAgentID;
    }
    kickerID   = _secondAgentID;
    receiverID = _firstAgentID;
}


/////////////////////////////////////////////////
///////////////DATASTORE BY MAHI/////////////////
/////////////////////////////////////////////////
CDataStore::CDataStore() {
    execRecord.robotId = -1;
}

//CDataStore::CDataStore(int _agentSize, ProfileMode _mode, bool _modify) {
//    agentSize = _agentSize;
//    mode = _mode;
//    modify = _modify;
//}

void CDataStore::init(int _agentSize, ProfileMode _mode, bool _modify) {
    agentSize = _agentSize;
    mode = _mode;
    modify = _modify;
}

void CDataStore::decideRecord() {
    int RecordAgentsID[16] = {-1};
    for(int i = 0;i< wm->our.activeAgentsCount();i++) {
        RecordAgentsID[i]  = wm->our.activeAgentID(i);
    }
    if(wm->our.activeAgentsCount() < agentSize) return;
    for(int i = 0;i<16;i++)
        AgentsID[i] = RecordAgentsID[i];
    for(int i = 0;i < agentSize;i++) {
        switch(mode) {
        case PKICK:
            for(int j = MIN_KICK_PROFILER; j < MAX_KICK_PROFILER;j += STEP_KICK_PROFILER) {
                if(isRecordNeeded(j,finalKickMap,PKICK)) {
                    fillRcrdStrct(i,PKICK,j,execRecord);
                    return;
                }
            }
            break;
        case PCHIP:
            for(int j = MIN_CHIP_PROFILER; j < MAX_CHIP_PROFILER;j += STEP_CHIP_PROFILER) {
                if(isRecordNeeded(j,chipMap,PCHIP)) {
                    fillRcrdStrct(i,PCHIP,j,execRecord);
                    return;
                }
            }
            break;
        case PSHOOT:
            for(int j = MIN_SHOOT_PROFILER; j < MAX_SHOOT_PROFILER;j += STEP_SHOOT_PROFILER) {
                if(isRecordNeeded(j,finalKickMap,PSHOOT)) {
                    fillRcrdStrct(i,PSHOOT,j,execRecord);
                    return;
                }
            }
            break;
        case PPASS:
            for(int j = MIN_PASS_PROFILER; j < MAX_PASS_PROFILER;j += STEP_PASS_PROFILER) {
                if(isRecordNeeded(j,finalKickMap,PPASS)) {
                    fillRcrdStrct(i,PPASS,j,execRecord);
                    return;
                }
            }
            break;
        case PONETOUCH:
            for(int j = MIN_ONETOUCH_PROFILER; j < MAX_ONETOUCH_PROFILER;j += STEP_ONETOUCH_PROFILER) {
                if(isRecordNeeded(j,finalKickMap,PONETOUCH)) {
                    fillRcrdStrct(i,PONETOUCH,j,execRecord);
                    return;
                }
            }
            break;
        }
    }
    debug(QString("RECORD IS COMPLETED"),D_MAHI);


}

void CDataStore::makeRecordExecutable() {
    if(execRecord.robotId < 0) return;
    //    execInput.kicker = wm->our.active(execRecord.robotId);
    execInput.chip = false;
    execInput.speed = execRecord.speed;
    //    execInput.receiver = wm->our.active(AgentsID[(execRecord.robotId == AgentsID[0]) ? 1 : 0]);
    switch(execRecord.mode) {
    case PKICK:
        //        execInput.target();
        break;
    case PSHOOT:
        break;
    case PCHIP:
        execInput.chip = false;
        break;
    case PPASS:
        break;
    case PONETOUCH:
        break;
    }
}


bool CDataStore::isRecordNeeded(int _speed, QMap<int, double> _map, ProfileMode _mode) {
    return (((!_map.contains(_speed-2)) ||
             (_map.contains(_speed-2) && (!isValidValue(_speed-2,_mode)))) &&
            ((!_map.contains(_speed-1)) ||
             (_map.contains(_speed-1) && (!isValidValue(_speed-1,_mode)))) &&
            ((!_map.contains(_speed)) ||
             (_map.contains(_speed) && (!isValidValue(_speed,_mode)))) &&
            ((!_map.contains(_speed+1)) ||
             (_map.contains(_speed+1) && (!isValidValue(_speed+1,_mode)))) &&
            ((!_map.contains(_speed+2)) ||
             (_map.contains(_speed+2) && (!isValidValue(_speed+2,_mode)))));
}

bool CDataStore::isValidValue(double _kickValue,ProfileMode _mode) {
    switch(_mode) {
    case PKICK:
        if(_kickValue > 0 && _kickValue < 12) return true;
        else return false;
        break;
    case PSHOOT:
        break;
    case PCHIP:
        break;
    case PPASS:
        break;
    case PONETOUCH:
        break;
    }

}

void CDataStore::fillRcrdStrct(int _roboID,ProfileMode _mode,int _speed,recordStrct &record) {
    record.robotId = _roboID;
    record.mode = _mode;
    record.speed = _speed;
}


void CDataStore::execute() {
    int EagentSize = agentSize;
    switch(EagentSize) {
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
    case 7:
        execute_7();
        break;
    case 8:
        execute_8();
        break;
    case 9:
        execute_9();
        break;
    case 10:
        execute_10();
        break;
    case 11:
        execute_11();
        break;
    case 12:
        execute_12();
        break;
    case 13:
        execute_13();
        break;
    case 14:
        execute_14();
        break;
    case 15:
        execute_15();
        break;
    default:
        execute_0();
    }

}

void CDataStore::execute_0() {

}

void CDataStore::execute_1() {

}

void CDataStore::execute_2() {

}

void CDataStore::execute_3() {

}

void CDataStore::execute_4() {

}

void CDataStore::execute_5() {

}

void CDataStore::execute_6() {

}

void CDataStore::execute_7() {

}

void CDataStore::execute_8() {

}

void CDataStore::execute_9() {

}

void CDataStore::execute_10() {

}

void CDataStore::execute_11() {

}

void CDataStore::execute_12() {

}

void CDataStore::execute_13() {

}

void CDataStore::execute_14() {

}

void CDataStore::execute_15() {

}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////ADDED BY MAHI :D
CProfile::CProfile() {
    isDeleted = false;
    isUnnecessaryValueAvailable = true;
    refresh();
}

void CProfile::refresh() {
    fillArray(finalKickMap, kickArr, true);
    sortPairArrByValue(kickArr,0,KICK_ARRAY_SIZE);
}

int CProfile::fillArray(QMap<int, double> _map, QPair<int, double> array[],bool kickMap) {
    QMap<int, double> tempMap = _map;

    isDeleted = false;
    if(kickMap) {
        if(tempMap.size() <= KICK_ARRAY_SIZE) {
            QMap<int, double>::const_iterator i = tempMap.constBegin();
            int j = 1;
            while(i != tempMap.constEnd()) {
                array[j].first  = i.key();
                array[j].second = i.value();
                j++;
                ++i;
            }
            return j;
        }
        else {
            if(isUnnecessaryValueAvailable) {
                tempMap.remove(keyOfUnnecessaryValues(tempMap, kickMap));
            }
            if(isDeleted) {
                fillArray(tempMap, array, kickMap);
            }
            else {
                isUnnecessaryValueAvailable = false;
                tempMap.remove(keyOfLessImportantValues(tempMap));
                if(isDeleted) fillArray(tempMap,array,kickMap);
                else debug("FILLING ARRAY HAVE A PROBLEM",D_MAHI);
            }
            return KICK_ARRAY_SIZE;
        }

    }
    else {
        if(tempMap.size() <= CHIP_ARRAY_SIZE) {
            QMap<int, double>::const_iterator i = tempMap.constBegin();
            int j = 0;
            while(i != tempMap.constEnd()) {
                array[j].first  = i.key();
                array[j].second = i.value();
                j++;
                ++i;
            }
            return j;
        }
        else {
            if(isUnnecessaryValueAvailable) {
                tempMap.remove(keyOfUnnecessaryValues(tempMap,kickMap));
            }
            if(isDeleted) fillArray(tempMap,array,kickMap);
            else {
                isUnnecessaryValueAvailable = false;
                tempMap.remove(keyOfLessImportantValues(tempMap));
                if(isDeleted) {
                    fillArray(tempMap,array,kickMap);
                }
                else {
                    debug("FILLING ARRAY HAVE A PROBLEM",D_MAHI);
                }
            }
            return CHIP_ARRAY_SIZE;
        }
    }
}
int CProfile::keyOfUnnecessaryValues(QMap<int, double> _map, bool kickMap) {
    isDeleted = false;
    QMap<int, double>::const_iterator i = _map.constBegin();
    double minmax;
    int final = -1;
    if(kickMap) minmax = 8;
    else minmax = 10;
    while(i != _map.constEnd()) {
        if(i.value() > minmax) {
            minmax = i.value();
            final = i.key();
        }
        ++i;
    }
    if(final != -1) {
        isDeleted = true;
        return final;
    }
    if(kickMap) minmax = 1;
    else minmax = 0;
    i = _map.constBegin();
    while(i != _map.constEnd()) {
        if(i.value() < minmax) {
            minmax = i.value();
            final  = i.key();
        }
        ++i;
    }
    if(final != -1) {
        isDeleted = true;
        return final;
    }
    isDeleted = false;
}

int CProfile::keyOfLessImportantValues(QMap<int, double> _map) {
    isDeleted = false;
    QMap<int, double>::const_iterator i = _map.constBegin();
    double min = 100000,tempImportantness;
    int final = -1;
    while(i != _map.constEnd()) {
        tempImportantness = ((i+1).value() - (i  ).value())/((i+1).key() - (i  ).key()) -
                ((i  ).value() - (i-1).value())/((i  ).key() - (i-1).key()) ;
        if(tempImportantness < min) {
            min = tempImportantness;
            final = i.key();
        }
        ++i;
    }
    if(final != -1) {
        isDeleted = true;
        return final;
    }
}

void CProfile::getTheMean(QMap<int, double> _map) {
    QMap<int, double>::const_iterator i = _map.constBegin();

    //    while(i != _map)
}

int CProfile::getKickSpeed(double _firstVelocity) {
    //    fillArray(kickMap, kickArr, true);
    //    sortPairArrByValue(kickArr,0,KICK_ARRAY_SIZE);
    return mahiBinarySearch(kickArr,_firstVelocity,0,KICK_ARRAY_SIZE);
    //return 0;
}

int CProfile::mahiBinarySearch(QPair<int,double> _Array[], double key, int iMin, int iMax) {
    int resualt;
    if (iMax < iMin) {
        resualt = ((_Array[iMin].first - _Array[iMax].first)/(_Array[iMin].second - _Array[iMax].second)) \
                *(key - _Array[iMax].second) + _Array[iMax].first;
        return resualt;

    }
    else {
        int imid = midPoint(iMin, iMax);

        if (_Array[imid].second > key) return mahiBinarySearch(_Array, key, iMin, imid - 1);
        else if (_Array[imid].second < key) return mahiBinarySearch(_Array, key, imid + 1, iMax);
        else {
            return _Array[imid].first;
        }
    }
}

int CProfile::getChipSpeed(double _firstVelocity) {
    //return mahiBinarySearch(chipDictionary,_firstVelocity,0,20);
    return 1;
}

int CProfile::midPoint(int _first, int _last) {
    return ceil((_last + _first)/2);
}

/////////////////////////////////////////////JSON
void CProfile::write(QVariantMap &profile) {
    QVariantList tempVariantList, innerList;
    QVariantMap tempVariantMap;
    QVariantMap keyValueMap;
    QMap<int,double>::const_iterator i ;
    QMap<int , QList<double> >::const_iterator ki ;

    ////////////////insert KICK
    ki = kickMap.constBegin();
    while(ki != kickMap.constEnd()) {
        tempVariantMap.clear();
        innerList.clear();
        keyValueMap.clear();
        keyValueMap.insert(QString("kickSpeed"),ki.key());
        innerList.clear();
        for(int i=0; i<ki.value().constEnd()-ki.value().constBegin(); i++)
            innerList.append(ki.value().at(i));
        keyValueMap.insert(QString("realSpeed"),innerList);
        tempVariantList.append(keyValueMap);
        ++ki;
    }
    profile.insert(QString("kick"),tempVariantList);
    tempVariantList.clear();
    ////////////////insert SPINKICK
    ki = SpinKickMap.constBegin();
    while(ki != SpinKickMap.constEnd()) {
        tempVariantMap.clear();
        innerList.clear();
        keyValueMap.clear();
        keyValueMap.insert(QString("SpinKickSpeed"),ki.key());

        for(int i=0; i<ki.value().constEnd()-ki.value().constBegin(); i++)
            innerList.append(ki.value().at(i));
        keyValueMap.insert(QString("SpinRealSpeed"),innerList);
        tempVariantList.append(keyValueMap);
        ++ki;
    }
    profile.insert(QString("SpinKick"),tempVariantList);
    tempVariantList.clear();

    ///////////////insert CHIP
    i = chipMap.constBegin();
    while(i != chipMap.constEnd()) {
        tempVariantMap.clear();
        tempVariantMap.insert(QString("chipSpeed"),i.key());
        tempVariantMap.insert(QString("realDistance"),i.value());
        tempVariantList.append(tempVariantMap);
        ++i;
    }
    profile.insert(QString("chip"),tempVariantList);
    tempVariantList.clear();

    ///////////////insert SPINCHIP
    i = SpinChipMap.constBegin();
    while(i != SpinChipMap.constEnd()) {
        tempVariantMap.clear();
        tempVariantMap.insert(QString("SpinChipSpeed"),i.key());
        tempVariantMap.insert(QString("SpinRealDistance"),i.value());
        tempVariantList.append(tempVariantMap);
        ++i;
    }
    profile.insert(QString("SpinChip"),tempVariantList);
    tempVariantList.clear();

    //////////////////insert extraDetail
    //    profile.insert(QString("extraDetail"),getIntByEnum(extraDetail));
    tempVariantMap.clear();
    tempVariantMap.insert(QString("kick") , ExtraDetail.value("kick"));
    tempVariantMap.insert(QString("chip") , ExtraDetail.value("chip"));
    tempVariantMap.insert(QString("SpinKick") , ExtraDetail.value("SpinKick"));
    tempVariantMap.insert(QString("SpinChip") , ExtraDetail.value("SpinChip"));
    profile.insert(QString("extraDetail") , tempVariantMap);
    tempVariantMap.clear();

    //////////////////insert extra data
    //    profile.insert(QString("extraDetail"),getIntByEnum(extraDetail));
}

void CProfile::read(QVariantMap &profile) {
    QVariantList tempVariantList;
    QList<double> innerList;
    QVariantMap tempVariantMap;

    //////////////////////////////KICK
    tempVariantList.clear();
    tempVariantList = profile.value("kick").toList();
    kickMap.clear();
    innerList.clear();

    foreach(QVariant data, tempVariantList){
        for(int i=0; i<data.toMap().value(QString("realSpeed")).toList().count(); i++)
            innerList.append(data.toMap().value(QString("realSpeed")).toList().at(i).toDouble());
        kickMap.insert(data.toMap().value(QString("kickSpeed")).toInt(), innerList);
        innerList.clear();
    }

    for(int i=0; i<kickMap.count(); i++)
        finalKickMap.insert(kickMap.keys().at(i), AvgWithoutOutliers(kickMap.values().at(i) , 0.9));
    //////////////////////////////SPINKICK
    tempVariantList.clear();
    tempVariantList = profile.value("SpinKick").toList();
    SpinKickMap.clear();
    innerList.clear();

    foreach(QVariant data, tempVariantList){
        for(int i=0; i<data.toMap().value(QString("SpinRealSpeed")).toList().count(); i++)
            innerList.append(data.toMap().value(QString("SpinRealSpeed")).toList().at(i).toDouble());
        SpinKickMap.insert(data.toMap().value(QString("SpinKickSpeed")).toInt(), innerList);
        innerList.clear();
    }

    for(int i=0; i<SpinKickMap.count(); i++)
        finalSpinKickMap.insert(SpinKickMap.keys().at(i), AvgWithoutOutliers(SpinKickMap.values().at(i) , 0.9));
    //////////////////////////////EXTRADETAIL
    extraDetail = getEnumByInt(profile.value("extraDetail").toInt());

    tempVariantMap = profile.value("extraDetail").toMap();
    ExtraDetail.clear();

    ExtraDetail.insert(QString("SpinChip") , tempVariantMap.value(QString("SpinChip")).toString());
    ExtraDetail.insert(QString("SpinKick") , tempVariantMap.value(QString("SpinKick")).toString());
    ExtraDetail.insert(QString("chip") , tempVariantMap.value(QString("chip")).toString());
    ExtraDetail.insert(QString("kick") , tempVariantMap.value(QString("kick")).toString());

    ////////////////////CHIP
    tempVariantList.clear();
    tempVariantList = profile.value("chip").toList();
    chipMap.clear();
    foreach(QVariant data, tempVariantList)
        chipMap.insert(data.toMap().value(QString("chipSpeed")).toInt(),
                       data.toMap().value(QString("realDistance")).toDouble());

    ////////////////////SPINCHIP
    SpinChipMap.clear();
    tempVariantList.clear();
    tempVariantList = profile.value("SpinChip").toList();
    foreach(QVariant data, tempVariantList)
        SpinChipMap.insert(data.toMap().value(QString ("SpinChipSpeed")).toInt(),
                           data.toMap().value(QString("SpinRealDistance")).toDouble());

}


void CProfile::sortPairArrByValue(QPair<int, double> arr[],int left,int right) {
    int i = left, j = right;
    double tmp;
    double pivot = arr[(left + right) / 2].second;

    /* partition */
    while (i <= j) {
        while (arr[i].second < pivot) i++;
        while (arr[j].second > pivot) j--;
        if (i <= j) {
            tmp = arr[i].second;
            arr[i].second = arr[j].second;
            arr[j].second = tmp;
            i++;
            j--;
        }
    };

    /* recursion */
    if (left < j)  sortPairArrByValue(arr, left, j);
    if (i < right) sortPairArrByValue(arr, i, right);

    //    if(arr[0].first)
}


void CProfile::drawProfile() {
    Polygon2D tPolygon;
    QMap<int,double>::const_iterator i = finalKickMap.constBegin();
    while (i != finalKickMap.constEnd()) {
        tPolygon.addVertex(Vector2D(i.key()/100,i.value())/3.2);
        ++i;
    }
    draw(tPolygon);
}


int CProfile::getIntByEnum(robotExtraDetail _XtraDetail) {
    switch (_XtraDetail) {
    case MNOTHING:
        return 0;
    case MGOALKIPPER:
        return 1;
    case MDEFENCE:
        return 2;
    case MOFFENCE:
        return 3;
    case MPASSER:
        return 4;
    case MSHOOTER:
        return 5;
    case MONETOUCHER:
        return 6;
    case MRECIVER:
        return 7;
    case MWELLMOTION:
        return 8;
    default : return 0;
    }
}

robotExtraDetail CProfile::getEnumByInt(int _XtraDetailIntID) {
    switch (_XtraDetailIntID) {
    case 0:
    default:
        return MNOTHING;
    case 1:
        return MGOALKIPPER;
    case 2:
        return MDEFENCE;
    case 3:
        return MOFFENCE;
    case 4:
        return MPASSER;
    case 5:
        return MSHOOTER;
    case 6:
        return MONETOUCHER;
    case 7:
        return MRECIVER;
    case 8:
        return MWELLMOTION;
    }
}
