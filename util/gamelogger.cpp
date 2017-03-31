#include "gamelogger.h"
#include "worldmodel.h"
#include <string>


 CGameLogger *gameLogger;
QMutex *loggerMutex;

CGameLogger::CGameLogger(){
    mywm = new CWorldModel;
    playPauseMode = false;
    logMode = false;
    replayMode = false;
    fileLogError = true;
    fileReplayError = true;
    showDraws = true;
    showDebugs = true;
    replayFPS = STANDARD_LOG_REPLAY_TIME;
    closeLogger = true;
    isLogMode = false;
    isReplayMode = false;
	fBfMode = false;
	loggerClosed = true;
}

CGameLogger::~CGameLogger()
{
    closeLogFiles(false);
}

void CGameLogger::resetLocalData(){
    localData.type = NULL;
    localData.text = "";
    localData.startAng = localData.endAng = 0;
    localData.fill = false;
    localData.pos1 = Vector2D(0,0);
    localData.pos2 = Vector2D(0,0);
	localData.color = Qt::black;
    localData.r = 0;
}

void CGameLogger::setDrawer(CDrawer *_drawer ){
    drawer = _drawer;
}

void CGameLogger::openFilesToLog(QString baseFileName,QString totalDescription){

#ifndef Q_OS_MACX
    baseFileName = "logs/" + baseFileName;
#else
     baseFileName = "../../../logs/" + baseFileName;
#endif
    logFile.setFileName(baseFileName + ".log");
    debugFile.setFileName(baseFileName + ".debug");
    drawFile.setFileName(baseFileName + ".draw");
    infoFile.setFileName(baseFileName + ".info");
    analyzeFile.setFileName(baseFileName + ".analyze");


    if( !logFile.open(QIODevice::WriteOnly) || !debugFile.open(QIODevice::WriteOnly) || !drawFile.open(QIODevice::WriteOnly) || !infoFile.open(QIODevice::WriteOnly) || !analyzeFile.open(QIODevice::WriteOnly)){
        qDebug() << "Log: open log files failed";
		closeLogFiles(true);
        fileLogError = true;
    }
    else{
        qint32 descriptionSz;
        descriptionSz=totalDescription.toStdString().size();


        logDS.setDevice(&logFile);
        logDS << LOGGER_MAGIC;
        logDS << LOGGER_VERSION;
        logDS << descriptionSz;
        logDS.writeRawData(totalDescription.toStdString().c_str() , descriptionSz);

        debugDS.setDevice(&debugFile);
        debugDS << LOGGER_MAGIC;
        debugDS << LOGGER_VERSION;
        debugDS << descriptionSz;
        debugDS.writeRawData(totalDescription.toStdString().c_str() , descriptionSz);

        drawDS.setDevice(&drawFile);
        drawDS << LOGGER_MAGIC;
        drawDS << LOGGER_VERSION;
        drawDS << descriptionSz;
        drawDS.writeRawData(totalDescription.toStdString().c_str() , descriptionSz);

        infoDS.setDevice(&infoFile);
        infoDS << LOGGER_MAGIC;
        infoDS << LOGGER_VERSION;
        infoDS << descriptionSz;
        infoDS.writeRawData(totalDescription.toStdString().c_str() , descriptionSz);
        AnalyzeDS.setDevice(&analyzeFile);
        AnalyzeDS << "{";



        counter = 0;
        draws.clear();
        draws[counter] = QList<SDrawData>();
        debugs.clear();
        debugs[counter] = QList<SDebugData>();
        packets.clear();
        timeToWrite.start();
        continueToDrawTime = lastTime = profiler.getTime();
        fileLogError = false;
    }
}

