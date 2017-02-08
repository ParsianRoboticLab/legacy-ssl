#ifndef WIDGETS_H
#define WIDGETS_H

#include "motiontrainer.h"
#include "base.h"
#include "varswidget.h"
#include "communicator.h"
#include "worldmodel.h"
#include "coach.h"
#include "agent.h"
#include "drawer.h"
#include "VarTypes/VarTypes.h"
#include "varswidget.h"
#include "skills.h"
#include "roles.h"
#include "profiler.h"
#include "graphwidget.h"
#include "gamelogger.h"
#include "util/MLP.h"
#include "profiler.h"
#include "recorder.h"
#include "tools/loadplayoffjson.h"
#include "tools/planloader.h"

#include <QDockWidget>
#include <QLabel>
#include <QGridLayout>
#include <QTableWidget>
#include <QItemDelegate>
#include <QMetaType>
#include <QPointF>
#include <QVector>
#include <QGLWidget>
#include <QWidget>
#include <QFocusEvent>
#include <QPainter>
#include <QToolButton>
#include <QPushButton>
#include <QLCDNumber>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QFrame>
#include <QSize>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QColor>
#include <QTextDocument>
#include <QTime>
#include <QComboBox>
#include <QStackedWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QButtonGroup>
#include <QProgressBar>
#include <QDockWidget>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include <QCheckBox>
#include <QTime>
#include <QColorDialog>
#include <QPaintEvent>
#include <QHostAddress>
#include <QLabel>
#include <QGLWidget>
#include <QSize>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QSpacerItem>
#include <QSlider>
#include <QTreeView>
#include <QScrollBar>
#include <QMessageBox>
#include <QFrame>

#include<QMenuBar>

#define CHANNELS_COUNT 11

class CTabDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    CTabDockWidget(QWidget* parent , bool autoHideAbility);
    ~CTabDockWidget();
    QTabWidget *tabs;
    QPushButton *btnAutoHide;
    QWidget* w;
    QTimer *hideShowTimer;
    const bool autoHideBool;
    bool isHide , hideable;
signals:
    void closeSignal(bool);
protected:
    void closeEvent(QEvent*);
public slots:
    void autoHide();
    void setHideable();
};

class CInfoWidget : public QGLWidget
{
    Q_OBJECT
public:
    CInfoWidget(QWidget* parent=0);
    ~CInfoWidget();

    void setSenders(QHostAddress *_refSender,QHostAddress *_visSender);
    void setWorldModel(CWorldModel *_wm);
    void setBallsCount(int _ballsCount);
    void render (QPainter *p, const QSize& size);
    QString status;
protected:
    void paintEvent(QPaintEvent*);
    void closeEvent(QCloseEvent*);
    void resizeEvent (QResizeEvent*);
private:
    QHostAddress *refSender,*visSender;
    CWorldModel *wm;
    int ballsCount;
public slots:
    void update();
signals:
    void closeSignal(bool);
};

class CGameInfoWidget : public QWidget
{
    Q_OBJECT
public:
    QTableWidget* table;
    QComboBox* technical;
    QCheckBox* joystick;
    QMap<QString, QTableWidgetItem*> items;
    CGameInfoWidget();
    ~CGameInfoWidget();
    void update();
public slots:
    void technicalChanged(QString t);
};

class CAgentsWidget : public QWidget
{
    Q_OBJECT
    QPushButton *btnLoadConf , *btnSaveConf , *btnWarmup;
    QPixmap *robotsBlub[_MAX_NUM_PLAYERS];
    QLabel *lblRobotsBlub[_MAX_NUM_PLAYERS];
    QPainter *painter;
    QPushButton *btnGyro[_MAX_NUM_PLAYERS] , *btnKick[_MAX_NUM_PLAYERS] , *btnChip[_MAX_NUM_PLAYERS] , *btnSpin[_MAX_NUM_PLAYERS] , *btnOn[_MAX_NUM_PLAYERS] , *btnIn[_MAX_NUM_PLAYERS] ;
    QButtonGroup *btnGroupGyro , *btnGroupKick , *btnGroupChip , *btnGroupSpin , *btnGroupOn , *btnGroupIn;
    int gyro[_MAX_NUM_PLAYERS] , kick[_MAX_NUM_PLAYERS] , chip[_MAX_NUM_PLAYERS] , spin[_MAX_NUM_PLAYERS] , on[_MAX_NUM_PLAYERS] , in[_MAX_NUM_PLAYERS];
    int starter;
    QComboBox *cmbStarter;
    QLabel *lblStarter , *lbls[_MAX_NUM_PLAYERS];
    CAgent **agents;
    CCoach *coach;
    int infoCount;
    void hideColNumber(int);
    void showColNumber(int);
    void connectToSlot();
    void disConnectFromSlot();
    bool dontUpdate;
public:
    CAgentsWidget(CAgent **_agents);
    ~CAgentsWidget();
    void update();
    void setCoach(CCoach *_coach);

public slots:
    void changed();
    void loadConf();
    void saveConf();
    void setWarmup();
    void gyroChange(int);
    void kickChange(int);
    void chipChange(int);
    void spinChange(int);
    void onChange(int);
    void inChange(int);
    void starterChange(int);
};

