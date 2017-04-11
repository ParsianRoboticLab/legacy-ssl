#pragma once
#include <QFile>
#include <QTime>
#include <stack>
#include <QMap>
#include "Object.h"
#include "SmartPointer.h"
#include <list>
#include <sys/time.h>
#include <sys/types.h>
#include <QString>
#include <QThread>
#include <QDataStream>
#include <QDebug>
#include <QMutex>
#include "profiler.h"
#include "drawer.h"


#define LOGGER_MAX_OBJECT_NUM 120
#define LOGGER_MAGIC    (qint32)0xBBAADDCC
#define LOGGER_VERSION  (qint32)0xAABBCCDD

#define LOGGER_DRAW_SEGMENT (qint32)0xAAAABBBB
#define LOGGER_DRAW_CIRCLE2D (qint32)0xBBBBCCCC
#define LOGGER_DRAW_TEXT (qint32)0xCCCCDDDD
#define LOGGER_DRAW_POINT (qint32)0xDDDDEEEE
#define LOGGER_DRAW_RECT (qint32)0xEEEEFFFF

#define STANDARD_LOG_REPLAY_TIME 0.016666667

#define SKILL_NAME_LEN 10
struct SRobotInfo{
    Vector2D pos , vel;
    double dir;
	qint8 id;
};

struct SInfoData{
    bool shootSens;
    bool spinLoad;
    bool canKick;
    bool canChip;
    qint8 shootBoard;
    qint8 Battery;
};

struct SPacketData{
    qint32 size;
    double time;
    quint8 refcom;
    quint8 refcomcnt , goals_blue , goals_yellow;
	quint8 game_mode, game_state;

    quint16 time_remaining;

    quint8 N , M , B;
    struct SRobotInfo our[LOGGER_MAX_OBJECT_NUM];
    struct SRobotInfo opp[LOGGER_MAX_OBJECT_NUM];
    struct SRobotInfo balls[LOGGER_MAX_OBJECT_NUM];
};

struct SDebugData{
    qint16 type;
    QString debug;
};
struct SAnalyzeData{
    QString key;
    QVariant value;
};

struct SDrawData{
    qint32 type , color;
    QString text;
    qint8 font , fill;
    qint16 startAng , endAng;
    Vector2D pos1 , pos2;
    double r;
    SDrawData(){
        type = NULL;
        text = "";
        startAng = endAng = 0;
        fill = false;
        pos1 = Vector2D(0,0);
        pos2 = Vector2D(0,0);
        color = Qt::black;
        r = 0;
    }
};

class CGameLogger;

//CreateSmartPointer(CGameLogger);

class CGameLogger: public QThread{
    QFile logFile , debugFile , drawFile , infoFile , analyzeFile;
    QFile readLog , readDebug , readDraw , readInfo , analyzeInfo;
    QDataStream logDS , debugDS , drawDS ,infoDS ;
    QTextStream AnalyzeDS;
    QDataStream readLogDS , readDebugDS , readDrawDS , readInfoDS;
    QList <SPacketData> packets;
    QList <SInfoData> infos;
    QMap <qint32 , QList<SDebugData> > debugs;
    QMap <QString , QList<QVariant> > analyses;
    QMap <qint32 , QList<SDrawData> > draws;
    QList <qint32> logSeekList , drawSeekList , debugSeekList;
    QList <SDrawData> drawList;
    bool fileLogError , fileReplayError;
    QTime timeToWrite;
    CProfiler profiler;
    CWorldModel *mywm;
    CDrawer *drawer;
    SDrawData localData;
    SDebugData debugData;
    double replayFPS;

    void resetLocalData();
    void addLocalDataToDraws();
    void addToDraws();
    SPacketData encodePacket();
    void writePackets();
    void writeDebugs();
    void writeAnalyses();
    void writeDraws();
    void writeInfo();
    void readLogPacket();
    void readDebugPacket();
    void readDrawPacket();
    void drawData();
public:
    CGameLogger();
    ~CGameLogger();
    void run();
    void addToDebugs(QString , qint16);
    void AddToAnalyze(QString,QVariant,bool);
    void addToDraws(Vector2D , QColor );
    void addToDraws(Segment2D , QColor);
    void addToDraws(Circle2D , int , int , QColor , bool );
    void addToDraws(QString , Vector2D , QColor , int );
    void addToDraws(Rect2D , QColor , bool);
    void closeLogFiles(bool force);
    void openFilesToLog(QString,QString);
    void closeReplayFile(bool force);
    void openFileToReplay(QString);
    void setDrawer(CDrawer *);
    void setReplayFPS(int fps=60);

    int seek;
    qint32 counter;
    double lastTime , continueToDrawTime;
	SPacketData lastLogData;
    int framenum;
	bool playPauseMode , replayMode , logMode , showDraws , showDebugs , closeLogger , loggerClosed , fBfMode;
    QList <SDebugData> debugList;

    Property(bool , IsLogMode , isLogMode);
    Property(bool , IsReplayMode , isReplayMode);
};

extern CGameLogger *gameLogger;
extern QMutex *loggerMutex;


inline std::ostream& operator << (std::ostream& stream, const QString & str)
{
    stream << ((char*) (str.toAscii().constData()));
    return stream;
}