void CGameLogger::closeLogFiles(bool force){
    if( !fileLogError || force ){
        fileLogError = true;
        packets.append(encodePacket());
        writePackets();
        writeDebugs();
        writeDraws();
        writeInfo();
        writeAnalyses();
        AnalyzeDS << "}";
        logFile.close();
        debugFile.close();
        drawFile.close();
        infoFile.close();
        analyzeFile.close();
    }
}
void CGameLogger::openFileToReplay(QString fileName){
    qint32 magic , version , magic2 , version2 , magic3 , version3 , magic4 , version4;
    QString totalDescription;
    qint32 sz, descriptionSz;
    char text[100000];
    std::string desc;
    int curPos;
    QString ss;
    QList <QString> qq;
    QList <qint32> qq32;
    closeReplayFile(false);
    readLog.setFileName(fileName);

    ss = QString(fileName.toStdString().substr(0 ,fileName.size()-4).c_str());
    ss += ".debug";
    readDebug.setFileName(ss);

    ss = QString(fileName.toStdString().substr(0 ,fileName.size()-4).c_str());
    ss += ".draw";
    readDraw.setFileName(ss);

    ss = QString(fileName.toStdString().substr(0 ,fileName.size()-4).c_str());
    ss += ".info";
    readInfo.setFileName(ss);

    if( !readLog.open(QIODevice::ReadOnly) || !readDebug.open(QIODevice::ReadOnly) || !readDraw.open(QIODevice::ReadOnly) || !readInfo.open(QIODevice::ReadOnly) ){
        qDebug() << "Replay: open Replay Files failed.";
        fileReplayError = true;
    }
    else{
        readLogDS.setDevice(&readLog);
        readDebugDS.setDevice(&readDebug);
        readDrawDS.setDevice(&readDraw);
        readInfoDS.setDevice(&readInfo);

        readLogDS >> magic >> version;
        readDebugDS >> magic2 >> version2;
        readDrawDS >> magic3 >> version3;



        readInfoDS >> magic4 >> version4;
        readInfoDS >> descriptionSz;
        readInfoDS.readRawData(text , descriptionSz);
        desc.assign(text , descriptionSz);
        totalDescription = totalDescription.fromStdString(desc);
//        qDebug()<<"___Info desc___"+totalDescription+"___"<<descriptionSz;
        readLogDS >> descriptionSz;
        readLogDS.readRawData(text , descriptionSz);
        desc.assign(text , descriptionSz);
        totalDescription = totalDescription.fromStdString(desc);
//        qDebug()<<"___Log desc___"+totalDescription+"___"<<descriptionSz;


        if( magic == LOGGER_MAGIC && version == LOGGER_VERSION &&
            magic2 == LOGGER_MAGIC && version2 == LOGGER_VERSION &&
            magic3 == LOGGER_MAGIC && version3 == LOGGER_VERSION &&
            magic4 == LOGGER_MAGIC && version4 == LOGGER_VERSION){

            curPos = 12+descriptionSz;
            logSeekList.clear();
            while( readLog.size() > curPos ){
                logSeekList.append(curPos);
                readLog.seek(curPos);
                readLogDS >> sz;
                if( sz == 0 ){
                    qDebug() << "Replay: log packet, sz is zero.";
                    break;
                }
                curPos += sz;
            }
            framenum = logSeekList.size();
			framenum = min(framenum , drawSeekList.size());
			framenum = min(framenum , debugSeekList.size());

            curPos = 12+descriptionSz;
            debugSeekList.clear();
            while( readDebug.size() > curPos ){
                debugSeekList.append(curPos);
                readDebug.seek(curPos);
                readDebugDS >> sz;
                if( sz == 0 ){
                    qDebug() << "Replay: debug packet, sz is zero.";
                    break;
                }
                curPos += sz;
            }

            curPos = 12+descriptionSz;
            drawSeekList.clear();
            while( readDraw.size() > curPos ){
                drawSeekList.append(curPos);
                readDraw.seek(curPos);
                readDrawDS >> sz;
                if( sz == 0 ){
                    qDebug() << "Replay: draw packet, sz is zero.";
                    break;
                }
                curPos += sz;
            }

            drawList.clear();
            debugList.clear();
            seek = 0;
            continueToDrawTime = lastTime = profiler.getTime();
            lastLogData.N = 0;
            lastLogData.M = 0;
            lastLogData.B = 0;
            fileReplayError = false;
            if( drawSeekList.size() != debugSeekList.size() || debugSeekList.size() != logSeekList.size() ){
                qDebug() << "Replay: the size of frames doesn't equal. --> " << drawSeekList.size() << "-" << debugSeekList.size() << "-" << logSeekList.size();

//                closeReplayFile(true);
//                fileReplayError = true;
            }
        }
        else{
            qDebug() << magic << " " << version << "----" << magic2 << " " << version2 << "-----" << magic3 << " " << version3;
            qDebug() << "Replay: Magic or Version or Both is Invalid.";
            closeReplayFile(true);
            fileReplayError = true;
        }
    }
}