class CStatusWidget : public QDockWidget
{
    Q_OBJECT
public:
    CInfoWidget* info;
    CAgentsWidget *agentsWidget;
    CGameInfoWidget* gameInfo;
    CStatusWidget(CStatusPrinter* _statusPrinter , CAgent ** , CCoach *);
    ~CStatusWidget();
    QTextEdit *statusText;
    QLabel *titleLbl;
    QTextDocument content;

public slots:
    void write(QString str, QColor color = QColor("black"));
    void update();

private:
    CStatusPrinter *statusPrinter;
    QTime logTime;

signals:
    void closeSignal(bool);

protected:
    void closeEvent(QEvent*);

};

/************************* SKILL WIDGET *************************/
class CSkillWidget : public QWidget
{
    Q_OBJECT

public:
    CSkillWidget(CAgent **_agents, QWidget *parent = 0);
    ~CSkillWidget();
    bool executing;
    void getMousePos(Vector2D _pos);

public slots:
    bool executeSkill();
    void startSkill();
    void stopSkill();
    void setCurrentAgent(int num);
    void setCurrentPage(int num);

protected:

    CAgent **agents;
    int currentAgent;
    bool stopExecution;
    bool timeReported;
    Vector2D interceptInitialPoint;

    QComboBox *agentsCombo;
    QComboBox *skillsCombo;
    QStackedWidget *pagesWidget;
    QPushButton *execButton;
    QPushButton *stopButton;
    QProgressBar *skillPrgrs;

    Vector2D mousePos;

    QList<CSkillConfigWidget*> pages;

signals:
    void closeSignal(bool);
protected:
    void closeEvent(QEvent*);

};

////////////////MAHI :D

class CProfilerWidget : public QWidget
{
    Q_OBJECT

public:
    CProfilerWidget(CAgent **_agents, QWidget *parent = 0);
    ~CProfilerWidget();
    void startRecord();
    void stopRecord();
    void execute();

protected:

    CAgent **agents;
    int currentAgent;

    QComboBox   *agentsCombo;
    QComboBox   *skillsCombo;
    QPushButton *addButton;
    QPushButton *saveButton;
    QPushButton *loadButton;
    QPushButton *execButton;
    QPushButton *stopButton;
    QPushButton *applyButton;
    QPushButton *refreshButton;
    QPushButton *clearButton;
    QPushButton *insertButton;

    QLineEdit *kickSpeed;
    QLineEdit *realSpeed;

    QLabel    *fileDirLable;
    QLabel    *theMeanLable;


    QTextEdit *editor;
    QString fileDir;

public slots:
    void slt_save();
    void slt_load();
    void slt_add();
    void slt_exec();
    void slt_stop();
    void slt_apply();
    void slt_refresh();
    void slt_clear();
    void slt_insert();
    void slt_setCurrentAgent(int);
    void slt_modeEdited(QString);
    void slt_modeEdited(int);

private:
    ProfileMode getMode(int _index);

    bool executing;
    bool stopExecuting;
    double theMean;
    QList<double> ballSpeedAddedList;
    QList<double> guessBallSpeedList;

};


class CPlayOffWidget : public QWidget {

    Q_OBJECT

public:
    CPlayOffWidget(CLoadPlayOffJson* _loader, QWidget *parent = 0);
    ~CPlayOffWidget();

protected:

    QPushButton *mode;
    QPushButton *active;
    QPushButton *update;
    QPushButton *master;
    QPushButton *deactive;

    QColumnView *columns;

    bool debugMode = false;

    QStandardItemModel *model;
    QItemSelectionModel *selection;

    QList<QStandardItem> *fileList;
    QList<QStandardItem> *planList;

