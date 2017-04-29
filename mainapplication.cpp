#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QAction>
#include <QThread>
#include <QTime>
#include <QPainter>
#include <QStatusBar>
#include <QMessageBox>

#include "mainapplication.h"

#include "net/netraw.h"

#include "geom.h"
#include <gamelogger.h>
#include <widgets.h>

#include <math.h>
#include <time.h>
#include <QFileDialog>

#include "experimental1.h"
#include "experimental2.h"
#include "experimental3.h"
#include "experimental4.h"
#include "experimental5.h"
#include "experimental6.h"
#include "technicalchallenge.h"
#include "mergeCamerasExperiment.h"
#include "joystickhandy.h"
#include "kickprofiler.h"
#include "autoballplacement.h"


QString startUpMode;
QString coachMode;

CMainApplication* mainapp;

CMainApplication::CMainApplication(QWidget *parent)
    : QMainWindow(parent, 0)
{

    srand(QTime(0,0,0).secsTo(QTime::currentTime()));

    updatedEver=false;
    experimental=0;

    //profiler
    collectKickProfile=new CollectProfileData();

    /* Init Workspace */

    statusbarLabel = new QLabel("", this);
    statusBar()->addWidget(statusbarLabel);

    refShortcuts = new QCheckBox("Ref Shortcuts", this);
    refShortcuts->setCheckState( Qt::Unchecked );
    //knowledge->refShortcuts = true;
    connect( refShortcuts,  SIGNAL( toggled(bool)), this, SLOT(refShortcutsChanged(bool)));

    statusBar()->addWidget( refShortcuts );


    //mini referee
    QButtonGroup *btnRefGroup = new QButtonGroup();
    strRefNames << "H" << "FS" << "S" << "NS" << "FK" << "IK" << "KO" << "PK" << "BP" << "FK" << "IK" << "KO" << "PK" << "BP";
    strRefCommands << "H" << "s" << "S" << " " << "F" << "I" << "K" << "P" << "B" << "f" << "i" << "k" << "p" << "b";
    for(int i=0 ; i<14 ; i++ )
    {
        btnRefs[i] = new QPushButton(strRefNames[i],this);
        QString strType = "n";
        if( i >=4 )
            strType = "b";
        if( i >=9 )
            strType = "y";
        btnRefs[i]->setProperty("refType" , strType);
        btnRefGroup->addButton(btnRefs[i] , i);
    }
    for( int i=0 ; i<14 ; i++ )
    {
        statusBar()->addWidget(btnRefs[i]);
    }
    double widgetWidth = 300;
    QString strWidth = QString("%1px").arg((int) (widgetWidth / 12));
    //StyleSheet
    QString styleSheet =
            QString("\
                    QPushButton[refType=\"n\"] {background-color:gray; color:black; padding: 0; border: 1px solid black; border-radius: 5px; width: %1} \
            QPushButton[refType=\"b\"] {background-color:blue; color:white; padding: 0; border: 1px solid black; border-radius: 5px; width: %2} \
            QPushButton[refType=\"y\"] {background-color:yellow; color:black; padding: 0; border: 1px solid black; border-radius: 5px; width: %3} \
            QPushButton[refType=\"n\"]:hover {border: 2px solid red} \
            QPushButton[refType=\"b\"]:hover {border: 2px solid red} \
            QPushButton[refType=\"y\"]:hover {border: 2px solid red} \
            ").arg(strWidth).arg(strWidth).arg(strWidth);

            statusBar()->setStyleSheet(styleSheet);
            connect(btnRefGroup , SIGNAL(buttonClicked(int)) , this , SLOT(SetManualGS(int)));
    //

    initVars(new CVarsWidget());
    initPolicy(new CPolicyWidget());

    srand(time(0));

    globalTime = new QTime();

    /* Objects */

    drawerBuffer = new CDrawer();
    replayBuffer = new CDrawer();

    printer = new CStatusPrinter();
    logger = new QQueue<CStatusText>;
    initLogger((void*)printer);
    initDrawer((void*)drawerBuffer);


    soccer = new MySoccer();

    simPlotSocket = NULL;
    reconnectSimPlot(NULL);
    mouseClickEvent = false;
    /* Policy Learner */
    //policyLearner = new CPolicyLearner(soccer->coach);



    //TEMP: loging text file
    //    fData = new QFile("DATA.CSV");
    //    if (fData->open(QFile::WriteOnly | QFile::Truncate)) {
    //        sData = new QTextStream(fData);
    //    }else{
    //        sData = new QTextStream();
    //    }
    //    (*sData)<<"timestamp"<<","<<"vision pos x"<<","<<"vision pos y"<<","<<"kamlan x"<<","<<"kalman y"<<","<<"kalman vel x"<<","<<"kalman vel y"<<"\n";
    //    logging = false;
    //    logStart = new QPushButton("Start Logging",0);
    //    connect(logStart,SIGNAL(clicked()),this,SLOT(logBtnPressed()));
    //

    //sData->setRealNumberPrecision(15);
    haveCoach = false;
    displayFPS = 0;
    commandFPS = 0;
    tempCnt = 0;


    /* Timers */
    logTimer = new QTimer(this);
    logTimer->setInterval(1000/60);
    logTimer->stop();

    networkDataFilterTimer.start();

    /* Widgets */
    workspace = new QWorkspace(this);
    setCentralWidget(workspace);

    trainWidget = new CTrainWidget();
    workspace->addWindow(trainWidget, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint );
    trainWidget->show();


    monitorWidget = new CMonitorWidget(drawerBuffer, this);
    monitorWidget->setWindowTitle(tr("Monitor"));
    workspace->addWindow(monitorWidget, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    monitorWidget->setWindowState(Qt::WindowMaximized );

    tabWidget = new CTabDockWidget(0, true);
    addDockWidget(Qt::LeftDockWidgetArea, tabWidget);

//    rightTabWidget = new CTabDockWidget(0 , false);
//    addDockWidget(Qt::RightDockWidgetArea,rightTabWidget);

    //TEMP
    keyboardWidget = new CKeyboardWidget(soccer->agents, this);
    keyboardWidget->setWindowTitle(tr("Handy Control"));
    keyboardWidget->setFocusPolicy(Qt::ClickFocus);
    //	workspace->addWindow(keyboardWidget, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);

    loggerWidget = new CLoggerWidget();
    loggerWidget->setWindowTitle("Log Player");
    loggerWidget->setFocusPolicy(Qt::ClickFocus);

    loggerMutex->lock();
    gameLogger->setDrawer(replayBuffer);
    loggerMutex->unlock();

    plotWidget = new CPlotWidget();
    plotWidget->setWindowTitle(tr("Plot Widget"));
    knowledge->plotWidgetCustom = plotWidget->custom;

    statusWidget = new CStatusWidget(printer , soccer->agents , soccer->coach);
    addDockWidget(Qt::BottomDockWidgetArea, statusWidget);
    printer->textBuffer.enqueue(CStatusText("Initializing..."));

    statusWidget->info->setSenders(&(soccer->refSender),&(soccer->visSender));
    statusWidget->info->setWorldModel(wm);

    skillWidget = new CSkillWidget(soccer->agents, 0);
    skillWidget->setWindowTitle(tr("Skills"));

    /////////////////MAHI :D
    profilerWidget = new CProfilerWidget(soccer->agents, 0);
    profilerWidget->setWindowTitle("Profiler");


    playOffWidget = new CPlayOffWidget(soccer->coach->getPlanLoader(), 0);
    playOffWidget->setWindowTitle("Play Off Plans");
    //////////////////


    roleWidget = new CRoleWidget(soccer->agents, 0);
    roleWidget->setWindowTitle(tr("Roles"));

    gpWidget = new CSimulatorControllerWidget(wm);
    gpWidget->setWindowTitle("Simulator Controller");
    monitorWidget->simulatorControlWidget = gpWidget;

    //	condCheckWidget = new CConditionCheckerWidget();
    //	condCheckWidget->setWindowTitle("Condition Checker");

    //	playsWidget = new CPlaysWidget();
    //	playsWidget->setWindowTitle("Plays");
    //	playsWidget->coach = soccer->coach;
    //	playsWidget->initialize();

    //	playsWidget2 = new CPlaysWidget(true);
    //	playsWidget2->resize(800, 800);
    //	playsWidget2->setWindowTitle("Plays");
    //	playsWidget2->coach = soccer->coach;
    //	playsWidget2->initialize();

    //	behavioursWidget = new CBehavioursWidget();

    //	knowledgeVarsWidget = new CKnowledgeVarsWidget();

#ifdef DISTURBANCE_MATRIX
    learnEWidget = new CLearnEWidget(soccer->agents);
    learnEWidget->setWindowTitle("Learn E");
#endif

    //	rectSlect = new CRectsWidget(monitorWidget);
    //	rectSlect->setWindowTitle("Rects");

    changeTeamColor(NULL);
    changeTeamSide(NULL);

    workspace->setActiveWindow(monitorWidget);

    /* Actions */
    loadMonitorAct = new QAction(tr("Monitor"), this);
    loadMonitorAct->setCheckable(true);
    loadMonitorAct->setChecked(true);

    loadTabsAct = new QAction(tr("Tabs"), this);
    loadTabsAct->setCheckable(true);
    loadTabsAct->setChecked(true);

    loadPlotAct = new QAction(tr("Plotter"), this);
    loadPlotAct->setCheckable(true);
    loadPlotAct->setChecked(false);

    loadPlaysWidgetAct = new QAction(tr("Plays"), this);
    loadPlaysWidgetAct ->setCheckable(true);
    loadPlaysWidgetAct ->setChecked(false);

    setSimModeAct = new QAction(tr("Simulation"), this);
    setSimModeAct->setCheckable(true);
    setSimModeAct->setChecked(true);
    setSimModeAct->setShortcut(QKeySequence(tr("Ctrl+S" , "Log")));

    //setSimModeAct->setShortcut(soccer->joystick->getButton1());


    setRealModeAct = new QAction(tr("Real"), this);
    setRealModeAct->setCheckable(true);
    setRealModeAct->setChecked(false);
    setRealModeAct->setShortcut(QKeySequence(tr("Ctrl+A" , "Log")));

    setSpyModeAct = new QAction(tr("Spy") , this);
    setSpyModeAct->setCheckable(true);
    setSpyModeAct->setChecked(false);

    setMixedAct = new QAction(tr("Mixed Team Mode"), this);
    setMixedAct->setCheckable(true);
    setMixedAct->setChecked(false);//(wm->mixedMode == true));

    setExp1Act = new QAction("Experimental1", this);
    setExp1Act->setCheckable(true);
    setExp1Act->setChecked((experimental==1));

    setExp2Act = new QAction("Experimental2", this);
    setExp2Act->setCheckable(true);
    setExp2Act->setChecked((experimental==2));

    setExp3Act = new QAction("Experimental3", this);
    setExp3Act->setCheckable(true);
    setExp3Act->setChecked((experimental==3));

    setExp4Act = new QAction("Experimental4", this);
    setExp4Act->setCheckable(true);
    setExp4Act->setChecked((experimental==4));

    setExp5Act = new QAction("Experimental5", this);
    setExp5Act->setCheckable(true);
    setExp5Act->setChecked((experimental==5));

    setExp6Act = new QAction("Experimental6", this);
    setExp6Act->setCheckable(true);
    setExp6Act->setChecked((experimental==6));

    setFProfiler = new QAction("CollectProfileData", this);
    setFProfiler->setCheckable(true);
    setFProfiler->setChecked((experimental==11));
    ///////////////////////////////////////////////////////////////profiler
    setKProfiler = new QAction("Kick Profiler", this);
    setKProfiler->setCheckable(true);
    setKProfiler->setChecked((experimental==10));
    setKProfiler->setShortcut(QKeySequence(tr("Ctrl+P","Kick Profiler")));

    /////////////////////////////////////////////////joy stick handy
    setJsHandy = new QAction("JoyStick Handy Controller", this);
    setJsHandy->setCheckable(true);
    setJsHandy->setChecked((experimental==9));
    setJsHandy->setShortcut(QKeySequence(tr("Ctrl+J","JoyStick Handy Controller")));

    /////////////////////////////////////////////////auto ball placement
    setAutoBallPlacement = new QAction("Auto Ball Placement", this);
    setAutoBallPlacement->setCheckable(true);
    setAutoBallPlacement->setChecked((experimental==12));

    ////////////////////////////////////////////////
    setTechnicalChallengeAct = new QAction("TechnicalChallenge", this);
    setTechnicalChallengeAct->setCheckable(true);
    setTechnicalChallengeAct->setChecked((experimental==7));

    setMergeCamerasExperimentAct = new QAction("MergeCamerasExperiment",this);
    setMergeCamerasExperimentAct->setCheckable(true);
    setMergeCamerasExperimentAct->setChecked((experimental==8));

    setHCModeAct = new QAction(tr("Handy Control"), this);
    setHCModeAct->setCheckable(true);
    setHCModeAct->setChecked(false);
    setHCModeAct->setShortcut(QKeySequence(tr("Ctrl+H", "Handy Control")));


    setAIModeAct = new QAction(tr("AI"), this);
    setAIModeAct->setCheckable(true);
    setAIModeAct->setChecked(true);
    setAIModeAct->setShortcut(QKeySequence(tr("Ctrl+G", "AI")));

    setCoachModeAct = new QAction(tr("Started"), this);
    setCoachModeAct->setCheckable(true);
    setCoachModeAct->setChecked(haveCoach? true : false);

    setLogMode = new QAction(tr("Log") , this);
    setLogMode->setCheckable(true);
    setLogMode->setChecked(false);
    setLogMode->setShortcut(QKeySequence(tr("Ctrl+L" , "Log")));


    setReplayMode = new QAction(tr("Replay") , this);
    setReplayMode->setCheckable(true);
    setReplayMode->setChecked(false);
    setReplayMode->setShortcut(QKeySequence(tr("Ctrl+R" , "Replay")));

    setLogTagMode = new QAction(tr("LogTag") , this);
    setLogTagMode->setCheckable(true);
    setLogTagMode->setChecked(false);
    setLogTagMode->setShortcut(QKeySequence(tr("Ctrl+Shift+L" , "LogTag")));



    /* Menus */
    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(loadMonitorAct);
    viewMenu->addAction(loadTabsAct);
    viewMenu->addAction(loadPlotAct);
    viewMenu->addAction(loadPlaysWidgetAct);
    modeMenu = menuBar()->addMenu(tr("&Mode"));
    modeMenu->addAction(setSimModeAct);
    modeMenu->addAction(setRealModeAct);
    modeMenu->addAction(setSpyModeAct);
    modeMenu->addSeparator();
    modeMenu->addAction(setExp1Act);
    modeMenu->addAction(setExp2Act);
    modeMenu->addAction(setExp3Act);
    modeMenu->addAction(setExp4Act);
    modeMenu->addAction(setExp5Act);
    modeMenu->addAction(setExp6Act);
    modeMenu->addAction(setJsHandy);
    modeMenu->addAction(setAutoBallPlacement);
    modeMenu->addAction(setKProfiler);
    modeMenu->addAction(setFProfiler);
    modeMenu->addAction(setTechnicalChallengeAct);
    modeMenu->addAction(setMergeCamerasExperimentAct);
    modeMenu->addAction(setMixedAct);
    modeMenu->addSeparator();
    modeMenu->addAction(setAIModeAct);
    modeMenu->addAction(setHCModeAct);

    coachMenu = menuBar()->addMenu(tr("&Coach"));
    coachMenu->addAction(setCoachModeAct);

    loggerMenu = menuBar()->addMenu(tr("&Logger"));
    loggerMenu->addAction(setLogMode);
    loggerMenu->addAction(setReplayMode);
    loggerMenu->addAction(setLogTagMode);

    /* Final */
    this->showMaximized();
    this->setWindowTitle(QString("Parsian AI (%1)").arg(QString(REVNUM).remove("$")));

    simPlotSocket = NULL;reconnectSimPlot(NULL);


    connect(&loadFormationTimer , SIGNAL(timeout()) , this , SLOT(loadFormationConfigs()));
    loadFormationTimer.setInterval(2000);
    loadFormationTimer.start();


    /* Initialize Constants */


    /* Signal / Slots */
    connect(setCoachModeAct, SIGNAL(triggered(bool)), this, SLOT(stopStartCoach(bool)));
    connect(modeMenu, SIGNAL(triggered(QAction*)), this, SLOT(setQuiescentMode(QAction*)));
    connect(loggerMenu , SIGNAL(triggered(QAction*)) , this , SLOT(setLoggerReplayMode(QAction*)));
    connect(loadMonitorAct, SIGNAL(triggered(bool)), this, SLOT(showHideMonitorWidget(bool)));

    connect(loadTabsAct, SIGNAL(triggered(bool)), this, SLOT(showHideTabWidget(bool)));
    connect(loadPlotAct, SIGNAL(triggered(bool)), this, SLOT(showHidePlotWidget(bool)));
    connect(loadPlaysWidgetAct, SIGNAL(triggered(bool)), this, SLOT(showHidePlaysWidget(bool)));

    connect(tabWidget, SIGNAL(closeSignal(bool)), this, SLOT(showHideTabWidget(bool)));
    connect(monitorWidget, SIGNAL(closeSignal(bool)), this, SLOT(showHideMonitorWidget(bool)));
    connect(plotWidget, SIGNAL(closeSignal(bool)), this, SLOT(showHidePlotWidget(bool)));
    connect(loggerWidget , SIGNAL(rejected()) , this , SLOT(loggerWidgetRejected()));
    connect(keyboardWidget , SIGNAL(rejected()) , this , SLOT(keyboardWidgetRejected()));
    connect(trainWidget->start,SIGNAL(pressed()),this,SLOT(trainStart()));
    connect(trainWidget->end,SIGNAL(pressed()),this,SLOT(trainEnd()));

    connect(workspace, SIGNAL(windowActivated(QWidget*)), this, SLOT(updateActiveWidgetMenu(QWidget*)));
    connect(monitorWidget, SIGNAL(mousePosChanged(Vector2D,int)), this, SLOT(getMousePos(Vector2D,int)));

    //config related signals
    connect(conf()->v_Common_Viewport_Width,SIGNAL(wasEdited(VarType*)),monitorWidget,SLOT(changeViewportWidth(VarType*)));
    connect(conf()->v_LocalSettings_OurTeamSide,SIGNAL(wasEdited(VarType*)),this,SLOT(changeTeamSide(VarType*)));
    connect(conf()->v_LocalSettings_RefereeMulticastAddr,SIGNAL(wasEdited(VarType*)),this,SLOT(reconnectReferee(VarType*)));
    connect(conf()->v_LocalSettings_RefereeMulticastPort,SIGNAL(wasEdited(VarType*)),this,SLOT(reconnectReferee(VarType*)));
    connect(conf()->v_LocalSettings_SimulatorAddr,SIGNAL(wasEdited(VarType*)),this,SLOT(reconnectSimulator(VarType*)));
    connect(conf()->v_LocalSettings_SimulatorPort,SIGNAL(wasEdited(VarType*)),this,SLOT(reconnectSimulator(VarType*)));
    connect(conf()->v_LocalSettings_SerialDev,SIGNAL(wasEdited(VarType*)),this,SLOT(reconnectSerial(VarType*)));
    connect(conf()->v_LocalSettings_SerialRec,SIGNAL(wasEdited(VarType*)),this,SLOT(reconnectSerial(VarType*)));
    connect(conf()->v_LocalSettings_SSLVisionMulticastAddr,SIGNAL(wasEdited(VarType*)),this,SLOT(reconnectSSLVision(VarType*)));
    connect(conf()->v_LocalSettings_SSLVisionMulticastPort,SIGNAL(wasEdited(VarType*)),this,SLOT(reconnectSSLVision(VarType*)));
    connect(conf()->v_Plotter_data_time,SIGNAL(wasEdited(VarType*)),plotWidget,SLOT(data_time_changed(VarType*)));
    connect(conf()->v_Plotter_net_port,SIGNAL(wasEdited(VarType*)),this,SLOT(reconnectSimPlot(VarType*)));
    connect(conf()->v_LocalSettings_OurTeamColor, SIGNAL(wasEdited(VarType*)), this, SLOT(changeTeamColor(VarType*)));
    connect(conf()->v_BallTracker_onoffState, SIGNAL(wasEdited(VarType*)), this, SLOT(setBallTrackerOnOffState(VarType*)));
    connect(conf()->v_Experiments_AutoReferee_autorefereefMulticastAddr, SIGNAL(wasEdited(VarType*)), this, SLOT(reconfigureAutomatedReferee(VarType*)));
    connect(conf()->v_Experiments_AutoReferee_autorefereefMulticastPort, SIGNAL(wasEdited(VarType*)), this, SLOT(reconfigureAutomatedReferee(VarType*)));
    connect(conf()->v_Experiments_AutoReferee_autorefereefMulticastPort, SIGNAL(wasEdited(VarType*)), this, SLOT(reconfigureAutomatedReferee(VarType*)));

    //connect(defensePositioning, SIGNAL(update()), this, SLOT(updateMonitor()));
    connect(statusWidget->gameInfo->joystick, SIGNAL(clicked()), this, SLOT(joystickEnable()));

    tabWidget->tabs->addTab(conf(), "Config");
    tabWidget->tabs->addTab(policy(), "Policy");
    tabWidget->tabs->addTab(roleWidget,"Roles");
    tabWidget->tabs->addTab(skillWidget,"Skills");
    tabWidget->tabs->addTab(gpWidget, "Simulator");
    tabWidget->tabs->addTab(trainWidget,"Train");
    tabWidget->tabs->addTab(profilerWidget,"Profiler");
    tabWidget->tabs->addTab(playOffWidget, "Plans");
#ifdef DISTURBANCE_MATRIX
    tabWidget->tabs->addTab(learnEWidget, "Learn E");
#endif

    //    workspace->addWindow(plotWidget, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    //    plotWidget->hide();

//    rightTabWidget->tabs->addTab(playsWidget, "Plays");
//    rightTabWidget->tabs->addTab(knowledgeVarsWidget, "Knowledge");
//    rightTabWidget->tabs->addTab(behavioursWidget, "Behaviours");
//    rightTabWidget->tabs->addTab(rectSlect, "Rects");
//    rightTabWidget->tabs->addTab(condCheckWidget, "Conditions");

    initInfo((void*)statusWidget->info);

    runTimer = new QTimer;
    runTimer->setInterval(100);
    connect(runTimer, SIGNAL(timeout()), this, SLOT(run()));
    runTimer->start();


    //TODO: Find a way to do this via reConfigureAutoReferee Slot
    /*    wm->autoRef->reConfigure(
    conf()->Experiments_AutoReferee_autorefereefMulticastPort(),
    QString::fromStdString(conf()->Experiments_AutoReferee_autorefereefMulticastAddr()),
    QString(""));*/
    //wm->setSimulatorDesiredFPS(10);
    //without graphics every thing will be faster
    //wm->setSimulatorOpenGLState(false);
    //wm->setSimulatorOpen4ETimeStep(0.02); //it should be proportional to desired FPS
    //wm->setSimulatorDesiredFPS(50);

    mainapp = this;

    for( int i = 0; i < _MAX_NUM_PLAYERS;i++)
        wm->our[i]->setNewRobot(true);
    knowledge->roleOptimization = true;

    qDebug() << "L: " << wm->field->ourGoalL().x << wm->field->ourGoalL().y;
    qDebug() << "R: " << wm->field->ourGoalR().x << wm->field->ourGoalR().y;


    qDebug() << "L: " << (((wm->field->ourGoalL().x / wm->field->fieldRect().size().length()) + 1.0) * 100 / 2.0 ) <<
                (((wm->field->ourGoalL().y / wm->field->fieldRect().size().width()) + 1.0) * 100 / 2.0 );
    qDebug() << "R: " << (((wm->field->ourGoalR().x / wm->field->fieldRect().size().length()) + 1.0) * 100 / 2.0 ) <<
                (((wm->field->ourGoalR().y / wm->field->fieldRect().size().width()) + 1.0) * 100 / 2.0 );

    if (startUpMode == "real")
    {
        setQuiescentMode(setRealModeAct);
        if (coachMode.count() > 0)
            wm->gs->transition(coachMode.at(0).toAscii());
        //    if (!soccer->robCom->isSerialConnected())
        //    {
        //        soccer->connectSerial();
        //        if (!soccer->robCom->errorOccured())
        //            printer->textBuffer.enqueue(CStatusText("Send COM Port : " + QString(conf()->LocalSettings_SerialDev().c_str()) + " Succesfully configured", QColor("green")));
        //    }
        //    else if (!soccer->robCom->errorOccured())
        //        printer->textBuffer.enqueue(CStatusText("Send Using COM Port", QColor("green")));
        //    soccer->setMode(CSoccer::Real);
        //    setSpyModeAct->setChecked(false);
        //    setSimModeAct->setChecked(false);
        ////    comChanged = true;
        //    if( ! action->isChecked() )
        //    {
        //        setRealModeAct->setChecked(true);
        //    }
    }


    /////////////////////////////////////////mhmmd monitor update thread

    monitorUpdateThr = new monitorUpdateThread(this);
    monitorUpdateThr->start(QThread::LowestPriority);

    ///////////////////
}

void CMainApplication::refShortcutsChanged(bool val)
{
    if(val)
    {
        knowledge->setRefShortcuts(true);
    }
    else
    {
        knowledge->setRefShortcuts(false);
    }
}

void CMainApplication::SetManualGS( int id ){
    //added
    if(strRefCommands.at(id).toAscii().at(0) == 'b' || strRefCommands.at(id).toAscii().at(0) == 'B'){
        knowledge->setBPPosition(knowledge->getMousePos().x, knowledge->getMousePos().y);
        qDebug() << "pos : " << knowledge->getBPPosition().x << "," << knowledge->getBPPosition().y << endl;
    }

    wm->gs->transition(strRefCommands.at(id).toAscii().at(0));
}


void CMainApplication::run()
{

    runTimer->stop();
    soccer->run();

}

void CMainApplication::reconnectReferee(VarType*)
{
    soccer->connectReferee();
}

void CMainApplication::reconnectSSLVision(VarType*)
{   
    soccer->connectVision();
}

void CMainApplication::reconnectSimPlot(VarType*)
{
    if(simPlotSocket != NULL){
        disconnect(simPlotSocket,SIGNAL(readyRead()),this,SLOT(simPlotDataReady()));
        delete simPlotSocket;
    }
    simPlotSocket = new QUdpSocket(this);
    simPlotSocket->bind((conf()->Plotter_net_port()));

    connect(simPlotSocket,SIGNAL(readyRead()),this,SLOT(simPlotDataReady()));
}

void CMainApplication::reconnectSimulator(VarType*)
{
    soccer->connectSimulation();
}

void CMainApplication::reconnectSerial(VarType*)
{    
    soccer->connectSerial();
}

void CMainApplication::customControl(bool &custom)
{

    /////////////////////////
    static int loopFPSCounter = -1;
    wm->setCommandTimeStep(commandTimer.elapsed()/1000.0f);
    loopFPSCounter++;
    if( loopFPSTimer.elapsed() >= 1000 || loopFPSCounter == 0 )
    {
        commandFPS = loopFPSCounter;
        loopFPSCounter = 0;
        loopFPSTimer.restart();
    }
    //debug(QString("elapsed error : %1").arg(commandTimer.elapsed()),D_ERROR);
    commandTimer.restart();
    /////////////////////////

    //	if (rightTabWidget->tabs->currentWidget() == condCheckWidget)
    //		condCheckWidget->check();

    //	playsWidget->update();
    //	behavioursWidget->update();
    statusWidget->gameInfo->update();
    Vector2D stoppos = wm->ball->getStopPos();
    if (wm->ball->vel.length()>0.1)
    {
        draw(Segment2D(stoppos-wm->ball->vel.norm().rotatedVector(90)*0.25, stoppos+wm->ball->vel.norm().rotatedVector(90)*0.25), "red");
    }

    //	if (rightTabWidget->tabs->currentWidget() == knowledgeVarsWidget) knowledgeVarsWidget->update();
#ifdef GAME_MODE
    custom = false;
    return;
#endif
    //	if ( rectSlect->isVisible())
    //		rectSlect->update();
    custom = false;
    knowledge->setExperimentalMode(experimental);
    if (experimental != 0)
    {
        custom = true;
        if (experimental==1) Experimental1();
        if (experimental==2) Experimental2();
        if (experimental==3) Experimental3();
        if (experimental==4) Experimental4();
        if (experimental==5) Experimental5();
        if (experimental==6) Experimental6();
        if (experimental==7) TechnicalChallenge();
        if (experimental==9) JsHandy();
        if (experimental==10) kickProfiler();
        if (experimental==11) {
            if(ProfilerExecute)
                collectKickProfile->start();
        }
        if(experimental == 12)
        {
            autoBallPlacement();
        }

    }
    else {
        custom = false;
        if (tabWidget->tabs->currentWidget()==skillWidget) {
            custom = true;
            skillWidget->executeSkill();
            return;
        }
        if (tabWidget->tabs->currentWidget()==roleWidget) {
            custom = true;
            roleWidget->executeRoles();
            return;
        }
        if (loggerWidget->state()) {custom = true;return;}
        if (setSpyModeAct->isChecked()) {custom = true;return;}
        if (tabWidget->tabs->currentWidget() == profilerWidget) {
            custom = true;
            profilerWidget->execute();
            return;
        }
    }
}


void CMainApplication::showHideMonitorWidget(bool show)
{
    if( show )
    {
        monitorWidget->show();
    }
    else
    {
        monitorWidget->hide();
        loadMonitorAct->setChecked(false);
    }
}

void CMainApplication::showHideTabWidget(bool show)
{
    if( show )
    {
        tabWidget->show();
    }
    else
    {
        tabWidget->hide();
        loadTabsAct->setChecked(false);
    }
}

void CMainApplication::showHidePlotWidget(bool show)
{
    if( show )
    {
        //        plotWidget->show();
        tabWidget->tabs->addTab(plotWidget , "Plotter");
        tabWidget->tabs->setCurrentWidget(plotWidget);
    }
    else
    {
        //        plotWidget->hide();

        tabWidget->tabs->removeTab(tabWidget->tabs->indexOf(plotWidget));
        tabWidget->tabs->setCurrentIndex(0);

        loadPlotAct->setChecked(false);
    }
}

void CMainApplication::showHidePlaysWidget(bool show)
{
    if( show )
    {
        //		playsWidget2->show();
    }
    else
    {
        //		playsWidget2->hide();
    }
}


void CMainApplication::setQuiescentMode(QAction *action)
{

    /* Quiescent Mode */
    bool comChanged = false;
    if( action->text() == "Simulation" )
    {
        if (!soccer->robotCom->isUdpConnected())
        {
            soccer->connectSimulation();
            if (!soccer->robotCom->errorOccured())
                printer->textBuffer.enqueue(CStatusText("Connected to simulator command socket: " + QString(conf()->LocalSettings_SimulatorAddr().c_str()) + QString(":%1").arg(conf()->LocalSettings_SimulatorPort()), QColor("green")));
        }
        else if (!soccer->robotCom->errorOccured())
            printer->textBuffer.enqueue(CStatusText("Using simulator UDP socket", QColor("green")));
        haltAllRobots();
        soccer->setMode(CSoccer::Simulation);
        setSpyModeAct->setChecked(false);
        setRealModeAct->setChecked(false);
        comChanged = true;
        if( !action->isChecked() )
        {
            setSimModeAct->setChecked(true);
        }
    }
    else if( action->text() == "Real" )
    {
        if (!soccer->robotCom->isSerialConnected())
        {
            soccer->connectSerial();
            if (!soccer->robotCom->errorOccured())
                printer->textBuffer.enqueue(CStatusText("Send COM Port : " + QString(conf()->LocalSettings_SerialDev().c_str()) + " Succesfully configured", QColor("green")));
        }
        else if (!soccer->robotCom->errorOccured())
            printer->textBuffer.enqueue(CStatusText("Send Using COM Port", QColor("green")));
        soccer->setMode(CSoccer::Real);
        setSpyModeAct->setChecked(false);
        setSimModeAct->setChecked(false);
        comChanged = true;
        if( ! action->isChecked() )
        {
            setRealModeAct->setChecked(true);
        }
    }
    else if( action->text() == "Spy" ){
        soccer->setMode(CSoccer::Spy);
        setRealModeAct->setChecked(false);
        setSimModeAct->setChecked(false);
        if( ! action->isChecked() )
            setSpyModeAct->setChecked(true);
    }

    if (action->text() == setExp1Act->text())
    {
        experimental = 1;
        if (setExp1Act->isChecked()) setExp1Act->setChecked(true);
        else {setExp1Act->setChecked(false);experimental=0;}
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setExp2Act->text())
    {
        experimental = 2;
        if (setExp2Act->isChecked()) setExp2Act->setChecked(true);
        else {setExp2Act->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setExp3Act->text())
    {
        experimental = 3;
        if (setExp3Act->isChecked()) setExp3Act->setChecked(true);
        else {setExp3Act->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setExp4Act->text())
    {
        experimental = 4;
        if (setExp4Act->isChecked()) setExp4Act->setChecked(true);
        else {setExp4Act->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setExp5Act->text())
    {
        experimental = 5;
        if (setExp5Act->isChecked()) setExp5Act->setChecked(true);
        else {setExp5Act->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp6Act->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setExp6Act->text())
    {
        experimental = 6;
        if (setExp6Act->isChecked()) setExp6Act->setChecked(true);
        else {setExp6Act->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setTechnicalChallengeAct->text())
    {
        experimental = 7;
        if (setTechnicalChallengeAct->isChecked()) setTechnicalChallengeAct->setChecked(true);
        else {setTechnicalChallengeAct->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setJsHandy->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setMergeCamerasExperimentAct->text())
    {
        experimental = 8;
        if (setMergeCamerasExperimentAct->isChecked())
        {
            setMergeCamerasExperimentAct->setChecked(true);
        }
        else {
            setMergeCamerasExperimentAct->setChecked(false);experimental=0;
        }
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setJsHandy->setChecked(false);
        setTechnicalChallengeAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setJsHandy->text())
    {
        experimental = 9;
        if (setJsHandy->isChecked()) setJsHandy->setChecked(true);
        else {setJsHandy->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setKProfiler->text())
    {
        experimental = 10;
        if (setKProfiler->isChecked()) setKProfiler->setChecked(true);
        else {setKProfiler->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setJsHandy->setChecked(false);
        setJsHandy->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setFProfiler->setChecked(false);
    }
    else if (action->text() == setFProfiler->text())
    {
        experimental = 11;
        if (setFProfiler->isChecked()) setFProfiler->setChecked(true);
        else {setFProfiler->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setJsHandy->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setKProfiler->setChecked(false);
        setJsHandy->setChecked(false);
        CNewProfilerWidget *profilerWidget;
        profilerWidget=new CNewProfilerWidget(this);
        profilerWidget->show();
    }
    else if (action->text() == setAutoBallPlacement->text())
    {
        experimental = 12;
        if (setAutoBallPlacement->isChecked()) setAutoBallPlacement->setChecked(true);
        else {setAutoBallPlacement->setChecked(false);experimental=0;}
        setExp1Act->setChecked(false);
        setExp2Act->setChecked(false);
        setExp3Act->setChecked(false);
        setExp4Act->setChecked(false);
        setExp5Act->setChecked(false);
        setExp6Act->setChecked(false);
        setJsHandy->setChecked(false);
        setMergeCamerasExperimentAct->setChecked(false);
        setKProfiler->setChecked(false);
        setJsHandy->setChecked(false);
    }
    /* Control Mode */
    CSoccer::ControlMode lastControlMode = soccer->getControlMode();
    if (action->text() == "AI" )
    {
        if (action->isChecked() )
        {
            setHCModeAct->setChecked(false);
        }
        else
        {
            setHCModeAct->setChecked(false);
            setAIModeAct->setChecked(true);
        }

        tabWidget->tabs->removeTab(tabWidget->tabs->indexOf(keyboardWidget));
        tabWidget->tabs->setCurrentIndex(0);
        keyboardWidget->clearFocus();
        monitorWidget->setFocus();

        soccer->setControlMode(CSoccer::AI);
    }
    else if (action->text() == "Handy Control" )
    {
        if (action->isChecked() )
        {
            setAIModeAct->setChecked(false);
        }
        else
        {
            setAIModeAct->setChecked(false);
            setHCModeAct->setChecked(true);
        }

        tabWidget->tabs->addTab(keyboardWidget , "Handy Control");
        tabWidget->tabs->setCurrentWidget(keyboardWidget);
        keyboardWidget->setFocus();

        soccer->setControlMode(CSoccer::HandyControl);
    }

}

void CMainApplication::updateActiveWidgetMenu(QWidget* w)
{
    if (w == keyboardWidget)
    {
        keyboardWidget->jsTimer->start();
        soccer->setControlMode(CSoccer::HandyControl);
        setAIModeAct->setChecked(false);
        setHCModeAct->setChecked(true);
    }
    else
    {   keyboardWidget->jsTimer->stop();
        soccer->setControlMode(CSoccer::AI);
        setAIModeAct->setChecked(true);
        setHCModeAct->setChecked(false);
    }
}

void CMainApplication::updateMonitor()
{
    // return;
    knowledge->setMousePos(mousePos);
    displayFPS = 1000.0 / displayTimer.elapsed();
    displayTimer.restart();

    Rect2D mouseSelectRect;
    if( monitorWidget->drawMouseRect )
    {
        mouseSelectRect = Rect2D(monitorWidget->mousePressPos, mousePos);

    }

    //	if (knowledge->goalie != NULL)
    //	{
    //		draw(Circle2D(knowledge->goalie->pos(), 0.4), "yellow");
    //	}


    for( int i = 0; i < wm->our.activeAgentsCount(); i++ )
    {
        QColor col = wm->getTeamQColor();
        if (fabs(wm->our.active(i)->inSight-0.5)<0.01)
        {
            col.setAlpha(150);
        }
        int j = wm->our.active(i)->id;
        for (int k=1;k<wm->w.ourTeam[j].count();k++)
        {
            QColor col = col;
            col.setAlpha(100);
            drawerBuffer->drawRobot(wm->w.ourTeam[j][k]->pos, wm->w.ourTeam[j][k]->dir,
                                    col, j, soccer->agents[j]->commandID, "", soccer->agents[j]->self()->isNewRobot());
        }
        drawerBuffer->drawRobot(wm->our.active(i)->pos, wm->our.active(i)->dir,
                                col, wm->our.active(i)->id, soccer->agents[wm->our.active(i)->id]->commandID, "" ,soccer->agents[wm->our.active(i)->id]->self()->isNewRobot());
        if (soccer->agents[wm->our.active(i)->id]->skill!=NULL)
        {
            QString color = "cyan";
            if (soccer->agents[wm->our.active(i)->id]->skill->getName() == "position")
            {
                if (static_cast<CRolePosition*>(soccer->agents[wm->our.active(i)->id]->skill)->getDefaultPositioning())
                {
                    color = "purple";
                }
            }
            draw(soccer->agents[wm->our.active(i)->id]->skill->getName(), wm->our.active(i)->pos + Vector2D(0.1, 0.0), QColor(color));
            draw(soccer->agents[wm->our.active(i)->id]->skill->localAgentName, wm->our.active(i)->pos + Vector2D(0.2, 0.0), "yellow");
        }

        //        if (soccer->agents[wm->our.active(i)->id]->goalVisibility>0)
        //            draw(QString::number(soccer->agents[wm->our.active(i)->id]->goalVisibility,'f',2), wm->our.active(i)->pos + Vector2D(-0.3, -0.1), QColor("black"), 14);
        if (soccer->agents[wm->our.active(i)->id]->canRecvPass)
            draw(Circle2D(wm->our.active(i)->pos, wm->our.active(i)->robotRadius() + 0.04), 0, 360, QColor("purple"));
        if (soccer->agents[wm->our.active(i)->id]->idle)
            draw(Circle2D(wm->our.active(i)->pos, wm->our.active(i)->robotRadius() + 0.04), 0, 360, QColor("yellow"));
        if ((knowledge->selectedOur == true) && (knowledge->selectedId == wm->our.active(i)->id))
        {
            draw(Circle2D(wm->our.active(i)->pos, wm->our.active(i)->robotRadius() + 0.02), 0, 360, QColor("cyan"));
            draw(Circle2D(wm->our.active(i)->pos, wm->our.active(i)->robotRadius() + 0.04), 0, 360, QColor("cyan"));
        }

    }
    for( int i = 0; i < wm->opp.activeAgentsCount(); i++ )
    {
        QColor col = wm->getOppQColor();
        if (fabs(wm->opp.active(i)->inSight-0.5)<0.01)
        {
            col.setAlpha(150);
        }
        int j = wm->opp.active(i)->id;
        for (int k=1;k<wm->w.oppTeam[j].count();k++)
        {
            QColor col = col;
            col.setAlpha(100);
            drawerBuffer->drawRobot(wm->w.oppTeam[j][k]->pos, wm->w.oppTeam[j][k]->dir,
                                    col, j, soccer->agents[j]->commandID, "" , soccer->agents[j]->self()->isNewRobot());
        }
        drawerBuffer->drawRobot(wm->opp.active(i)->pos, wm->opp.active(i)->dir,
                                col, wm->opp.active(i)->id, -1);
        //////////////cleaned by mhmmd
        //                if (wm->opp.active(i)->role!="")
        //                        draw(wm->opp.active(i)->role, wm->opp.active(i)->pos + Vector2D(0.1, 0.0), QColor("red"));

        if ((knowledge->selectedOur == false) && (knowledge->selectedId == wm->opp.active(i)->id))
        {
            draw(Circle2D(wm->opp.active(i)->pos, wm->opp.active(i)->robotRadius() + 0.02), 0, 360, QColor("red"));
            draw(Circle2D(wm->opp.active(i)->pos, wm->opp.active(i)->robotRadius() + 0.04), 0, 360, QColor("red"));
        }
    }

    if( wm->ball->inSight > 0 )
    {
        draw(Circle2D(wm->ball->pos, CBall::radius), 0, 360, QColor("orange"), true);
        draw(Segment2D(wm->ball->pos, wm->ball->pos+wm->ball->vel/10.0), QColor("red"));
        for (int k=0;k<wm->w.ball.count();k++)
        {
            //            qDebug() << "Balls : " <<  wm->w.ball[k]->pos.x;
            drawerBuffer->draw(Circle2D(wm->w.ball[k]->pos, CBall::radius), 0, 360, QColor("orange"), false);
            drawerBuffer->draw(Segment2D(wm->w.ball[k]->pos, wm->w.ball[k]->pos+wm->w.ball[k]->vel/10.0), QColor("red"));
        }

        draw(Circle2D(wm->ball->pos,0.5),0,360,"red",false);
        QColor red("red");
        int ballHistSz = wm->ball->hist.count();
        for(int i=ballHistSz-1 ; i>=0 && ballHistSz-i<10 ;i--)
        {
            draw(Circle2D(wm->ball->hist[i].pos,wm->ball->radius),0,360,red.lighter(i),false);
        }
        QColor green("green");
        draw(Segment2D(wm->ball->pos + wm->ball->vel / 5, wm->ball->pos) , "blue" );

        //        if(wm->ball->vel.length()>0.1)
        //        for(int i=0; i<100;i++)
        //        {
        //            draw(Circle2D(wm->ball->predict((1.0/wm->getVisionFPS())*(double)i),wm->ball->radius),0,360,green.lighter(i),false);
        //        }
    }

    //    halfworldMutex->lock();
    //    for (int i=0;i<halfworld->ball.count();i++)
    //        draw(Circle2D(halfworld->ball[i]->pos,0.02), "red");
    //    halfworldMutex->unlock();

    monitorWidget->updateGL();
    // statusWidget->update();
    //statusWidget->agentsWidget->update();

    static QString msg1;
    static QString msg2;
    static QString msg3;
    static QString msg4;
    static QString msg5;
    static QString msg6;

    if( ++tempCnt % 10 == 0 )
    {
        tempCnt = 0;
        msg1 = QString("FPS( Vision: %1").arg(wm->getVisionFPS(), 2, 'f', 0) ;
        msg2 = QString(" | Display: %1").arg(displayFPS, 2, 'f', 2);
        msg3 = QString(" | Command: %1 )").arg(commandFPS, 2, 'f', 2);
        //msg4 = QString(" | Latency: %1 ms").arg(vc->res.visionLatency*1000.0);
    }
    msg5 = QString(" [%1, %2]").arg(mousePos.x, 2, 'f', 2).arg(mousePos.y, 2, 'f', 2);
    statusbarLabel->setText(msg1 + msg2 + msg3 + msg4 + msg5 + msg6);
    //    statusBar()->showMessage(msg1 + msg2 + msg3 + msg4 + msg5 + msg6);
    //statusBar()->add
}

void CMainApplication::stopStartCoach(bool start)
{
    haveCoach = start;
}

void CMainApplication::getMousePos(Vector2D _pos, int mouseClick)
{
    skillWidget->getMousePos(_pos);
    mousePos = _pos;
    if (mouseClick == Qt::RightButton) {
        soccer->selectBall(_pos);
        soccer->selectRobot(_pos);
        soccer->coach->swapAgents();
    }

}

void CMainApplication::changeTeamColor(VarType*) {   
    if(conf()->LocalSettings_OurTeamColor()=="Blue"){
        soccer->setTeamColor(_COLOR_BLUE);
        //added
        wm->our.setColor(_COLOR_BLUE);
        wm->opp.setColor(_COLOR_YELLOW);

    }
    if (conf()->LocalSettings_OurTeamColor()=="Yellow") {
        soccer->setTeamColor(_COLOR_YELLOW);
        //added
        wm->our.setColor(_COLOR_YELLOW);
        wm->opp.setColor(_COLOR_BLUE);
    }
    wm->gs->init((wm->getTeamColor() == _COLOR_YELLOW) ? TEAM_YELLOW : TEAM_BLUE );
}

void CMainApplication::changeTeamSide(VarType*) {
    if(conf()->LocalSettings_OurTeamSide()=="Right"){
        soccer->setTeamSide(_SIDE_RIGHT);
    }
    if(conf()->LocalSettings_OurTeamSide()=="Left"){
        soccer->setTeamSide(_SIDE_LEFT);
    }
}

void CMainApplication::closeEvent(QCloseEvent* /*event*/)
{
    soccer->closeAll();
    haltAllRobots();
    conf()->save();
    policy()->save();
}

CMainApplication::~CMainApplication()
{

    delete profilerWidget;
    delete playOffWidget;

    delete soccer;
    delete conf();
    delete policy();
    delete globalTime;
    //    //delete monitorWidget;
    delete drawerBuffer;
    delete printer;
    delete logger;
    delete gpWidget;
    //delete policyLearner;
    delete keyboardWidget;
    delete plotWidget;
    delete statusWidget;
    delete setAutoBallPlacement;
    delete skillWidget;
    delete roleWidget;


#ifdef DISTURBANCE_MATRIX
    delete learnEWidget;
#endif
    //	delete behavioursWidget;
    delete loadMonitorAct;
    delete loadPlotAct;
    delete loadTabsAct;
    delete setSimModeAct;
    delete setRealModeAct;
    delete setMixedAct;
    delete setExp1Act;
    delete setExp2Act;
    delete setExp3Act;
    delete setExp4Act;
    delete setExp5Act;
    delete setExp6Act;
    delete setJsHandy;
    delete setKProfiler;
    delete setFProfiler;
    delete setTechnicalChallengeAct;
    delete setMergeCamerasExperimentAct;
    delete tabWidget;
    delete workspace;

}

void CMainApplication::showEvent(QShowEvent* /*event*/){
    tabWidget->resize(tabWidget->tabs->size());
    tabWidget->move(tabWidget->pos()+QPoint(80,0));
}

void CMainApplication::simPlotDataReady(){
    debug("UPDATEddddddddd" , D_GAME);
    while (simPlotSocket->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(simPlotSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        simPlotSocket->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);
        if(datagram.size() == sizeof(float)*13){
            float id   = *((float*) (datagram.data()) + 0);
            float px   = *((float*) (datagram.data()) + 1)/1000.0;
            float py   = *((float*) (datagram.data()) + 2)/1000.0;
            float pdir = *((float*) (datagram.data()) + 4);
            float vx   = *((float*) (datagram.data()) + 5)/1000.0;
            float vy   = *((float*) (datagram.data()) + 6)/1000.0;
            float vdir = *((float*) (datagram.data()) + 8);
            float ax   = *((float*) (datagram.data()) + 9)/1000.0;
            float ay   = *((float*) (datagram.data()) + 10)/1000.0;
            float adir = *((float*) (datagram.data()) + 12);
            if(networkDataFilterTimer.elapsed() > 10)
            {
                if(id!=-1)
                {
                    if(int(id/100) == 0){
                        if(conf()->LocalSettings_OurTeamColor() == "Blue"){
                            plotWidget->simul->our[(int)id]->pos.x = px;
                            plotWidget->simul->our[(int)id]->pos.y = py;

                            plotWidget->simul->our[(int)id]->dir.setDir(pdir);
                            plotWidget->simul->our[(int)id]->dir.normalize();

                            plotWidget->simul->our[(int)id]->vel.x = vx;
                            plotWidget->simul->our[(int)id]->vel.y = vy;

                            plotWidget->simul->our[(int)id]->angularVel = vdir*_RAD2DEG;

                            plotWidget->simul->our[(int)id]->acc.x = ax;
                            plotWidget->simul->our[(int)id]->acc.y = ay;
                        }
                        else if( conf()->LocalSettings_OurTeamColor() == "Yellow" ){
                            plotWidget->simul->opp[(int)id]->pos.x = px;
                            plotWidget->simul->opp[(int)id]->pos.y = py;

                            plotWidget->simul->opp[(int)id]->dir.setDir(pdir);
                            plotWidget->simul->opp[(int)id]->dir.normalize();

                            plotWidget->simul->opp[(int)id]->vel.x = vx;
                            plotWidget->simul->opp[(int)id]->vel.y = vy;

                            plotWidget->simul->opp[(int)id]->angularVel = vdir*_RAD2DEG;

                            plotWidget->simul->opp[(int)id]->acc.x = ax;
                            plotWidget->simul->opp[(int)id]->acc.y = ay;
                        }
                    }
                    else if(int(id/100) == 1){
                        if(conf()->LocalSettings_OurTeamColor() == "Blue"){
                            plotWidget->simul->opp[(int)id - 105]->pos.x = px;
                            plotWidget->simul->opp[(int)id - 105]->pos.y = py;

                            plotWidget->simul->opp[(int)id - 105]->dir.setDir(pdir);
                            plotWidget->simul->opp[(int)id - 105]->dir.normalize();

                            plotWidget->simul->opp[(int)id - 105]->vel.x = vx;
                            plotWidget->simul->opp[(int)id - 105]->vel.y = vy;

                            plotWidget->simul->opp[(int)id - 105]->angularVel = vdir*_RAD2DEG;


                            plotWidget->simul->opp[(int)id - 105]->acc.x = ax;
                            plotWidget->simul->opp[(int)id - 105]->acc.y = ay;
                        }
                        else if(conf()->LocalSettings_OurTeamColor() == "Yellow"){
                            plotWidget->simul->our[(int)id - 105]->pos.x = px;
                            plotWidget->simul->our[(int)id - 105]->pos.y = py;

                            plotWidget->simul->our[(int)id - 105]->dir.setDir(pdir);
                            plotWidget->simul->our[(int)id - 105]->dir.normalize();

                            plotWidget->simul->our[(int)id - 105]->vel.x = vx;
                            plotWidget->simul->our[(int)id - 105]->vel.y = vy;

                            plotWidget->simul->our[(int)id - 105]->angularVel = vdir*_RAD2DEG;

                            plotWidget->simul->our[(int)id - 105]->acc.x = ax;
                            plotWidget->simul->our[(int)id - 105]->acc.y = ay;
                        }
                    }
                }
                else{
                    plotWidget->simul->ball->pos.x = px;
                    plotWidget->simul->ball->pos.y = py;

                    plotWidget->simul->ball->vel.x = vx;
                    plotWidget->simul->ball->vel.y = vy;

                    plotWidget->simul->ball->acc.x = ax;
                    plotWidget->simul->ball->acc.y = ay;
                }
                if(id == 109)networkDataFilterTimer.restart();//IMPORTANT: MAY CAUSE BUG
            }
        }
        else{

            qDebug()<<"Simulator Plotter: invalid packet size:"<<datagram.size();
        }
    }
}

void CMainApplication::sampleTimeChanged()
{
    for (int i=0;i<_NUM_PLAYERS;i++)
    {
        //wm->ourTeam[i].initKalman();
    }
}


void CMainApplication::setBallTrackerOnOffState(VarType*)
{
}

void CMainApplication::reconfigureAutomatedReferee(VarType *)
{
    /*    wm->autoRef->reConfigure(
    conf()->Experiments_AutoReferee_autorefereefMulticastPort(),
    QString::fromStdString(conf()->Experiments_AutoReferee_autorefereefMulticastAddr()),
    QString(""));*/
}

void CMainApplication::stopALL()
{
    //return ;
    /*    if(controlMode !=_STOP)
 {
  controlMode = _STOP;
  STOPALL->setText("AI");
 }else{
  controlMode = _AI_MODE;
  STOPALL->setText("Stop All");
 }
 for  (int i = 0; i < _NUM_PLAYERS; i++ )
 {


   agents[i]->setMotorsVel(0, 0, 0, 0);
   agents[i]->setKick(false, 0);
   agents[i]->setChip(false, 0);
   robCom->sendString(agents[i]->getOutputBuffer(), _PACKET_SIZE);


 }

*/
}

void CMainApplication::updateCoach()
{
}

void haltAllRobots()
{    
    char o[20];
    for (int k=0;k<100;k++)
    {
        for (int id=0;id<10;id++)
        {
            for (int i=0;i<20;i++) o[i]=0;
            o[0] = 0x99;
            o[1] = id & 0x0F;
            o[3] =o[4] =o[5] =o[6] =o[7] =o[8]=o[2] = 0x00;
            mainapp->soccer->robotCom->sendString(o, 14);
        }
    }
}


/// Sort the array between a low and high bound in either ascending or descending order
/// Parameters:
///      int  nFirst : between 0 and the upper bound of the array.
///      int  nLast  : between 0 and the upper bound of the array.
///                  : must be guaranteed >= nFirst.
///      bool bAscend: true  - sort in ascending order
///                  : false - sort in descending order


//TEMP
void CMainApplication::logBtnPressed()
{
    if(!logging)
    {
        logging = true;
        logStart->setText("Stop");
    }
    else
    {
        logging = false;
        //        sData->flush();
        //        fData->close();
    }
}

void CMainApplication::setLoggerReplayMode(QAction *action ){
    if( action->text() == "LogTag"){
        if( setReplayMode->isChecked() ){
            terminateLogOrReplay(false); // terminate Replay
        }
        if( action->isChecked() ){
            CLogTagWidget *LT;

            LT=new CLogTagWidget(this);
            LT->show();


        }
        else{
            terminateLogOrReplay(true);
        }

    }
    else if( action->text() == "Log" ){
        if( setReplayMode->isChecked() ){
            terminateLogOrReplay(false); // terminate Replay
        }
        if( action->isChecked() ){
            QChar cc = '0';
            QString suggestionName = QString("%1%2%3-%4%5%6")
                    .arg(QString::number(QDate::currentDate().year()) , 4 , cc)
                    .arg(QString::number(QDate::currentDate().month()) , 2 , cc)
                    .arg(QString::number(QDate::currentDate().day()) , 2 , cc)
                    .arg(QString::number(QTime::currentTime().hour()) , 2 , cc)
                    .arg(QString::number(QTime::currentTime().minute()) , 2 , cc)
                    .arg(QString::number(QTime::currentTime().second()) , 2 , cc);

            bool ok;
            QString totlaDescription=suggestionName+"#Nadia#robocup-germany#test plan1#shoot-direct";
            QString baseFileName = QInputDialog::getText(this, tr("Name") , tr("Enter the log name's: ") , QLineEdit::Normal , suggestionName , &ok);
            if( !ok )
                baseFileName = suggestionName;

            loggerMutex->lock();
            gameLogger->setIsLogMode(true);
            gameLogger->closeLogger = false;
            gameLogger->logMode = true;
            gameLogger->openFilesToLog(baseFileName,totlaDescription);
            loggerMutex->unlock();

            gameLogger->start(QThread::NormalPriority);
        }
        else{
            terminateLogOrReplay(true);
        }
    }
    else if( action->text() == "Replay" ){
        if(setLogMode->isChecked() || setLogTagMode->isChecked()){
            terminateLogOrReplay(true); // terminate log
        }
        if( action->isChecked() ){
            loggerWidget->isLoggerWorking = true;

            loggerMutex->lock();
            gameLogger->setIsReplayMode(true);
            gameLogger->closeLogger = false;
            gameLogger->replayMode = true;
            loggerMutex->unlock();

            gameLogger->start(QThread::NormalPriority);
            loggerWidget->reBuildWidget();

            //loggerWidget->show();
            tabWidget->tabs->addTab(loggerWidget , "Replay");
            tabWidget->tabs->setCurrentWidget(loggerWidget);

            loggerWidget->setFocus();
            replayBuffer->clear();
            monitorWidget->setDrawerBuffer(replayBuffer);
        }
        else{
            terminateLogOrReplay(false);
        }
    }

}









void CMainApplication::loggerWidgetRejected(){
    setReplayMode->trigger();
}

void CMainApplication::keyboardWidgetRejected(){
    setHCModeAct->trigger();
}

void CMainApplication::terminateLogOrReplay(bool whichOne ){
    if( whichOne ){
        setLogMode->setChecked(false);
        setLogTagMode->setChecked(false);

        loggerMutex->lock();
        gameLogger->closeLogger = true;
        gameLogger->setIsLogMode(false);
        gameLogger->logMode = false;
        gameLogger->closeLogFiles(false);
        loggerMutex->unlock();
    }
    else{
        setReplayMode->setChecked(false);
        loggerWidget->isLoggerWorking = false;

        //loggerWidget->hide();
        tabWidget->tabs->removeTab(tabWidget->tabs->indexOf(loggerWidget));
        tabWidget->tabs->setCurrentIndex(0);

        loggerWidget->clearFocus();
        monitorWidget->setFocus();
        drawerBuffer->clear();
        monitorWidget->setDrawerBuffer(drawerBuffer);

        loggerMutex->lock();
        gameLogger->closeLogger = true;
        gameLogger->setIsReplayMode(false);
        gameLogger->replayMode = false;
        loggerMutex->unlock();
    }
}

MySoccer::MySoccer() : CSoccer()
{

}

void MySoccer::monitorUpdate()
{
    mainapp->updateMonitor();
}

void MySoccer::customControl(bool &custom)
{
    mainapp->customControl(custom);
}

void CMainApplication::joystickEnable()
{
#ifndef Q_WS_MAC
#ifndef NO_JS
    if (1)// statusWidget->gameInfo->joystick->isChecked())
    {
        if ( soccer->joystick->isJsConnected())
            soccer->joystick->start(QThread::LowPriority);
    }
    else
    {
        if( soccer->joystick->isRunning())
            soccer->joystick->quit();
    }
#endif
#endif
}

void CMainApplication::trainEnd()
{
    soccer->agents[new_com_test_robot_id]->startTrain = false;
}

void CMainApplication::trainStart()
{
    soccer->agents[new_com_test_robot_id]->startTrain = true;
}

void CMainApplication::loadFormationConfigs(){
    loadFormationTimer.stop();
    QString prefix = "formation/config/";
#ifdef Q_WS_MAC
    prefix = "../../../" + prefix;
#endif
    /////Add this line in your play config files
    ///	 .pos_config = 2attacker   (or any other positioning config file defined below
    CCoach::editData["OurFreeKick6"] = new EditData;
    CCoach::editData["OurFreeKick6"]->openConf(prefix+"our_freekick_6.conf");
    CCoach::editData["TheirFreeKick6"] = new EditData;
    CCoach::editData["TheirFreeKick6"]->openConf(prefix+"their_freekick_6.conf");
    CCoach::editData["PlayOnDefense"] = new EditData;
    CCoach::editData["PlayOnDefense"]->openConf(prefix+"play_on_defense.conf");
    CCoach::editData["PlayOnOffense"] = new EditData;
    CCoach::editData["PlayOnOffense"]->openConf(prefix+"play_on_offense.conf");
    CCoach::editData["Stop6"] = new EditData;
    CCoach::editData["Stop6"]->openConf(prefix+"stop_6.conf");
    CCoach::editData["OurKickOff"] = new EditData;
    CCoach::editData["OurKickOff"]->openConf(prefix+"our_kickoff.conf");
    CCoach::editData["TheirPenaltyKick"] = new EditData;
    CCoach::editData["TheirPenaltyKick"]->openConf(prefix+"their_penalty_kick.conf");
    CCoach::editData["OurPenaltyKick"] = new EditData;
    CCoach::editData["OurPenaltyKick"]->openConf(prefix+"our_penalty_kick.conf");
    CCoach::editData["TheirP"] = new EditData;
    CCoach::editData["TheirP"]->openConf(prefix+"theirP.conf");
    CCoach::editData["OurP"] = new EditData;
    CCoach::editData["OurP"]->openConf(prefix+"ourP.conf");

}



////////////////////////////////////////////////mhmmd monitor update new thread
monitorUpdateThread::monitorUpdateThread(QObject *parent) :
    QThread(parent)
{
    timer = new QTimer(this);
    timer->setInterval(200);
    connect(timer,SIGNAL(timeout()),this,SLOT(mainLoop()));
}

void monitorUpdateThread::run()
{
    timer->start();
}

void monitorUpdateThread::mainLoop()
{
    // mainapp->statusWidget->agentsWidget->update();
    mainapp->statusWidget->update();
}


/////////////////////////////////////////////////