void CGameLogger::closeReplayFile( bool force ){
    if( !fileReplayError || force ){
        fileReplayError = true;
        readLog.close();
        readDebug.close();
        readDraw.close();
        readInfo.close();
    }
}

void CGameLogger::writePackets(){
    for( int i=0 ; i<packets.size() ; i++ ){
		if( packets.at(i).size >= 15 && packets.at(i).size <= 1048576 ){
            logDS << packets.at(i).size;
            logDS << packets.at(i).refcom;
            logDS << packets.at(i).refcomcnt;
            logDS << packets.at(i).goals_blue;
            logDS << packets.at(i).goals_yellow;
            logDS << packets.at(i).time_remaining;
			logDS << packets.at(i).game_state;
			logDS << packets.at(i).game_mode;

            logDS << packets.at(i).N;
            for( int j=0 ; j<packets.at(i).N ; j++ ){
                logDS << packets.at(i).our[j].id;
                logDS << packets.at(i).our[j].pos.x;
                logDS << packets.at(i).our[j].pos.y;
                logDS << packets.at(i).our[j].dir;
                logDS << packets.at(i).our[j].vel.x;
                logDS << packets.at(i).our[j].vel.y;
            }

            logDS << packets.at(i).M;
            for( int j=0 ; j<packets.at(i).M ; j++ ){
                logDS << packets.at(i).opp[j].id;
                logDS << packets.at(i).opp[j].pos.x;
                logDS << packets.at(i).opp[j].pos.y;
                logDS << packets.at(i).opp[j].dir;
                logDS << packets.at(i).opp[j].vel.x;
                logDS << packets.at(i).opp[j].vel.y;
			}

            logDS << packets.at(i).B;
            for( int j=0 ; j<packets.at(i).B ; j++ ){
                logDS << packets.at(i).balls[j].pos.x;
                logDS << packets.at(i).balls[j].pos.y;
                logDS << packets.at(i).balls[j].vel.x;
                logDS << packets.at(i).balls[j].vel.y;
			}
        }
        else{
            qDebug() << "Log: packet size invalid.( " << packets.at(i).size << " )";
			logDS << qint16(15); // packets.at(i).size
            logDS << packets.at(i).refcom;
            logDS << packets.at(i).refcomcnt;
            logDS << packets.at(i).goals_blue;
            logDS << packets.at(i).goals_yellow;
            logDS << packets.at(i).time_remaining;
            logDS << quint8(0); // packets.at(i).N
            logDS << quint8(0); // packets.at(i).M
            logDS << quint8(0); // packets.at(i).B
        }
    }
    packets.clear();
}
void CGameLogger::writeAnalyses(){
    QVariant a;
    QMap <QString , QList<QVariant> >::iterator ite = analyses.begin();
    for( ite ; ite != analyses.end() ; ite++ ){
        AnalyzeDS<<'"'<<ite.key()<<'"'<<":";
        if(ite.value().size()>1){
            AnalyzeDS<<'[';
            for( int i=0 ; i<ite.value().size() ; i++ ){
                AnalyzeDS<<ite.value().at(i).toString();
                if(i!=ite.value().size()-1)
                    AnalyzeDS <<',';
            }
            AnalyzeDS<<']';
        }
        else
        {
            AnalyzeDS<<ite.value().at(0).toString();
        }
        if(ite != analyses.end()-1)
            AnalyzeDS <<',';

    }
}

void CGameLogger::writeDebugs(){
    qint32 siz , sz;
    QMap <qint32 , QList<SDebugData> >::iterator ite = debugs.begin();
    for( ite ; ite != debugs.end() ; ite++ ){
        sz = 4;
        for( int i=0 ; i<ite.value().size() ; i++ ){
            sz += 6 +ite.value().at(i).debug.toStdString().size();
        }
        if( sz >= 4 && sz <= 1048576 ){
            debugDS << sz;

            for( int i=0 ; i<ite.value().size() ; i++ ){
                debugDS << ite.value().at(i).type;
                siz = ite.value().at(i).debug.toStdString().size();
                debugDS << siz;
                debugDS.writeRawData(ite.value().at(i).debug.toStdString().c_str() , siz);
            }
        }
        else{
            qDebug() << "Log: Debug size invalid.( " << sz << " )";
            sz = 4;
            debugDS << sz;
        }
    }
    debugs.clear();
}