    QList<NGameOff::SPlan*> m_plans;
    QList< QList< SPlayOffPlan*> >plans;
    CLoadPlayOffJson* m_loader;

    QLabel *details[8];

    NGameOff::SPlan* m_choosen;
private slots:

    void updateModel();
    void updateBtn(bool _debug);

public slots:
    void slt_changeMode();
    void slt_updatePlans();
    void slt_active();
    void slt_deactive();
    void slt_edit(QStandardItem*);
    void slt_master();
    void slt_selectionChanged(const QItemSelection &, const QItemSelection &);

};

//////////////////////////////
//////////////////////////////

class CRoleWidget : public QWidget
{
    Q_OBJECT
public:
    int rolescount;
    int playersNum;
    bool executing;
    CRoleWidget(CAgent** _agents, QWidget* parent = 0);
    ~CRoleWidget();
    bool executeRoles();
protected:
    CAgent **agents;
    QList<QComboBox*> rolesCombos;
    QStackedWidget* configPages;
    Vector2D mousePos;
    QPushButton *execButton;
    QButtonGroup *btnGroup;
public slots:
    void curAgentChanged(int i);
    void curRoleChanged(int i);
    void execBtnTriggered();
};

class CPlotWidget;


struct Sobject{
    Vector2D pos;
    Vector2D vel;
    Vector2D acc;
    Vector2D dir;
    double angularVel;
};

struct SSimulator{
    Sobject *our[5];
    Sobject *opp[5];
    Sobject *ball;
    SSimulator(){
        for( int i=0 ; i<5 ; i++ )
            our[i] = new Sobject() , opp[i] = new Sobject();
        ball = new Sobject();
    }

    ~SSimulator(){
        delete ball;
        for( int i=4 ; i>=0 ; i-- )
            delete opp[i] , delete our[i];
    }

    ETeamColorType getTeamColor()
    {
        return _COLOR_BLUE;
    }
};

class CPlotWidget : public QDialog
{
    Q_OBJECT
public:
    CGraphWidget *graph;
    struct SSimulator *simul;

    QCheckBox *OnOff[CHANNELS_COUNT+1];
    QComboBox *cmbMode[CHANNELS_COUNT];
    QComboBox *cmbTeam[CHANNELS_COUNT];
    QComboBox *cmbID[CHANNELS_COUNT];
    QComboBox *cmbPosVelAcc[CHANNELS_COUNT];
    QComboBox *cmbXYZ[CHANNELS_COUNT];
    QPushButton *btnColor[CHANNELS_COUNT];

    QPushButton *btnPause;
    QPushButton *btnSavePic;
    QPushButton *btnDock;
    QTimer *looptmr;
    QTimer *plotTimer;
    bool isPaused;


    double custom[CHANNELS_COUNT];

    CPlotWidget();
    ~CPlotWidget();
    double desiredValue(int);
    double GAIN;

private:
    template <class T>
    double recognizeTeam(T *, int);
    template <class T>
    double recognizeVar(T *, int);
    template <class T>
    double recognizeDir(T , int);

public slots:
    void pauseClicked();
    void savePicClicked();
    void updatePlots();
    void AddDelChnl0(bool);
    void AddDelChnl1(bool);
    void AddDelChnl2(bool);
    void AddDelChnl3(bool);
    void AddDelCustom0Chnl(bool);
    void AddDelCustom1Chnl(bool);
    void AddDelCustom2Chnl(bool);
    void AddDelCustom3Chnl(bool);
    void AddDelCustom4Chnl(bool);
    void AddDelCustom5Chnl(bool);
    void AddDelCustom6Chnl(bool);
    void AddDelCustom7Chnl(bool);

signals:
    void closeSignal(bool);
};

struct FrameData
{
    QImage* img;
    int msecs;
};

class CSimulatorControllerWidget;

class CMonitorWidget : public QGLWidget
{
    Q_OBJECT

    QTimer *recShowTimer;
    bool recShowBool;

public:
    CMonitorWidget(CDrawer* _drawerBuffer , QWidget *parent = 0);
    ~CMonitorWidget();
    int getViewportWidth();
    void setViewportWidth(int width);
    void save();
    void saveAll();
    void setDrawerBuffer(CDrawer *);

    bool drawMouseRect;
    bool addToSelection;
    Vector2D mousePressPos;

