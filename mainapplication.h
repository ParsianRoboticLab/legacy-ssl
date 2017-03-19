#ifndef CMAINAPPLICATION_H
#define CMAINAPPLICATION_H

#include <QMainWindow>
#include <QMenu>
#include <QWorkspace>
#include <QByteArray>
#include <QUdpSocket>
#include <QMenuBar>
#include <QTimer>
#include <QFile>
#include <QDataStream>
#include <QToolBar>
#include <QComboBox>

#include "net/netraw.h"

#include "worldmodel.h"
#include "agent.h"
#include "communicator.h"
#include "coach.h"
#include "ui/varswidget.h"
#include "ui/widgets.h"
#include "visionclient.h"
#include "motiontrainer.h"
#include "learning/policylearner.h"
#include <fstream>
#include <soccer.h>
#include "collectprofdata.h"

class MySoccer : public CSoccer
{
public:
    MySoccer();
    virtual void monitorUpdate();
    virtual void customControl(bool& custom);
};

/////////////////mhmmd monitor update thread

class monitorUpdateThread : public QThread
{
    Q_OBJECT
public:
    monitorUpdateThread(QObject *parent);
    void run();
    CStatusWidget *statusWidget;



    ////////////////from main app
    CStatusWidget *statusWidgetThread;
    MySoccer *monitorSoccer;
    CStatusPrinter *monitorPrinter;
    //////////////////
private:
    QTimer *timer;
signals:
    void update(bool _update);
public slots:
    void mainLoop();


};



class CMainApplication : public QMainWindow
{
    Q_OBJECT

public:
    CMainApplication(QWidget *parent = 0);
    ~CMainApplication();
    MySoccer* soccer;
    monitorUpdateThread *monitorUpdateThr;
public slots:       
    void run();
    void customControl(bool &custom);
    void logBtnPressed();
    void showHideMonitorWidget(bool show);
    void showHideTabWidget(bool show);
    void showHidePlotWidget(bool show);
    void showHidePlaysWidget(bool show);
    void setQuiescentMode(QAction *action);
    void updateActiveWidgetMenu(QWidget*);
    void updateMonitor();
    void stopStartCoach(bool start);
    void getMousePos(Vector2D _pos, int mouseClick);
    void changeTeamColor(VarType*);
    void changeTeamSide(VarType*);
    void reconnectReferee(VarType*);
    void reconnectSSLVision(VarType*);
    void reconnectSimulator(VarType*);
    void reconnectSimPlot(VarType*);
    void reconnectSerial(VarType*);
    void simPlotDataReady();
    void sampleTimeChanged();
    void setBallTrackerOnOffState(VarType*);
    void reconfigureAutomatedReferee(VarType*);
    void stopALL();
    void updateCoach();
    void refShortcutsChanged(bool val);
    void SetManualGS(int id);
    void setLoggerReplayMode(QAction*);
    void loggerWidgetRejected();
    void keyboardWidgetRejected();
    void joystickEnable();
    void trainStart();
    void trainEnd();
    void loadFormationConfigs();
signals:
    void wmUpdated();
protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
public:

    QQueue<CStatusText> *logger;
    CMotionTrainer* motionTrainer;
    QTimer *runTimer;
    QTimer loadFormationTimer;

    Vector2D mousePos;

    QUdpSocket* simPlotSocket;

    long int frameCnt;
    int monitorUpdateCnt;
    QTimer tempTimer;
    QTime *globalTime;
    // Log Viewer
    QTimer *logTimer;
    std::ifstream logData;
    float logVersion;
    //TEMP
    QFile *fData;
    QTextStream *sData;
    bool logging;
    QPushButton *logStart;


    bool alreadyReached;
    bool stopApplication;
    bool haveCoach;
    double visionFPS;
    double displayFPS;
    double commandFPS;
    int tempCnt;
    bool updatedEver;
    ////////////////////////////mouse click event
    bool mouseClickEvent;
    //////////////////////

    char* robComPort;
    CDrawer *drawerBuffer;
    CDrawer *replayBuffer;
    CStatusPrinter *printer;

    CTabDockWidget  * tabWidget;
//    CTabDockWidget  * rightTabWidget;

    CTrainWidget    * trainWidget;
    CMonitorWidget  * monitorWidget;
    CKeyboardWidget * keyboardWidget;
    CStatusWidget   * statusWidget;
    CSkillWidget    * skillWidget;
    CProfilerWidget * profilerWidget;
    CRoleWidget     * roleWidget;
    CPlotWidget     * plotWidget;
    CLoggerWidget   * loggerWidget;
    CPlayOffWidget  * playOffWidget;
    CSimulatorControllerWidget *gpWidget;
//    CPlaysWidget    * playsWidget;
//    CRectsWidget    * rectSlect;
//    CKnowledgeVarsWidget* knowledgeVarsWidget;
//    CConditionCheckerWidget *condCheckWidget;
//    CPolicyLearner* policyLearner;
//    CPlaysWidget *playsWidget2;




#ifdef DISTURBANCE_MATRIX
    CLearnEWidget* learnEWidget;
#endif
    //    CBehavioursWidget* behavioursWidget;

    QWorkspace *workspace;
    QMenu *viewMenu;
    QMenu *modeMenu;
    QMenu *coachMenu;
    QMenu *loggerMenu;
    QToolBar *toolbar;
    QLabel* statusbarLabel;
    QCheckBox *refShortcuts;
    QPushButton *btnRefs[14];
    QStringList strRefNames , strRefCommands;

    QTime displayTimer;
    QTime commandTimer;
    QTime loopFPSTimer;
    QTime networkDataFilterTimer;

    //Actions
    QAction *loadMonitorAct;
    QAction *loadTabsAct;
    QAction *loadPlotAct;
    QAction *loadPlaysWidgetAct;

    QAction *setSimModeAct;
    QAction *setSpyModeAct;
    QAction *setRealModeAct;
    QAction *setHCModeAct;
    QAction *setAIModeAct;
    QAction *setCoachModeAct;
    QAction *setExp1Act;
    QAction *setExp2Act;
    QAction *setExp3Act;
    QAction *setExp4Act;
    QAction *setExp5Act;
    QAction *setExp6Act;
    QAction *setJsHandy;
    QAction *setFProfiler;
    QAction *setKProfiler;
    QAction *setTechnicalChallengeAct;
    QAction *setMergeCamerasExperimentAct;
    QAction *setMixedAct;
    QAction *setLogMode;
    QAction *setReplayMode;
    QAction *setLogTagMode;

    //Game Related

    //Referee
    unsigned char lastCmdCnt;
    unsigned char cmdCnt;

    int experimental;
    void Experimental1();
    void Experimental2();
    void Experimental3();
    void Experimental4();
    void Experimental5();
    void Experimental6();
    void JsHandy();
    void kickProfiler();
    bool MergeCamerasExperiment();
    void TechnicalChallenge();

    double maxBallSpeed();


    void terminateLogOrReplay(bool);
};

void haltAllRobots();

extern QString startUpMode;
extern QString coachMode;





#endif // CMAINAPPLICATION_H