void CGameLogger::writeInfo(){



/*
    for( int i=0 ; i<infos.size() ; i++ ){

        infoDS << infos.at(i).canChip;
        infoDS << infos.at(i).canKick;
        infoDS << infos.at(i).spinLoad;
        infoDS << infos.at(i).shootSens;
        infoDS << infos.at(i).shootBoard;
    }
    infos.clear();
*/




}

void CGameLogger::writeDraws(){
    qint32 sz;
    qint32 siz;

    QMap <qint32 , QList<SDrawData> >::iterator ite = draws.begin();
    for( ite ; ite != draws.end() ; ite++ ){

        sz = 4; // size(4 bytes)

        for( int i=0 ; i<ite.value().size() ; i++ ){
            switch(ite.value().at(i).type){
            case LOGGER_DRAW_CIRCLE2D:
                sz += 37;
                break;
            case LOGGER_DRAW_POINT:
                sz += 24;
                break;
            case LOGGER_DRAW_RECT:
                sz += 41;
                break;
            case LOGGER_DRAW_SEGMENT:
                sz += 40;
                break;
            case LOGGER_DRAW_TEXT:
                sz += 29 + ite.value().at(i).text.toStdString().size();
                break;
            default:
                break;
            }
        }

        if( sz >= 4 && sz <= 1048576 ){
            drawDS << sz;

            for( int i=0 ; i<ite.value().size() ; i++ ){
                switch(ite.value().at(i).type){
                case LOGGER_DRAW_CIRCLE2D:
                    drawDS << ite.value().at(i).type;
                    drawDS << ite.value().at(i).pos1.x;
                    drawDS << ite.value().at(i).pos1.y;
                    drawDS << ite.value().at(i).r;
                    drawDS << ite.value().at(i).startAng;
                    drawDS << ite.value().at(i).endAng;
                    drawDS << ite.value().at(i).fill;
                    drawDS << ite.value().at(i).color;
                    break;
                case LOGGER_DRAW_POINT:
                    drawDS << ite.value().at(i).type;
                    drawDS << ite.value().at(i).pos1.x;
                    drawDS << ite.value().at(i).pos1.y;
                    drawDS << ite.value().at(i).color;
                    break;
                case LOGGER_DRAW_RECT:
                    drawDS << ite.value().at(i).type;
                    drawDS << ite.value().at(i).pos1.x;
                    drawDS << ite.value().at(i).pos1.y;
                    drawDS << ite.value().at(i).pos2.x;
                    drawDS << ite.value().at(i).pos2.y;
                    drawDS << ite.value().at(i).fill;
                    drawDS << ite.value().at(i).color;
                    break;
                case LOGGER_DRAW_SEGMENT:
                    drawDS << ite.value().at(i).type;
                    drawDS << ite.value().at(i).pos1.x;
                    drawDS << ite.value().at(i).pos1.y;
                    drawDS << ite.value().at(i).pos2.x;
                    drawDS << ite.value().at(i).pos2.y;
                    drawDS << ite.value().at(i).color;
                    break;
                case LOGGER_DRAW_TEXT:
                    drawDS << ite.value().at(i).type;
                    siz = ite.value().at(i).text.toStdString().size();
                    drawDS << siz;
                    drawDS.writeRawData(ite.value().at(i).text.toStdString().c_str() , siz);
                    drawDS << ite.value().at(i).pos1.x;
                    drawDS << ite.value().at(i).pos1.y;
                    drawDS << ite.value().at(i).font;
                    drawDS << ite.value().at(i).color;
                    break;
                default:
                    break;
                }
            }
        }
        else{
            qDebug() << "Log: Draw size invalid.( " << sz << " )";
            sz = 4;
            drawDS << sz;
        }
    }
    draws.clear();
}