    CSimulatorControllerWidget *simulatorControlWidget;

public slots:
    void changeViewportWidth(VarType* v);
    void showHideRec();

signals:
    void closeSignal(bool);
    void mousePosChanged(Vector2D, int);
    void setBallPos(float, float);
    void aPosToGo(float, float);
    void aPosToGo(Vector2D);
    void mouseReleasedInmonitor();
    void mouseMovePos(Vector2D);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QList<FrameData> images;
    QTime timer;
    CDrawer *drawerBuffer;
    CDrawer *replayDrawer;

    QScrollBar *verScrl;

    GLuint list;
    GLuint object;

    double viewportWidth;
    QSizeF viewportSize;
    QSizeF stadiumSize;

    QRectF field;
    QRectF fieldCenter;
    QRectF leftPenalty;
    QRectF rightPenalty;

    double WH_RATIO;
    double DEG2RAD;

    void drawField();
    void drawRobot(double x, double y, double ang, int ID, int comID, QColor color,QString str="", bool newRobots=false);
    void drawText(double x, double y, QString text, QColor color = QColor("black"), int size = 12);
    GLuint drawArc(double centerX, double centerY, double radius, int start, int end, QColor color = QColor(255, 255, 255), bool fill = false, bool fullFill = false);
    GLuint drawRect(double topLeftX, double topLeftY, double buttomRightX, double buttomRightY, QColor color = QColor(255, 255, 255), bool fill = false);
    GLuint drawLine(double x1, double y1, double x2, double y2, QColor color = QColor(255, 255, 255));
    GLuint drawPoint(double x, double y, QColor color = QColor(0, 0, 0));



    QColor stadiumGreen;
    QColor fieldGreen;
    QPointF mousePos;

};


/*
 * Copyright (C) 2009 Matteo Bertozzi.
 *
 * This file is part of Thasis.
 *
 * Thasis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Thasis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Thasis.  If not, see <http://www.gnu.org/licenses/>.
 */


class THLineGraphData {
public:
    THLineGraphData(double xValue, double yValue) {
        m_xValue = xValue;
        m_yValue = yValue;
    }

    ~THLineGraphData() {
    }

    double x (void) const { return(m_xValue); }
    double y (void) const { return(m_yValue); }

    void setX (double xValue) { m_xValue = xValue; }
    void setY (double yValue) { m_yValue = yValue; }

private:
    double m_xValue;
    double m_yValue;
};

class THLineGraphPrivate;

class THLineGraph : public QGLWidget {
    Q_OBJECT

public:
    THLineGraph (QWidget *parent = 0);
    ~THLineGraph();

    void append (const THLineGraphData& data, int CH=0);
    void insert (int index, const THLineGraphData& data, int CH=0);
    void delfirst (int num=1, int CH=0);
    void delall(int CH=0);
    int datalen(int CH=0);
    void appendCH(QList<THLineGraphData> *dataCH, QColor *color,float *startX,float *endX,float *startY,float *endY,bool *active);

    bool save (const QString& fileName, const QSize& size);

    void setGraphBorder (const QColor& graphBorder);
    void setGraphBackground (const QColor& graphBg);
    void setTextColor (const QColor& textColor);
    void setBackground (const QColor& bgColor);
    void setBorder (const QColor& bgBorder);
    void setMaxDataTime(int maxDataTime);

protected:
    void resizeEvent (QResizeEvent *event);
    void paintEvent (QPaintEvent *event);

private:
    THLineGraphPrivate *d;
};

class CTrainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CTrainWidget(QWidget *parent = 0);
    QPushButton* start;
    QPushButton* end;
private:
    QGridLayout* lOut;
};

class CKeyboardWidget : public QDialog
{
    Q_OBJECT

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void closeEvent(QEvent*);
public:
    CKeyboardWidget(CAgent **_agents, QWidget *parent = 0);
    ~CKeyboardWidget();
    void takeAction(EActionType action);

    QTimer *jsTimer;

public slots:
    void setCurrentAgent(int num);
    void translateJs();
    void kickSpeedChanged();
    void forward(){takeAction(_FORWARD);}
    void backward(){takeAction(_BACKWARD);}
    void left(){takeAction(_LEFT);}
    void right(){takeAction(_RIGHT);}
    void turnCW(){takeAction(_TURN_CW);}
    void turnCCW(){takeAction(_TURN_CCW);}
    void kick(){takeAction(_KICK);}
    void chip(){takeAction(_CHIP);}
    void roller(){takeAction(_ROLLER);}
    void click_released(){takeAction(_STOP_NAVIGATION); takeAction(_STOP_OTHER);}
signals:
    void closeSignal(bool);

private:

