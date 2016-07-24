#ifndef PROFILER_H
#define PROFILER_H
#define DATA_COUNT 21


#include <qjson/serializer.h>
#include <qjson/parser.h>
#include <qjson/qjson_export.h>
#include <qjson/qobjecthelper.h>
#include <qvariant.h>
#include <QFile>
#include <QIODevice>
//#include <skills.h>
//#include <QMap>

#include <QString>
#include <QList>
#include <QLinkedList>
//#include "agent.h"

/////////////////DEFINES
#define MIN_KICK_PROFILER 200
#define MAX_KICK_PROFILER 1023
#define STEP_KICK_PROFILER 25

#define MIN_CHIP_PROFILER 200
#define MAX_CHIP_PROFILER 1023
#define STEP_CHIP_PROFILER 25

#define MIN_PASS_PROFILER 200
#define MAX_PASS_PROFILER 1023
#define STEP_PASS_PROFILER 25

#define MIN_SHOOT_PROFILER 200
#define MAX_SHOOT_PROFILER 1023
#define STEP_SHOOT_PROFILER 25

#define MIN_ONETOUCH_PROFILER 200
#define MAX_ONETOUCH_PROFILER 1023
#define STEP_ONETOUCH_PROFILER 25

#define KICK_ARRAY_SIZE 30
#define CHIP_ARRAY_SIZE 30


struct CTimeProbe
{
    double time;
    int id;
    QString name;
};

class CProfiler
{
private:
    QLinkedList<CTimeProbe> probes;
    int id;
public:
    static double getTime();
    static long getKKTime();
    CProfiler();
    int putProbe(QString name);
    double elapesed();
    double takeProbe(int id);
    QString getProbeName(int i);
    int getProbeID(int i);
    double getProbeTime(int i);
    int probeCount();
};

#include <worldmodel.h>
#include <QTime>
#include <QFile>
#include <QTextStream>
class motionProfile
{
public :
    Vector2D position;
    Vector2D velocity;
    Vector2D direction;
    double forwardVel;
    double normalVel;
    double angularVel;

    double time;
    motionProfile(Vector2D _p, Vector2D _v, double _t, double _forwardVel, double _normalVel, Vector2D dir, double _angVel);
};

class CAgent;
class CMotionProfile
{
    int id;
    QTime profTimer;
    bool profTimerStarted;
    QFile *fMP;
    QTextStream *sMP;
    QList <motionProfile> mProfile;
    CAgent* agent;

public:
    CMotionProfile(CAgent* _agent);

    void init(int k);
    void store();
    void myStore(QList <double>);
};



//added by Mahi
enum SaveFormat {
    JSON,
    Binary
};

class CProfile {
public :
    CProfile();
    QMap<int, double> kickMap;
    QMap<int, double> chipMap;
    robotExtraDetail extraDetail;
    QPair<int,double> kickArr[KICK_ARRAY_SIZE];
    QPair<int,double> chipArr[CHIP_ARRAY_SIZE];

    int fillArray(QMap<int,double> _map,QPair<int,double> array[],bool kickMap = 1);
    void sortPairArrByValue(QPair<int,double> arr[], int left, int right);


    //    ~CProfile();
    int getKickSpeed(double _firstVelocity);
    int getChipSpeed(double _firstVelocity);

    void drawProfile();

    int getIntByEnum(robotExtraDetail _XtraDetail);
    robotExtraDetail getEnumByInt(int _XtraDetailIntID);
    void write(QVariantMap &profile);
    void read(QVariantMap &profile);
    void refresh();

private :
    //    QPair<int,double> kickDictionary[21];
    bool isUnnecessaryValueAvailable;
    int midPoint(int _first,int _last);
    int mahiBinarySearch(QPair<int, double> _Array[], double key, int iMin, int iMax);
    int keyOfUnnecessaryValues(QMap<int,double> _map,bool kickMap);
    int keyOfLessImportantValues(QMap<int,double> _map);
    void getTheMean(QMap<int, double> _map);
    bool isDeleted;



};

struct recordStrct{
    int robotId;
    ProfileMode mode;
    int speed;
};

struct executStrct{
    Vector2D target;
    //    CAgent kicker,receiver;
    int speed;
    bool chip;
};

class CDataStore {
public:
    void init(int _agentSize,ProfileMode _mode,bool _modify);
    void record();
    void decideRecord();
    void makeRecordExecutable();
    CDataStore();
    //    CDataStore(int _agentSize = 2,ProfileMode _mode = PKICK,bool _modify = true);
private:
    void execute_0();
    void execute_1();
    void execute_2();
    void execute_3();
    void execute_4();
    void execute_5();
    void execute_6();
    void execute_7();
    void execute_8();
    void execute_9();
    void execute_10();
    void execute_11();
    void execute_12();
    void execute_13();
    void execute_14();
    void execute_15();
    void execute();
    Property(int,AgentSize,agentSize);
    Property(ProfileMode,Mode,mode);
    Property(bool,Modify,modify);

    bool isValidValue(double _kickValue,ProfileMode _mode);
    bool isValidChipValue();

    int AgentsID[16] = {-1};

    QMap<int,double> kickMap;
    QMap<int,double> chipMap;

    bool isRecordNeeded(int _speed,QMap<int,double> _map,ProfileMode _mode);
    void fillRcrdStrct(int _roboID,ProfileMode _mode,int _speed,recordStrct &record);
    recordStrct execRecord;
    executStrct execInput;



};

struct SRecordArgs {
    ProfileMode mode;
    int inputSpeed;
    double RealSpeed;
    int robotId;
};
//added by mahi
class CNewProfiler : public QObject{

    Q_OBJECT

private:    
    bool isParsedOk;
    QVariantMap variantMapResualt;
    int kickerID,receiverID;
    bool isFirstTime;
    bool isKickDone;
    bool isRecordInserted;


public :

    CNewProfiler();
    CProfile robotsProfile[16];
    CDataStore dataStore;
    void read(QVariantMap &variantMap);
    void write(QVariantMap &dataBase);

    void insertRecord(ProfileMode mode,int inputSpeed,double RealSpeed, int robotId);

    void kick(ProfileMode _mode);
    void record(ProfileMode _mode);
    void receive(ProfileMode _mode);
    void chooseKicker(int _firstAgentID, int _secondAgentID);
    void mahiRecord(int _firstAgentID, int _secondAgentID,ProfileMode _mode);

    void profile(int _agentSize,ProfileMode _mode);

    QString getFile(const QString& _dir);
    void setFile(const QByteArray& _data, const QString& _dir);
    void refresh();


public slots:
    bool load(SaveFormat _loadFormat, const QString& _dir = "MahiProfiler.json");
    bool save(SaveFormat _saveFormat, const QString& _dir = "MahiProfiler.json");
    void insertRecord(SRecordArgs _args);
    //    ~CNewProfiler();
};


#endif // PROFILER_H