void CGameLogger::readLogPacket(){
    readLogDS >> lastLogData.size;
    readLogDS >> lastLogData.refcom;
    readLogDS >> lastLogData.refcomcnt;
    readLogDS >> lastLogData.goals_blue;
    readLogDS >> lastLogData.goals_yellow;
    readLogDS >> lastLogData.time_remaining;
	readLogDS >> lastLogData.game_state;
	readLogDS >> lastLogData.game_mode;

    readLogDS >> lastLogData.N;
    for( int j=0 ; j<lastLogData.N ; j++ ){
        readLogDS >> lastLogData.our[j].id;
        readLogDS >> lastLogData.our[j].pos.x;
        readLogDS >> lastLogData.our[j].pos.y;
        readLogDS >> lastLogData.our[j].dir;
        readLogDS >> lastLogData.our[j].vel.x;
        readLogDS >> lastLogData.our[j].vel.y;
    }

    readLogDS >> lastLogData.M;
    for( int j=0 ; j<lastLogData.M ; j++ ){
        readLogDS >> lastLogData.opp[j].id;
        readLogDS >> lastLogData.opp[j].pos.x;
        readLogDS >> lastLogData.opp[j].pos.y;
        readLogDS >> lastLogData.opp[j].dir;
        readLogDS >> lastLogData.opp[j].vel.x;
        readLogDS >> lastLogData.opp[j].vel.y;
	}

    readLogDS >> lastLogData.B;
    for( int j=0 ; j<lastLogData.B ; j++ ){
        readLogDS >> lastLogData.balls[j].pos.x;
        readLogDS >> lastLogData.balls[j].pos.y;
        readLogDS >> lastLogData.balls[j].vel.x;
        readLogDS >> lastLogData.balls[j].vel.y;
	}
}

void CGameLogger::readDrawPacket(){
    qint32 curPos;
    qint32 siz , sz;
    char text[10240];
    drawList.clear();
    SDrawData data;
    std::string ss;
    curPos = readDraw.pos();

    readDrawDS >> sz;
    if( sz > 0 ){
        while( readDraw.pos() < curPos + sz ){

            readDrawDS >> data.type;

            switch( data.type){
            case LOGGER_DRAW_RECT:
                readDrawDS >> data.pos1.x;
                readDrawDS >> data.pos1.y;
                readDrawDS >> data.pos2.x;
                readDrawDS >> data.pos2.y;
                readDrawDS >> data.fill;
                readDrawDS >> data.color;

                drawList.append(data);

                break;
            case LOGGER_DRAW_CIRCLE2D:
                readDrawDS >> data.pos1.x;
                readDrawDS >> data.pos1.y;
                readDrawDS >> data.r;
                readDrawDS >> data.startAng;
                readDrawDS >> data.endAng;
                readDrawDS >> data.fill;
                readDrawDS >> data.color;

                drawList.append(data);
                break;
            case LOGGER_DRAW_POINT:
                readDrawDS >> data.pos1.x;
                readDrawDS >> data.pos1.y;
                readDrawDS >> data.color;

                drawList.append(data);

                break;
            case LOGGER_DRAW_SEGMENT:
                readDrawDS >> data.pos1.x;
                readDrawDS >> data.pos1.y;
                readDrawDS >> data.pos2.x;
                readDrawDS >> data.pos2.y;
                readDrawDS >> data.color;

                drawList.append(data);
                break;
            case LOGGER_DRAW_TEXT:
                readDrawDS >> siz;
                readDrawDS.readRawData(text , siz);
                ss.assign(text , siz);
                data.text = data.text.fromStdString(ss);
                readDrawDS >> data.pos1.x;
                readDrawDS >> data.pos1.y;
                readDrawDS >> data.font;
                readDrawDS >> data.color;

                drawList.append(data);
                break;
            default:
                break;
            }
        }
    }
}

void CGameLogger::readDebugPacket(){
    qint32 curPos;
    qint32 siz , sz;
    char text[100000];
    debugList.clear();
    SDebugData data;
    std::string ss;
    curPos = readDebug.pos();

    readDebugDS >> sz;
    if( sz > 0 ){
        while( readDebug.pos() < curPos + sz ){
            readDebugDS >> data.type;
            readDebugDS >> siz;
            readDebugDS.readRawData(text , siz);
            ss.assign(text , siz);
            data.debug = data.debug.fromStdString(ss);
            debugList.append(data);
        }
    }
}