    bool isKeyboardFree;
    CAgent **agents;

    short int currentAgent;
    int speed;
    short int kickSpeed;

    CWorldModel* wm;

    bool bb1, bb2, bb3, bb4;
    bool bbup, bbbottom, bbright, bbleft;
    bool bkick, bspin;

    bool bshift, bctrl;

    QComboBox *agentsCombo;
    QLabel* agentsLbl;
    QLabel* speedLbl;
    QLabel* kickSpeedLbl;
    QLabel* joystickLbl;

    QCheckBox* joystickChk;

    QLineEdit *speedLineedit;
    QLineEdit *kickSpeedLineedit;

    QPushButton *upBtn;
    QPushButton *downBtn;
    QPushButton *rightBtn;
    QPushButton *leftBtn;
    QPushButton *cwBtn;
    QPushButton *ccwBtn;
    QPushButton *kickBtn;
    QPushButton *chipBtn;
    QPushButton *rollerBtn;
    QPushButton *haltBtn;

    QGridLayout *hDLayout;
    QGridLayout *selectionLayout;
    QGridLayout *actionsLayout;
    QGridLayout *vDLayout;
    QGridLayout *navigationLayout;

};

class CAgentsStatusWidget : public QWidget
{
    Q_OBJECT
public:
    CCoach* coach;
    CWorldModel *wm;
    CAgent **agents;
    CAgentsStatusWidget(CCoach *_coach,CWorldModel *_wm,CAgent **_agents);
    ~CAgentsStatusWidget();
    void update();
    QLineEdit *refCommand;
    QLabel **l;
    QLabel **v;
    QLabel **r;
    QLabel *ballInfo;
public slots:
    void updateBallInfo(QString _ballInfo);
};

class CSimulatorControllerWidget : public QWidget
{
    Q_OBJECT
public:
    CSimulatorControllerWidget(CWorldModel *);
    ~CSimulatorControllerWidget();
private:
    QLabel *lblFPS , *lblDETStp , *lblteams , *lblagents, *lblTeam, *lblBall, *lbAgentData, *lbAgentPos;
    QPushButton *btnFPS , *btnDETStp , *btnlocate, *btnBallLocate;
    QCheckBox *OGLstate;
    QLineEdit *txtFPS , *txtDETStp;
    QComboBox *cmbteams , *cmbagents;
public slots:
    void SetSimulatorFPS();
    void SetSimulatorOGLState();
    void SetSimulatorOpenDETimeStep();
    void locateRobot();
    void locateBall();
    void locateBall(Vector2D _newPos);
};

class CLoggerWidget : public QDialog
{
    Q_OBJECT
public:
    CLoggerWidget();
    ~CLoggerWidget();
    void reBuildWidget();
    bool state();
    bool isLoggerWorking;
private:
    QFileDialog *dialog;
    QPushButton *btnPlay , *btnBrowse , *btnNextFrame , *btnPreviousFrame;
    QComboBox *cmbList;
    QLineEdit *txtFPS;
    QSlider *slider;
    QTimer *timer;
    QTextEdit *debugTexts;
    QLabel *lblFPS;
    QLabel *lblTime , *lblRefCmd;
    QPushButton *btnClear;
    QStringList fileNames;
    QCheckBox *chbxDraws , *chbxDebugs;
    QCheckBox *chbxDebug[20];
    QMap<int16_t,QColor> tcolor;
    bool pause;
    qint16 loggerFPS;
    long type;
    long type1;
    int fBfStep;
public slots:
    void browseDialog();
    void playLog();
    void goNextFrame();
    void goPreviousFrame();
    void seekChange(int);
    void cursorIncrement();
    void clearDebugTexts();
    void playThisFile(int);
    void showHideDraws(bool);
    void showHideDebugs(bool);
    void debugTypeChanged();
    void replayFPSChanged();
};


class CLogTagWidget : public QDialog
{
    Q_OBJECT
public:
    CLogTagWidget(QWidget*);
    ~CLogTagWidget();
private:
    QLineEdit *WhoLogs, *WhereLogs, *DescriptionLogs, *teamNameLogs , *TagsInThisLog;
    QLabel *WhoLogsl, *WhereLogsl, *DescriptionLogsl, *teamNameLogsl, *TagsInThisLogl;
    QPushButton *StartLog;
public slots:
    void StartLogfunc();
    QString getWhoLogs();
    QString getWhereLogs();
    QString getDescriptionLogs();
    QString getTagsInThisLog();
};