SPacketData CGameLogger::encodePacket(){
    SPacketData data;

    halfworldMutex->lock();

    mywm->update(halfworld);
    int ballcount = halfworld->ball.count();
    for( int i=0 ; i<ballcount ; i++ ){
        data.balls[i].pos.x = halfworld->ball.at(i)->pos.x;
        data.balls[i].pos.y = halfworld->ball.at(i)->pos.y;
        data.balls[i].vel.x = halfworld->ball.at(i)->vel.x;
        data.balls[i].vel.y = halfworld->ball.at(i)->vel.y;
    }

    data.refcom = halfworld->gsp.cmd;
    data.refcomcnt = halfworld->gsp.cmd_counter;
    data.goals_blue = halfworld->gsp.goals_blue;
    data.goals_yellow = halfworld->gsp.goals_yellow;
    data.time_remaining = halfworld->gsp.time_remaining;
	data.game_mode = halfworld->game_mode;
	data.game_state = halfworld->game_state;

    halfworldMutex->unlock();

    data.N = (qint8)mywm->our.activeAgentsCount() ,
    data.M = (qint8)mywm->opp.activeAgentsCount() ,
    data.B = (qint8)ballcount;

	data.size = (41)*(data.N + data.M) + 32*data.B  + 15;

    for( int i=0 ; i<data.N ; i++ ){
		data.our[i].id = (qint8) mywm->our.active(i)->id;
		data.our[i].pos.x = mywm->our.active(i)->pos.x;
		data.our[i].pos.y = mywm->our.active(i)->pos.y;
		data.our[i].dir = mywm->our.active(i)->dir.dir().radian();
		data.our[i].vel.x = mywm->our.active(i)->vel.x;
		data.our[i].vel.y = mywm->our.active(i)->vel.y;
    }

    for( int i=0 ; i<data.M ; i++ ){
        data.opp[i].id = (qint8) mywm->opp.active(i)->id;
        data.opp[i].pos.x =  mywm->opp.active(i)->pos.x;
        data.opp[i].pos.y = mywm->opp.active(i)->pos.y;
        data.opp[i].dir = mywm->opp.active(i)->dir.dir().radian();
        data.opp[i].vel.x = mywm->opp.active(i)->vel.x;
        data.opp[i].vel.y = mywm->opp.active(i)->vel.y;
    }
    return data;
}

void CGameLogger::drawData(){

	QString states[] = {
						"Halt",
						"Stop",
						"OurKickOff",
						"TheirKickOff",
						"OurDirectKick",
						"TheirDirectKick",
						"OurIndirectKick",
						"TheirIndirectKick",
						"OurPenaltyKick",
						"TheirPenaltyKick",
						"Start",
                                                "NormalStart",
                                                "OurBallPlacement",
                                                "TheirBallPlacement"};

    for( int j=0 ; j<lastLogData.N ; j++ )
	{
        drawer->drawRobot(lastLogData.our[j].pos , Vector2D(cos(lastLogData.our[j].dir),sin(lastLogData.our[j].dir)) , wm->getTeamQColor() , lastLogData.our[j].id  , -1 , "" , true);
	}
    for( int j=0 ; j<lastLogData.M ; j++ )
	{
        drawer->drawRobot(lastLogData.opp[j].pos , Vector2D(cos(lastLogData.opp[j].dir),sin(lastLogData.opp[j].dir)) , wm->getOppQColor() , lastLogData.opp[j].id , -1 , "" , true);
	}

    for( int j=0 ; j<lastLogData.B ; j++ ){
        if( j )
		{
            drawer->draw(Circle2D(lastLogData.balls[j].pos , 0.03) , 0 , 360 , QColor("black") , true);			
		}
        else
		{
            drawer->draw(Circle2D(lastLogData.balls[j].pos , 0.03) , 0 , 360 , QColor("orange") , true);
		}
    }
	drawer->draw(QString("GS: ") + states[lastLogData.game_state] , Vector2D(2.5, 2.55), QColor("red") , 15);
	drawer->draw(QString("GM: ") + states[lastLogData.game_mode], Vector2D(2.5, 2.35), QColor("blue") , 15);


    QColor color;
    SDrawData dataToDraw;
    if( showDraws ){
        for( int i=0 ; i<drawList.size() ; i++ ){
            dataToDraw = drawList.at(i);
            color.setRgb(dataToDraw.color/1000000 , (dataToDraw.color/1000) % 1000 , dataToDraw.color % 1000 );
            switch( dataToDraw.type){
            case LOGGER_DRAW_CIRCLE2D:
                drawer->draw(Circle2D(dataToDraw.pos1 , dataToDraw.r) , dataToDraw.startAng , dataToDraw.endAng , color , dataToDraw.fill);
                break;
            case LOGGER_DRAW_POINT:
                drawer->draw(dataToDraw.pos1 , 1 , color);
                break;
            case LOGGER_DRAW_RECT:
                drawer->draw(Rect2D(dataToDraw.pos1 , dataToDraw.pos2) , color , dataToDraw.fill);
                break;
            case LOGGER_DRAW_SEGMENT:
                drawer->draw(Segment2D(dataToDraw.pos1 , dataToDraw.pos2) , color);
                break;
            case LOGGER_DRAW_TEXT:
                drawer->draw(dataToDraw.text , dataToDraw.pos1 , color , dataToDraw.font);
                break;
            default:
                break;
            }
        }
    }
}
void CGameLogger::AddToAnalyze(QString key,QVariant value , bool SaveSameValues){

#ifdef GAME_MODE
    return;
#endif
    loggerMutex->lock();
    if( gameLogger->getIsLogMode() ){
//        AnalyzeDS.writeRawData(key.toStdString().c_str() , key.size());


        if(analyses.contains(key))
            if(analyses[key].contains(value) && !SaveSameValues);
            else
                analyses[key].append(value);
        else
        {
            QList<QVariant> emptyValueList;
            emptyValueList.append(value);
            analyses.insert(key,emptyValueList);
        }
        //AnalyzeDS <<"{"<<key<<":"<<value.toString()<<"},";
    }
    loggerMutex->unlock();
}

void CGameLogger::addToDebugs( QString packet , qint16 type){
    debugData.type = type;
    debugData.debug = packet;

    debugs[counter].append(debugData);
}

void CGameLogger::addToDraws(Vector2D pos , QColor color){

    resetLocalData();

    localData.type = LOGGER_DRAW_POINT;
    localData.pos1 = pos;
    localData.color = 1000000*color.red() + 1000*color.green() + color.blue();

    addLocalDataToDraws();
}

void CGameLogger::addToDraws(Segment2D seg , QColor color ){

    resetLocalData();

    localData.type = LOGGER_DRAW_SEGMENT;
    localData.pos1 = seg.a();
    localData.pos2 = seg.b();
    localData.color = 1000000*color.red() + 1000*color.green() + color.blue();

    addLocalDataToDraws();
}

void CGameLogger::addToDraws(Circle2D circ , int startAng , int endAng , QColor color , bool filled ){

    resetLocalData();

    localData.type = LOGGER_DRAW_CIRCLE2D;
    localData.r = circ.radius();
    localData.pos1 = circ.center();
    localData.startAng = startAng;
    localData.endAng = endAng;
    localData.color = 1000000*color.red() + 1000*color.green() + color.blue();
    localData.fill = filled;

    addLocalDataToDraws();
}

void CGameLogger::addToDraws(QString text , Vector2D pos , QColor color , int size ){

    resetLocalData();

    localData.type = LOGGER_DRAW_TEXT;
    localData.text = text;
    localData.pos1 = pos;
    localData.color = 1000000*color.red() + 1000*color.green() + color.blue();
    localData.font = size;

    addLocalDataToDraws();
}

void CGameLogger::addToDraws(Rect2D rect , QColor color , bool filled ){

    resetLocalData();

    localData.type = LOGGER_DRAW_RECT;
    localData.pos1 = rect.topLeft();
    localData.pos2 = rect.bottomRight();
    localData.color = 1000000*color.red() + 1000*color.green() + color.blue();
    localData.fill = filled;

    addLocalDataToDraws();
}