class MyTreeView : public QTreeView
{
    Q_OBJECT
public:    
    MyTreeView(QTextEdit* _txt, QWidget *parent);
protected:    
    QTextEdit* txt;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
signals:
    void fileChanged(QString filename);
};

class MyTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    MyTextEdit(QWidget* parent=0);
    bool ctrl;
protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
signals:
    void modified();
};

//class CPlaysWidget : public QWidget
//{
//    Q_OBJECT
//public:
//	CPlaysWidget(bool _treeView=true);
//    void addPlay(QStandardItem *parentItem, Play *play);
//    void initialize();
//    void update();
//    void reloadCoach();
//    CCoach* coach;
//    QList<Play*> lastPlays;
//private:
//	bool treeView;
//    MyTreeView* t;
//    QStandardItemModel* m;
//    QLabel* lblPlay, *lblCondition;
//    QPushButton *btnSave, *btnLoad;
//    MyTextEdit* txt;
//    QString playfilename;
//public slots:
//    void savePlay();
//    void loadPlay();
//    void playModified();
//    void playFileChanged(QString filename);
//};

//class CNewRectDialog : public QDialog
//{
//    Q_OBJECT
//protected:
//    QPushButton* okBtn;
//    QPushButton* cancelBtn;
//    QLineEdit* nameL;
//public:
//    QString name;
//    CNewRectDialog();
//public slots:
//    void saveClicked();
//    void cancelClicked();
//};

//class CRectsWidget : public QWidget
//{
//    Q_OBJECT
//public:
//    CRectsWidget(CMonitorWidget* _monitor);
//    void update();
//private:
//    CMonitorWidget* monitor;
//    Vector2D startPoint;
//    Vector2D endPoint;
//    Vector2D topleft;
//    Rect2D selectionRect;
//    QPushButton* saveBtn;
//    CNewRectDialog* rectNameDlg;
//    QFile* rectsFile;

//public slots:
//    void startSelecting(Vector2D _point, int b);
//    void selecting(Vector2D _point);
//    void endSelecting();
//    void saveSlt();
//};

#ifdef DISTURBANCE_MATRIX
class CLearnEWidget : public QWidget {
    Q_OBJECT
public:
    CLearnEWidget(CAgent** _agents);
    ~CLearnEWidget();
private:
    QPushButton *btnGo,*btnStop;
    QPushButton *btnPrintTrainSet,*btnClearTrainSet,*btnTrain,*btnEstimate;
    QPushButton *btnLoadTrainSet,*btnSaveTrainSet;
    QPushButton *btnGenerateRandomTS;
    QLineEdit *leID,*leVf,*leVn,*leW;
    QLineEdit *leWaitFrames,*leRunFrames,*leSamples;
    QTextEdit *teLog;
    CAgent** agents;
    CSkillGotoPointAvoid *gotopoint;
    int runingState;
    QList< Matrix > matList;
    QList< QPair<Matrix, Matrix> > trainSet;
    int i,tsn;
    TMLP *mlp;
    bool trained,randomTSG;
public slots:
    void btnGoPressed();
    void btnStopPressed();
    void btnPrintTSPressed();
    void btnClearTSPressed();
    void btnTrainPressed();
    void btnEstimatePressed();
    void btnLoadTSPressed();
    void btnSaveTSPressed();
    void btnGenerateRandomTSPressed();
    bool execute();
};
#endif

//class CKnowledgeVarsWidget : public QWidget {
//    Q_OBJECT
//public:
//    CKnowledgeVarsWidget();
//    void update();
//private:
//    QLabel* label;
//};

//class CBehavioursWidget : public QWidget {
//    Q_OBJECT
//public:
//    CBehavioursWidget();
//    ~CBehavioursWidget();
//    void update();
//private:
//    QTextEdit* txt, *dbg;
//    QComboBox* cmb;
//    QPushButton* btn;
//    QString dbgString;
//public slots:
//    void load();
//    void save();
//    void changed();
//};


//TODO:  right a widget that creates different velocities and checks the robot response; to find out what is the function between desired and actual velocity of robot

//class CConditionCheckerWidget : public QWidget
//{
//	Q_OBJECT
//public:
//	CConditionCheckerWidget();
//	~CConditionCheckerWidget();
//	QLineEdit* conditionTxt;
//	QLabel* conditionLbl;
//	QPushButton* btn;
//public slots:
//	void check();
//};

#endif