void CGameLogger::addLocalDataToDraws(){
    draws[counter].append(localData);
}

void CGameLogger::setReplayFPS(int fps ){
    if( fps >= 1 && fps <= 600 )
        replayFPS = 1.00/fps;
}

void CGameLogger::run(){
    QList<SDrawData> emptyQListSDraw;
    QList<SDebugData> emptyQListSDebug;
	loggerMutex->lock();
	loggerClosed = false;
	loggerMutex->unlock();

	bool closeFlag;

    while( true ){

		loggerMutex->lock();
		closeFlag = closeLogger;
		loggerMutex->unlock();

		if( closeFlag )
			break;

        if( logMode ){
            if( !fileLogError ){
                if( timeToWrite.elapsed() < 1000 ){
                    if( profiler.getTime() - lastTime >= STANDARD_LOG_REPLAY_TIME ){
                        loggerMutex->lock();
                        packets.append(encodePacket());
                        ++counter;
                        draws[counter] = emptyQListSDraw;
                        debugs[counter] = emptyQListSDebug;
                        lastTime = profiler.getTime();
                        loggerMutex->unlock();
                    }
                }
                else{
                    loggerMutex->lock();
                    timeToWrite.restart();
                    packets.append(encodePacket());
                    writePackets();
                    writeDebugs();
                    writeDraws();
                    ++counter;
                    draws[counter] = emptyQListSDraw;
                    debugs[counter] = emptyQListSDebug;
                    loggerMutex->unlock();
                }
            }
        }
        else if( replayMode ){
			if( !fileReplayError && seek < framenum && seek >= 0 ){
				if( fBfMode ){
					loggerMutex->lock();
					if( seek < logSeekList.size() ){
						readLog.seek(logSeekList.at(seek));
						readLogPacket();
					}
					if( showDraws ){
						if( seek < drawSeekList.size() ){
							readDraw.seek(drawSeekList.at(seek));
							readDrawPacket();
						}
					}
					if( showDebugs ){
						if( seek < debugSeekList.size() ){
							readDebug.seek(debugSeekList.at(seek));
							readDebugPacket();
						}
					}
					continueToDrawTime = lastTime = profiler.getTime();
					drawData();
					loggerMutex->unlock();
				}
				else{
					if( playPauseMode ){
						if( profiler.getTime() - lastTime >= replayFPS ){
							loggerMutex->lock();
							if( seek < logSeekList.size() ){
								readLog.seek(logSeekList.at(seek));
								readLogPacket();
							}
							if( showDraws ){
								if( seek < drawSeekList.size() ){
									readDraw.seek(drawSeekList.at(seek));
									readDrawPacket();
								}
							}
							if( showDebugs ){
								if( seek < debugSeekList.size() ){
									readDebug.seek(debugSeekList.at(seek));
									readDebugPacket();
								}
							}
							seek++;
							continueToDrawTime = lastTime = profiler.getTime();
							drawData();
							loggerMutex->unlock();
						}
						else if( profiler.getTime() - continueToDrawTime >= STANDARD_LOG_REPLAY_TIME ){
							loggerMutex->lock();
							drawData();
							continueToDrawTime = profiler.getTime();
							loggerMutex->unlock();
						}
					}
					else if( profiler.getTime() - continueToDrawTime >= STANDARD_LOG_REPLAY_TIME){
						loggerMutex->lock();
						if( seek < logSeekList.size() ){
							readLog.seek(logSeekList.at(seek));
							readLogPacket();
						}
						if( showDraws ){
							if( seek < drawSeekList.size() ){
								readDraw.seek(drawSeekList.at(seek));
								readDrawPacket();
							}
						}
						if( showDebugs ){
							if( seek < debugSeekList.size() ){
								readDebug.seek(debugSeekList.at(seek));
								readDebugPacket();
							}
						}
						drawData();
						continueToDrawTime = profiler.getTime();
						loggerMutex->unlock();
					}
				}
			}
        }
        msleep(1);
    }
	loggerMutex->lock();
	loggerClosed = true;
	loggerMutex->unlock();
	qDebug() << "Logger thread ended.";
}
