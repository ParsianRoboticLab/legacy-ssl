#include "widgets.h"
#include "base.h"
#include "worldmodel.h"
#include "VarTypes/VarTypes.h"
#include "coach.h"
#include "gamelogger.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QValidator>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPainter>
#include <QTimer>
#include <QSizePolicy>
#include <QDoubleValidator>
#include <QRadioButton>
#include <QButtonGroup>
#include <QtGui>
#include <QtOpenGL>
#include <QDebug>
#include <QPaintEvent>
#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QDateTime>
#include <math.h>
#include <cmath>
#include <QFont>
#include <algorithm>
#include "simulation/simulator.h"

#include<QMenu>
#include<QAction>


CTabDockWidget::CTabDockWidget(QWidget* parent , bool autoHide)
    : QDockWidget(parent) , autoHideBool(autoHide)
{
    w = new QWidget(this);
    tabs = new QTabWidget(w);
    QGridLayout *layout = new QGridLayout(w);
    tabs->setTabPosition(QTabWidget::West);
    if( autoHide ){
        isHide = hideable = false;
        btnAutoHide = new QPushButton(this);
        hideShowTimer = new QTimer(this);
        btnAutoHide->setText("AutoHide");
        hideShowTimer->setInterval(50);
        connect(hideShowTimer , SIGNAL(timeout()) , this , SLOT(autoHide()));
        connect(btnAutoHide , SIGNAL(clicked()) , this , SLOT(setHideable()));
        layout->addWidget(btnAutoHide , 0 , 0);
        layout->addWidget(tabs , 1 , 0);
    }
    else
        layout->addWidget(tabs,0,0);
    w->setLayout(layout);
    setWidget(w);
}


CTabDockWidget::~CTabDockWidget()
{
    if( autoHideBool ){
        delete hideShowTimer;
        delete btnAutoHide;
    }
    delete tabs;
    delete w;
}

void CTabDockWidget::closeEvent(QEvent*)
{
    emit closeSignal(false);
}

void CTabDockWidget::autoHide(){
    if( !underMouse() && isHide == false ){
        if( tabs->width() > 50 ){
            tabs->setMaximumSize(tabs->width()-40 , height());
            setMaximumSize(tabs->width()+30,height());
        }
        else
            isHide = true;
    }
    else if( underMouse() && isHide ){
        if( width() < 500 ){
            setMaximumSize(tabs->width()+70,height());
            tabs->setMaximumSize(tabs->width()+40,height());
        }
        else
            isHide = false;
    }
}

CTrainWidget::CTrainWidget(QWidget *parent) :
    QWidget(parent)
{
    setGeometry(1400,530,400,400);
    setWindowTitle("Train");

    start = new QPushButton("Start");
    end = new QPushButton("End");

    lOut = new QGridLayout();
    this->setLayout(lOut);

    lOut->addWidget(start, 0, 0);
    lOut->addWidget(end, 0, 1);
}

void CTabDockWidget::setHideable(){
    if( hideable ){
        btnAutoHide->setText("AutoHide");
        hideable = false;
        hideShowTimer->stop();
    }
    else{
        btnAutoHide->setText("AlwaysShow");
        hideable = true;
        hideShowTimer->start();
    }
}

CStatusWidget::CStatusWidget(CStatusPrinter* _statusPrinter , CAgent **_agents , CCoach *_coach)
{
    info = new CInfoWidget(0);
    gameInfo = new CGameInfoWidget();
    agentsWidget = new CAgentsWidget(_agents);
    agentsWidget->setCoach(_coach);
    statusPrinter = _statusPrinter;
    logTime.start();

    this->setAllowedAreas(Qt::BottomDockWidgetArea);
    this->setFeatures(QDockWidget::NoDockWidgetFeatures);
    statusText = new QTextEdit(this);
    //statusText->setReadOnly(true);
    titleLbl = new QLabel(tr("Messages"));
    statusText->setMaximumWidth(1000);
    //    statusText->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QWidget* w = new QWidget(this);

    statusText->setFixedSize(500, 200);
    gameInfo->setFixedSize(400, 200);

    QGridLayout* layout = new QGridLayout(w);
    layout->addWidget(statusText, 0, 0);
    layout->addWidget(gameInfo, 0, 1);
    layout->addWidget(agentsWidget , 0 , 2);
    layout->setAlignment(statusText, Qt::AlignLeft);
    layout->setAlignment(gameInfo, Qt::AlignLeft);
    layout->setAlignment(agentsWidget , Qt::AlignLeft);
    layout->setColumnMinimumWidth(0, 500);
    statusText->resize(1000, 200);
    w->setLayout(layout);

    this->setSizeIncrement(this->width(), 100);
    //    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setWidget(w);
}

CStatusWidget::~CStatusWidget()
{
    delete info;
    delete statusText;
    delete titleLbl;
}

void CStatusWidget::write(QString str, QColor color)
{
    if( statusText->textCursor().blockNumber() > 1000 )
        statusText->clear();

    statusText->setTextColor(color);
    statusText->append(QString::number(logTime.elapsed()) + " : " + str);
    statusText->setTextColor(QColor("black"));
}

void CStatusWidget::update()
{
    //info->update();
    CStatusText text;
    while(!statusPrinter->textBuffer.isEmpty())
    {
        text = statusPrinter->textBuffer.dequeue();
        write(text.text, text.color);
    }
}

void CStatusWidget::closeEvent(QEvent*)
{
    emit closeSignal(false);
}

CGameInfoWidget::CGameInfoWidget()
{

    QGridLayout *l = new QGridLayout(this);
    QStringList props;
    props << "Formation" << "Play" << "State" << "Mode" << "Loop" << "Max Loop" << "Goalie";
    table = new QTableWidget(props.length(),1,this);
    for(int i=0; i<props.length(); i++)
    {
        items[props[i]] = new QTableWidgetItem("");
        table->setItem(i,0,items[props[i]]);
        items[props[i]]->setFlags(Qt::ItemIsEnabled);
        table->setRowHeight(i, 22);
    }
    table->setVerticalHeaderLabels(props);
    table->setColumnWidth(0, 200);
    l->addWidget(table,0,0);
    l->setAlignment(table, Qt::AlignLeft);
    //	technical = new QComboBox(this);
    //	for (int i=0;i<CKnowledge::TechnicalModes.count();i++)
    //		technical->addItem(CKnowledge::TechnicalModes[i]);
    //	l->addWidget(technical,0,1);
#ifndef NO_JS
    joystick = new QCheckBox("Joystick");
    joystick->setChecked(false);
    l->addWidget(joystick, 0,2);
#endif
    //	connect(technical, SIGNAL(currentIndexChanged(QString)), this, SLOT(technicalChanged(QString)));
}

void CGameInfoWidget::technicalChanged(QString t)
{
    knowledge->setTechnicalMode(t);
}

CGameInfoWidget::~CGameInfoWidget()
{
    delete table;
}

void CGameInfoWidget::update()
{
    QStringList s1, s2;
    for (int i=0;i<knowledge->currentFormation.first.count();i++)
    {
        s1.append(QString("%1").arg(knowledge->currentFormation.first.at(i)));
    }
    for (int i=0;i<knowledge->currentFormation.second.count();i++)
    {
        s2.append(QString("%1").arg(knowledge->currentFormation.second.at(i)));
    }
    items["Formation"]->setText(s1.join(",")+QString(";")+s2.join(","));
    items["Play"]->setText(knowledge->executingPlays.join(":"));
    items["Mode"]->setText(knowledge->stateToString(knowledge->getGameMode()));
    items["State"]->setText(knowledge->stateToString(knowledge->getGameState()));
    items["Loop"]->setText(QString("%1 ms").arg(QString::number(knowledge->loopTime*1000.0, 'f', 3)));
    //	items["Max Loop"]->setText(QString("%1 ms").arg(QString::number(knowledge->maxLoopTime*1000.0, 'f', 3)));
    items["Max Loop"]->setText(QString("%1 ms").arg(QString::number(knowledge->visionProcessTime*1000.0, 'f', 3)));
    if (knowledge->goalie != NULL)
        items["Goalie"]->setText(QString("%1").arg(knowledge->goalie->id()));
    else
        items["Goalie"]->setText(QString("-"));
}


CSkillWidget::CSkillWidget(CAgent **_agents, QWidget *parent)
    : QWidget(parent)
{
    agents = _agents;
    mousePos.assign(0.0, 0.0);
    //        this->setFixedSize(200, 300);
    execButton = new QPushButton("Execute",this);
    stopButton = new QPushButton("Stop",this);
    stopExecution = true;
    agentsCombo = new QComboBox(this);
    for(int i = 0; i< _NUM_PLAYERS; i++){
        agentsCombo->addItem(QString("%1").arg(i));
    }
    currentAgent = agentsCombo->currentIndex();
    skillsCombo = new QComboBox(this);
    pagesWidget = new QStackedWidget(this);
    for (int i=0;i<CSkills::skillsCount();i++)
    {
        if (CSkills::skill(i)->level()==1)
        {
            CSkillConfigWidget* page = CSkills::skill(i)->generateConfigWidget(this);
            if (page!=NULL)
            {
                page->mousePos = &mousePos;
                skillsCombo->addItem(CSkills::skill(i)->getName());
                pages.append(page);
                pagesWidget->addWidget(page);
            }
        }
    }
    skillPrgrs = new QProgressBar(this);
    skillPrgrs->reset();
    skillPrgrs->setMinimum(0);
    skillPrgrs->setMaximum(100);
    QVBoxLayout *verticalLayout = new QVBoxLayout;
    verticalLayout->addWidget(agentsCombo);
    verticalLayout->addWidget(skillsCombo);
    verticalLayout->addWidget(pagesWidget);
    verticalLayout->addWidget(skillPrgrs);
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(execButton);
    buttonsLayout->addWidget(stopButton);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(verticalLayout);
    //    mainLayout->addStretch(1);
    //	mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);


    connect(skillsCombo, SIGNAL(activated(int)), this, SLOT(setCurrentPage(int)));
    connect(execButton, SIGNAL(clicked()), this, SLOT(startSkill()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopSkill()));
    connect(agentsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentAgent(int)));
    executing = false;

    for (int i=0;i<skillsCombo->count();i++)
        if (skillsCombo->itemText(i) == QString("gotopointavoid"))
        {
            skillsCombo->setCurrentIndex(i);
            setCurrentPage(i);
            break;
        }
    interceptInitialPoint.invalidate();
    timeReported = false;
}

CSkillWidget::~CSkillWidget()
{
    delete execButton;
    delete stopButton;
    delete agentsCombo;
    delete skillsCombo;

    int ii=pagesWidget->count();
    QWidget *tW;
    while(ii>0)
    {
        tW =  pagesWidget->widget(ii-1);
        pagesWidget->removeWidget(tW);
        delete tW;
        ii=pagesWidget->count();
    }
    delete pagesWidget;
    delete skillPrgrs;
}

bool CSkillWidget::executeSkill()
{    
    if (!executing) return false;
    if(stopExecution) return false;
    CSkill *s = ((CSkillConfigWidget*) pagesWidget->currentWidget())->skill;
    if (s->getName()=="kickonetouch")
    {
        //        if (s->getAgent()->self()->ballComingSpeed() > 0.2)
        //        {
        s->execute();
        //        }
        //        else
        //            s->getAgent()->waitHere();
    }
    else if (s->getName()=="intercept")
    {
        if (((CSkillConfigWidget*) pagesWidget->currentWidget())->checkBoxs[1]->isChecked())
        {

        }
        else
        {
            CSkillIntercept* g = static_cast<CSkillIntercept*> (s);
            double vx = ((CSkillConfigWidget*) pagesWidget->currentWidget())->lineEdits[3]->text().toDouble();
            double vy = ((CSkillConfigWidget*) pagesWidget->currentWidget())->lineEdits[4]->text().toDouble();
            if ((s->getAgent()->pos()-g->getTarget()->pos).length() < 1.0)
            {
                if (interceptInitialPoint.valid())
                {
                    g->getTarget()->pos += conf()->Common_Command_Interval()*Vector2D(vx, vy)/1000.0;
                    g->getTarget()->vel.assign(vx, vy);
                }
                else {
                    interceptInitialPoint = g->getTarget()->pos;
                }
                draw(g->getTarget()->pos, 1, "blue");
            }
            else
            {
                if (interceptInitialPoint.valid())
                {
                    g->getTarget()->pos = interceptInitialPoint;
                }
                g->getTarget()->vel.assign(0.0, 0.0);
            }
        }
        s->execute();
    }
    else s->execute();
    if (s->getName() == "gotopointavoid")
    {
        if (simulator->isReached(static_cast<CSkillGotoPointAvoid*>(s)->getFinalPos(), static_cast<CSkillGotoPointAvoid*>(s)->getFinalDir() , s->getAgent()->id()))
        {
            if (!timeReported)
            {
                double timePast = (CProfiler::getTime() - static_cast<CSkillGotoPointAvoid*>(s)->timeStarted);
                double timeEstimated = static_cast<CSkillGotoPointAvoid*>(s)->timeEstimated;
                debug(QString("expected time=%1; real time=%2").arg(timeEstimated).arg(timePast), D_ERROR);
            }
            timeReported = true;
        }
        else timeReported = false;
    }
    else timeReported = false;
    skillPrgrs->setValue(((CSkillConfigWidget*) pagesWidget->currentWidget())->skill->progress()*100);
    draw(mousePos, 1, QColor("purple"));
    //    draw (QString("rrttime=%1").arg(QString::number(knowledge->rrttime, 'f', 4)), Vector2D(-1.5,1.5), "red", 24);
    return true;
}

void CSkillWidget::startSkill()
{
    executing = true;
    stopExecution = false;
    skillPrgrs->setEnabled(true);
    skillPrgrs->reset();
    execButton->setEnabled(false);
    ((CSkillConfigWidget*) pagesWidget->currentWidget())->skill->generateFromConfig(agents[currentAgent]);
    interceptInitialPoint.invalidate();
}

void CSkillWidget::stopSkill()
{
    executing = false;
    stopExecution = true;
    skillPrgrs->setEnabled(false);
    skillPrgrs->reset();
    execButton->setEnabled(true);
    agents[currentAgent]->waitHere();
}

void CSkillWidget::setCurrentAgent(int num)
{
    stopSkill();
    currentAgent = num;
}

void CSkillWidget::setCurrentPage(int num)
{
    stopSkill();
    pagesWidget->setCurrentIndex(num);
}

void CSkillWidget::getMousePos(Vector2D _pos)
{
    mousePos = _pos;
    ((CSkillConfigWidget*) pagesWidget->currentWidget())->skill->generateFromConfig(agents[currentAgent]);
}


//----------------------------------------------------------------------------------------
/////////////////////////////////////////

CProfilerWidget::CProfilerWidget(CAgent **_agents, QWidget *parent) : QWidget(parent) {
    agents = _agents;

    execButton    = new QPushButton("Execute" , this);
    stopButton    = new QPushButton("Stop"    , this);
    addButton     = new QPushButton("ADD"     , this);
    saveButton    = new QPushButton("SAVE"    , this);
    loadButton    = new QPushButton("LOAD"    , this);
    applyButton   = new QPushButton("apply"   , this);
    refreshButton = new QPushButton("refresh" , this);
    clearButton   = new QPushButton("CLEAR"   , this);
    insertButton  = new QPushButton("Insert"  , this);

    agentsCombo = new QComboBox(this);
    for(int i = 0; i< _NUM_PLAYERS; i++){
        agentsCombo->addItem(QString("%1").arg(i));
    }

    currentAgent = agentsCombo->currentIndex();

    skillsCombo  = new QComboBox(this);
    skillsCombo->addItem("Kick");
    skillsCombo->addItem("Chip");
    skillsCombo->addItem("SpinKick");
    skillsCombo->addItem("SpinChip");

    QLabel *kickSpeedLable = new QLabel("Kick Speed :", this);
    kickSpeed = new QLineEdit(this);
    kickSpeed->setFixedSize(100,20);

    QLabel *realSpeedLable = new QLabel("Real Speed :", this);
    realSpeed = new QLineEdit(this);
    realSpeed->setFixedSize(100,20);

    fileDirLable = new QLabel("File Name :"    , this);
    theMeanLable      = new QLabel("The Mean is : " , this);

    QVBoxLayout *mainLayout   = new QVBoxLayout;
    QHBoxLayout *stackLayout  = new QHBoxLayout;
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QVBoxLayout *comboLayout  = new QVBoxLayout;
    QHBoxLayout *jsonLayout   = new QHBoxLayout;
    QVBoxLayout *editorLayout = new QVBoxLayout;

    //    stackLayout->addWidget(saveButton);
    stackLayout->addWidget(addButton);
    stackLayout->addWidget(kickSpeedLable);
    stackLayout->addWidget(kickSpeed);
    stackLayout->addWidget(realSpeedLable);
    stackLayout->addWidget(realSpeed);
    stackLayout->setAlignment(Qt::AlignTop);

    editor = new QTextEdit(this);

    jsonLayout->addWidget(saveButton);
    jsonLayout->addWidget(loadButton);
    jsonLayout->addLayout(editorLayout);
    //    jsonLayout->addWidget(editor);

    editorLayout->addWidget(fileDirLable);
    editorLayout->addWidget(editor);
    editorLayout->addWidget(applyButton);
    editorLayout->addWidget(refreshButton);
    editorLayout->addWidget(clearButton);

    comboLayout->addWidget(agentsCombo);
    comboLayout->addWidget(skillsCombo);
    comboLayout->setAlignment(Qt::AlignTop);

    buttonLayout->addWidget(execButton);
    buttonLayout->addWidget(stopButton);

    mainLayout->addLayout(comboLayout);
    mainLayout->addLayout(stackLayout);
    mainLayout->addWidget(theMeanLable);
    mainLayout->addWidget(insertButton);
    mainLayout->addLayout(jsonLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    connect(saveButton   , SIGNAL(clicked())  , this, SLOT(slt_save()));
    connect(loadButton   , SIGNAL(clicked())  , this, SLOT(slt_load()));
    connect(addButton    , SIGNAL(clicked())  , this, SLOT(slt_add()));
    connect(execButton   , SIGNAL(clicked())  , this, SLOT(slt_exec()));
    connect(stopButton   , SIGNAL(clicked())  , this, SLOT(slt_stop()));
    connect(applyButton  , SIGNAL(clicked())  , this, SLOT(slt_apply()));
    connect(refreshButton, SIGNAL(clicked())  , this, SLOT(slt_refresh()));
    connect(clearButton  , SIGNAL(clicked())  , this, SLOT(slt_clear()));
    connect(insertButton , SIGNAL(clicked())  , this, SLOT(slt_insert()));

    connect ( realSpeed, SIGNAL( returnPressed() ), this, SLOT( slt_add() ) );

    QAction* InsertAct = new QAction(this);
    InsertAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    addAction(InsertAct);
    connect(InsertAct, SIGNAL(triggered()), this, SLOT(slt_insert()));

    connect(agentsCombo  , SIGNAL(currentIndexChanged(int)),
            this         , SLOT  (slt_setCurrentAgent(int)));

    connect(kickSpeed    , SIGNAL(textEdited(QString)),
            this         , SLOT  (slt_modeEdited(QString)));
    connect(agentsCombo  , SIGNAL(currentIndexChanged(int)),
            this         , SLOT  (slt_modeEdited(int)));
    connect(skillsCombo  , SIGNAL(currentIndexChanged(QString)),
            this         , SLOT  (slt_modeEdited(QString)));

    executing = false;
    stopExecuting = true;

    currentAgent = agentsCombo->currentIndex();
}

CProfilerWidget::~CProfilerWidget() {
    delete agentsCombo;
    delete skillsCombo;
    delete addButton;
    delete saveButton;
    delete stopButton;
    delete loadButton;
    delete execButton;
    delete applyButton;
    delete kickSpeed;
    delete realSpeed;
    delete insertButton;
}

void CProfilerWidget::slt_insert() {
    ProfileMode tempMode = getMode(skillsCombo->currentIndex());
    if(tempMode == PCHIP || tempMode == SCHIP)
        knowledge->profiler->insertRecord(tempMode,
                                          kickSpeed->text().toInt(),
                                          theMean,
                                          currentAgent);
    else
        knowledge->profiler->insertRecord(tempMode,
                                          kickSpeed->text().toInt(),
                                          ballSpeedAddedList,
                                          currentAgent);
    kickSpeed->setText("");
    realSpeed->setText("");
    theMeanLable->setText(QString("The Mean : "));
    kickSpeed->setFocus();


}

void CProfilerWidget::slt_modeEdited(QString) {
    realSpeed->setText("");
    ballSpeedAddedList.clear();
    theMean = 0.0;
    theMeanLable->setText(QString("The Mean : %1").arg(theMean));
}

void CProfilerWidget::slt_modeEdited(int) {
    realSpeed->setText("");
    ballSpeedAddedList.clear();
    theMean = 0.0;
    theMeanLable->setText(QString("The Mean : %1").arg(theMean));
}

void CProfilerWidget::slt_save() {
    int reply;
    QString tempDir;

    if(fileDir.length() > 3) {
        reply = QMessageBox::question(this,
                                      "Save Profiler",
                                      "Do U wanna save ?",
                                      QMessageBox::Save|
                                      QMessageBox::Yes|
                                      QMessageBox::No);
    }
    else {
        reply = QMessageBox::question(this,
                                      "Save Profiler",
                                      "Do U wanna save ?",
                                      QMessageBox::Yes|
                                      QMessageBox::No);

    }
    if(reply == QMessageBox::No) {
        return;
    }
    else if(reply == QMessageBox::Save || fileDir.length() <= 3) {
        tempDir = QFileDialog::getSaveFileName(this,
                                               "Save The Profiler",
                                               "",
                                               "*.json");
    }
    else if(reply == QMessageBox::Yes) {
        tempDir = fileDir;
    }

    if(tempDir.length() > 3) {
        fileDir = tempDir;
        knowledge->profiler->save(JSON, tempDir);

        if(tempDir.length() > 30) {
            QString temp("...");
            tempDir.remove(0, tempDir.length() - 30);
            tempDir = temp + tempDir;
        }

        fileDirLable->setText(QString("File Name: ") + tempDir);
        fileDirLable->setFont(QFont("mono"));

        slt_refresh();
    }
}

void CProfilerWidget::slt_load() {

    int reply;
    QString tempDir;

    if(fileDir.length() > 3) {
        reply = QMessageBox::question(this,
                                      "Load Profiler",
                                      "Do U wanna load ?",
                                      QMessageBox::Open|
                                      QMessageBox::Yes|
                                      QMessageBox::No);
    }
    else {
        reply = QMessageBox::question(this,
                                      "Load Profiler",
                                      "Do U wanna load ?",
                                      QMessageBox::Yes|
                                      QMessageBox::No);

    }
    if(reply == QMessageBox::No) {
        return;
    }
    else if(reply == QMessageBox::Open || fileDir.length() <= 3) {
        tempDir = QFileDialog::getOpenFileName(this,
                                               "Load The Profiler",
                                               "",
                                               tr("JSON Files (*.json);;All Files (*.*)"));
    }
    else if(reply == QMessageBox::Yes) {
        tempDir = fileDir;
    }

    if(tempDir.length() > 3) {
        fileDir = tempDir;
        knowledge->profiler->load(JSON, tempDir);

        if(tempDir.length() > 30) {
            QString temp("...");
            tempDir.remove(0, tempDir.length() - 30);
            tempDir = temp + tempDir;
        }

        fileDirLable->setText(QString("File Name: ") + tempDir);
        fileDirLable->setFont(QFont("mono"));

        slt_refresh();
    }
}

void CProfilerWidget::slt_add() {
    //    ballSpeedAddedList.append(realSpeed->text().toDouble());
    //    double temp;
    //    for (size_t i = 0; i < ballSpeedAddedList.size();i++) {
    //        temp += ballSpeedAddedList.at(i);
    //    }
    //    theMean = temp/ballSpeedAddedList.size();

    theMean*=ballSpeedAddedList.size();
    ballSpeedAddedList.append(realSpeed->text().toDouble());
    theMean+=ballSpeedAddedList.last();
    theMean/=ballSpeedAddedList.size();
    theMeanLable->setText(QString("The Mean : %1").arg(theMean));
    realSpeed->setText("");
}

void CProfilerWidget::slt_exec() {
    executing     = true;
    stopExecuting = false;
    execButton->setEnabled(false);
}

void CProfilerWidget::slt_stop() {
    executing     = false;
    stopExecuting = true;
    execButton->setEnabled(true);

}

void CProfilerWidget::slt_apply() {
    int tempInput = QMessageBox::question(this, "Are You Sure ?", "This Operation Can't Undo FOREVER",
                                          QMessageBox::No, QMessageBox::Yes);
    if(tempInput == QMessageBox::Yes) {
        knowledge->profiler->setFile(editor->toPlainText().toAscii(), fileDir);
    }
    if(fileDir.length() > 3) {
        knowledge->profiler->load(JSON, fileDir);
    }
    else {
        QMessageBox::warning(this, "File Was Not Found", "You Should Load a file First", QMessageBox::Ok);
    }
}

void CProfilerWidget::slt_refresh() {
    editor->setText(knowledge->profiler->getFile(fileDir));
    knowledge->profiler->refresh();
}

void CProfilerWidget::slt_clear() {

    int tempInput = QMessageBox::question(this, "Are You Sure ?", "This Operation Can't Undo FOREVER",
                                          QMessageBox::No, QMessageBox::Yes);
    if(tempInput == QMessageBox::Yes) {
        editor->setText("");
        knowledge->profiler->setFile(QByteArray(), fileDir);
        fileDir = "";
        fileDirLable->setText("File Name: ");
        delete knowledge->profiler;
        knowledge->profiler = new CNewProfiler();
    }
}

void CProfilerWidget::slt_setCurrentAgent(int num) {
    slt_stop();
    currentAgent = num;
}

ProfileMode CProfilerWidget::getMode(int _index) {
    switch (_index) {
    case 0:
        return PKICK;
        break;
    case 1:
        return PCHIP;
        break;

    case 2:
        return SKICK;
        break;
    case 3:
        return SCHIP;
        break;
    }
}

void CProfilerWidget::execute() {
    if (!executing) {
        return;
    }
    if (stopExecuting) {
        return;
    }
    agents[currentAgent]->setRobotVel(0.2, 0, 0);
    if (skillsCombo->currentIndex()) {
        agents[currentAgent]->setChip(kickSpeed->text().toInt());
    }
    else {
        agents[currentAgent]->setKick(kickSpeed->text().toInt());
    }
    startRecord();
}

void CProfilerWidget::startRecord() {
    if (wm->ball->vel.length() > 0.2)
        guessBallSpeedList.append(wm->ball->vel.length());
    if (guessBallSpeedList.size() > 30) {
        guessBallSpeedList.pop_front();
    }
    if (guessBallSpeedList.size() > 20) {
        for (size_t i = 0;i < guessBallSpeedList.size();i++) {
            if(guessBallSpeedList.at(guessBallSpeedList.size()/2) < guessBallSpeedList.at(i)) {
                return;
            }
        }
        realSpeed->setText(QString("%1").arg(guessBallSpeedList[guessBallSpeedList.size()/2]));
    }
}

/////////////////////PLAY OFF////////////////////////

CPlayOffWidget::CPlayOffWidget(CLoadPlayOffJson* _loader, QWidget *parent) : QWidget(parent) {


    m_loader = _loader;
    m_loader->setAutoUpdate(true);
    m_plans.clear();
    m_plans.append(_loader->getPlans());

    m_choosen = NULL;

    mode     = new QPushButton("Debug Mode", this);
    update   = new QPushButton("Update (Don't Worry! it will work fine :)", this);
    active   = new QPushButton("Active");
    deactive = new QPushButton("Deactive");
    master   = new QPushButton("Master");

    columns = new QColumnView();

    active->setEnabled(false);
    update->setEnabled(false);
    deactive->setEnabled(false);
    master->setEnabled(false);

//    selection = columns->selectionModel();

    model   = new QStandardItemModel();
    selection = new QItemSelectionModel(model);

    updateModel();

    columns->setModel(model);
    columns->setFont(QFont("Monospace"));
    columns->setSelectionModel(selection);

    QList<int> widthList;
    widthList.append(300);
    widthList.append(200);
    widthList.append(100);
    columns->setColumnWidths(widthList);

    //*Details*//


    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);


    QHBoxLayout *buttons  = new QHBoxLayout;
    QVBoxLayout *main     = new QVBoxLayout(this);
    QVBoxLayout *detail   = new QVBoxLayout;

    buttons->addWidget(deactive);
    buttons->addWidget(active);
    buttons->addWidget(master);

    for(int i = 0; i < 8;i++) {
        details[i] = new QLabel(this);
        details[i]->setFont(QFont("Monospace"));
        detail->addWidget(details[i]);
    }


    main->addWidget(mode);
    main->addWidget(update);
    main->addWidget(columns);
    main->addLayout(buttons);
    main->addWidget(line); //  <-- Just A Line
    main->addLayout(detail);

    connect(update   , SIGNAL(clicked()), this, SLOT(slt_updatePlans()));
    connect(mode     , SIGNAL(clicked()), this, SLOT(slt_changeMode()));
    connect(active   , SIGNAL(clicked()), this, SLOT(slt_active()));
    connect(master   , SIGNAL(clicked()), this, SLOT(slt_load()));
    connect(deactive , SIGNAL(clicked()), this, SLOT(slt_deactive()));
    connect(model    , SIGNAL(itemChanged(QStandardItem*)), this, SLOT(slt_edit(QStandardItem*)));
    connect(selection, SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
            this, SLOT(slt_selectionChanged(QItemSelection,QItemSelection)));
    connect(m_loader, SIGNAL(plansUpdated()), this, SLOT(updateModel()));

    setLayout(main);
}

CPlayOffWidget::~CPlayOffWidget() {
}

void CPlayOffWidget::updateModel() {

    model->clear();
    QStandardItem *pkg;
    QStandardItem *file;
    QStandardItem *plan;
    int pkgCounter  = 0;
    int fileCounter = 0;
    int planCounter = 0;
    NGameOff::SGUI* lastGui = new NGameOff::SGUI;
    for (size_t i = 0;i < m_plans.size();i++) {
        m_plans[i]->gui.index[0] = pkgCounter;
        m_plans[i]->gui.index[1] = fileCounter;
        m_plans[i]->gui.index[2] = planCounter;

        NGameOff::SGUI& guiPlan = m_plans.at(i)->gui;
        if (lastGui->package != guiPlan.package) {
            pkgCounter++;
            fileCounter++;
            qDebug() << "PKG";
            pkg = new QStandardItem(guiPlan.package);
            model->appendRow(pkg);
            qDebug() << "FILE";
            file = new QStandardItem(guiPlan.planFile);
            pkg->appendRow(file);
        }
        else if (lastGui->planFile != guiPlan.planFile) {
            fileCounter++;
            qDebug() << "PLAN";
            file = new QStandardItem(guiPlan.planFile);
            pkg->appendRow(file);
        }
        planCounter++;
        plan = new QStandardItem(QString("%1").arg(i));
        file->appendRow(plan);

        file->setEditable(false);
        plan->setEditable(false);
        pkg->setEditable(false);
        lastGui = &m_plans.at(i)->gui;
        qDebug() << guiPlan.package;

    }
}

void CPlayOffWidget::slt_changeMode() {
    debugMode = !debugMode;
    mode->setText((debugMode) ? "Game Mode" : "Debug Mode");
    updateBtn(debugMode);
    qDebug() << "Mode Chaged to " << ((debugMode) ? "Debug" : "Game") << " Mode";
}

void CPlayOffWidget::updateBtn(bool _debug) {
    if (_debug) {
        update   -> setEnabled(true);
        columns  -> setEnabled(true);
    } else {
        active   ->setEnabled(false);
        update   ->setEnabled(false);
        deactive ->setEnabled(false);
        master   ->setEnabled(false);
    }
}

void CPlayOffWidget::slt_updatePlans() {

    m_loader->loadAll();
    m_plans.clear();
    m_plans.append(m_loader->getPlans());
    updateModel();
}

void CPlayOffWidget::slt_active() {
    // TODO : make this plan/package/file active one/ones
}
void CPlayOffWidget::slt_deactive() {
    // TODO : make this plan/package/file deactive one/ones
}

void CPlayOffWidget::slt_master() {
    // TODO : Make this plan/package/file master
}

void CPlayOffWidget::slt_edit(QStandardItem *_item) {
    // TODO : Make it possible to edit plans via gui.
}

void CPlayOffWidget::slt_selectionChanged(const QItemSelection & selected, const QItemSelection & deselected) {

    for(int i = 0;i < 8;i++) details[i]->setText("");

    m_choosen = NULL;

    QModelIndexList modelList = selected.indexes();
    Q_FOREACH(QModelIndex model, modelList) {

        if (model.parent().row() == -1) {
            details[0]->setText(QString("Type  : Package"));
        }
        else if (model.parent().parent().row() == -1) {
            details[0]->setText(QString("Type : File"));
        }
        else if (model.parent().parent().parent().row() == -1) {

            int planIndex = model.data().toInt();

            m_choosen = m_plans.at(planIndex);

            details[0]->setText(QString("Type       : Plan"));
            details[1]->setText(QString("Agent Size : %1").arg(m_plans.at(planIndex)->common.agentSize));
            details[2]->setText(QString("Plan Mode  : %1").arg(m_loader->getModeStr(m_plans.at(planIndex)->common.planMode)));
            details[3]->setText(QString("Chance     : %1").arg(m_plans.at(planIndex)->common.chance));
            details[4]->setText(QString("Last Dist  : %1").arg(m_plans.at(planIndex)->common.lastDist));
            details[5]->setText(QString("Tags       : %1").arg(m_plans.at(planIndex)->common.tags.join(" - ")));
        }
        else {
            details[0]->setText(QString("Type : SubPlan !!"));

        }
    }


}


///////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
CRoleWidget::CRoleWidget(CAgent**_agents, QWidget*parent)
    : QWidget(parent)
{
    agents = _agents;
    mousePos.assign(0.0, 0.0);
    //    this->setFixedSize(200, 300);
    QGridLayout *mainLayout = new QGridLayout;

    execButton = new QPushButton("Execute",this);
    configPages = new QStackedWidget(this);
    btnGroup = new QButtonGroup(this);

    playersNum = 10;

    for(int i = 0; i< playersNum; i++){
        rolesCombos.append(new QComboBox(this));
        rolescount = 0;
        for (int j=0;j<CSkills::skillsCount();j++)
        {
            if (CSkills::skill(j)->level()==2)
            {
                rolescount++;
                CSkillConfigWidget*page = CSkills::skill(j)->generateConfigWidget(this);
                if (page==NULL)
                {
                    page = new CSkillConfigWidget(this);
                    page->skill = CSkills::skill(j)->allocate(agents[i]);
                }
                else {
                    page->mousePos = &mousePos;
                }
                rolesCombos.back()->addItem(CSkills::skill(j)->getName());
                configPages->addWidget(page);
            }
        }

        for (int j=0;j<rolesCombos.back()->count();j++)
            if (rolesCombos.back()->itemText(j) == QString("halt"))
            {
                rolesCombos.back()->setCurrentIndex(j);
                configPages->setCurrentIndex(j + i*rolescount);
                break;
            }
        QRadioButton* btn = new QRadioButton(QString("%1").arg(i));
        if (i==0) btn->setChecked(true);
        btnGroup->addButton(btn,i);
        mainLayout->addWidget(btn, i, 0);
        mainLayout->addWidget(rolesCombos.back(), i, 1);
        connect(rolesCombos.back(), SIGNAL(currentIndexChanged(int)), this, SLOT(curRoleChanged(int)));
    }
    connect(btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(curAgentChanged(int)));
    mainLayout->addWidget(configPages, 0, 2, playersNum, 1);
    mainLayout->addWidget(execButton, playersNum, 0);
    setLayout(mainLayout);
    connect(execButton, SIGNAL(clicked()), this, SLOT(execBtnTriggered()));
    executing = false;
}

CRoleWidget::~CRoleWidget()
{
    delete execButton;
    delete btnGroup;
    while(!rolesCombos.empty())
    {
        delete rolesCombos.back();
        rolesCombos.pop_back();
    }
    int ii=configPages->count();
    QWidget *tW;
    while(ii>0)
    {
        tW =  configPages->widget(ii-1);
        configPages->removeWidget(tW);
        delete tW;
        ii=configPages->count();
    }
    delete configPages;
}

void CRoleWidget::curAgentChanged(int i)
{
    configPages->setCurrentIndex(i*rolescount + rolesCombos[i]->currentIndex());
}

void CRoleWidget::curRoleChanged(int i)
{
    configPages->setCurrentIndex(btnGroup->checkedId()*rolescount + i);
}

void CRoleWidget::execBtnTriggered()
{
    if (!executing)
    {
        executing = true;
        execButton->setText("Pause");
        for (int i=0;i<playersNum;i++)
        {
            agents[i]->skill = static_cast<CSkillConfigWidget*>(configPages->widget(i*rolescount + rolesCombos[i]->currentIndex()))->skill;
            agents[i]->skillName = agents[i]->skill->getName();
            agents[i]->skill->generateFromConfig(agents[i]);
        }
    }
    else {
        execButton->setText("Execute");
        executing = false;
    }
}

bool CRoleWidget::executeRoles()
{
    if (!executing) return false;
    knowledge->frameCount ++;
    for (int i=0;i<playersNum;i++)
    {
        agents[i]->skill->execute();
    }
    return true;
}


QColor colorslist[27] = {QColor(0xff,0x00,0x00,0xff),
                         QColor(0x00,0xff,0x00,0xff),
                         QColor(0x00,0x00,0xff,0xff),
                         QColor(0xff,0xff,0x00,0xff),
                         QColor(0x00,0xff,0xff,0xff),
                         QColor(0xff,0x00,0xff,0xff),
                         QColor(0xff,0xff,0xff,0xff),
                         QColor(0x00,0x00,0x00,0xff),
                         QColor(0x00,0x00,0x9f,0xff),
                         QColor(0x00,0x9f,0x00,0xff),
                         QColor(0x00,0x9f,0x9f,0xff),
                         QColor(0x00,0x9f,0xff,0xff),
                         QColor(0x00,0xff,0x9f,0xff),
                         QColor(0x9f,0x00,0x00,0xff),
                         QColor(0x9f,0x00,0x9f,0xff),
                         QColor(0x9f,0x00,0xff,0xff),
                         QColor(0x9f,0x9f,0x00,0xff),
                         QColor(0x9f,0x9f,0x9f,0xff),
                         QColor(0x9f,0x9f,0xff,0xff),
                         QColor(0x9f,0xff,0x00,0xff),
                         QColor(0x9f,0xff,0x9f,0xff),
                         QColor(0x9f,0xff,0xff,0xff),
                         QColor(0xff,0x00,0x9f,0xff),
                         QColor(0xff,0x9f,0x00,0xff),
                         QColor(0xff,0x9f,0x9f,0xff),
                         QColor(0xff,0x9f,0xff,0xff),
                         QColor(0xff,0xff,0x9f,0xff)};
CPlotWidget::CPlotWidget()
{
    this->resize(500,400);
    QGridLayout *layout = new QGridLayout(this);
    looptmr = new QTimer(this);
    plotTimer = new QTimer(this);
    graph = new CGraphWidget(this , 680 , 300);
    btnPause = new QPushButton("Pause",this);
    btnSavePic = new QPushButton("Save Pic",this);
    for( int i=0 ; i<4 ; i++ ){
        OnOff[i] = new QCheckBox("On/Off" , this);
        cmbMode[i] = new QComboBox(this);
        cmbTeam[i] = new QComboBox(this);
        cmbID[i] = new QComboBox(this);
        cmbPosVelAcc[i] = new QComboBox(this);
        cmbXYZ[i] = new QComboBox(this);
        btnColor[i] = new QPushButton("Set Color" , this);
    }
    for( int i=4 ; i<=CHANNELS_COUNT ; i++ )
        OnOff[i] = new QCheckBox(QString("Custom%1").arg(i-4) , this);


    simul = new SSimulator;
    this->setLayout(layout);

    for( int i=0 ; i<4 ; i++ ){
        cmbMode[i]->addItem("Real Time");
        cmbMode[i]->addItem("Simulator");

        cmbTeam[i]->addItem("Blue");
        cmbTeam[i]->addItem("Yellow");
        cmbTeam[i]->addItem("Ball");

        for( int j=0 ; j<10 ; j++ )
            cmbID[i]->addItem(QString("%1").arg(j));

        cmbPosVelAcc[i]->addItem("Position");
        cmbPosVelAcc[i]->addItem("Velocity");
        cmbPosVelAcc[i]->addItem("Acceleration");
        cmbPosVelAcc[i]->addItem("Direction");
        cmbPosVelAcc[i]->addItem("Angular Velocity");

        cmbXYZ[i]->addItem("X");
        cmbXYZ[i]->addItem("Y");
        cmbXYZ[i]->addItem("Lenght");
    }
    GAIN = 35;

    graph->graphSize.setWidth(680);
    graph->graphSize.setHeight(300);
    graph->show();

    isPaused = false;
    looptmr->start();
    looptmr->setInterval(10);

    plotTimer->start();
    plotTimer->setInterval(1);

    layout->addWidget(graph,0,0,1,8);
    layout->addWidget(btnPause,1,0 , 1 ,2);
    layout->addWidget(btnSavePic,1,2 , 1 , 2);
    for( int i=4 ; i<=CHANNELS_COUNT ; i++ )
        layout->addWidget(OnOff[i] , 2 , i-4);
    for( int i=0 ; i<4 ; i++ ){
        layout->addWidget(OnOff[i] , 3 , i*2 , 1 , 2);
        layout->addWidget(cmbMode[i] , 4 , i*2 , 1 , 2);
        layout->addWidget(cmbTeam[i] , 5 , i*2 , 1 , 2);
        layout->addWidget(cmbID[i] , 6 , i*2 , 1 , 2);
        layout->addWidget(cmbPosVelAcc[i] , 7 , i*2 , 1 , 2);
        layout->addWidget(cmbXYZ[i] , 8 , i*2 , 1 , 2);
        layout->addWidget(btnColor[i] , 9 , i*2 , 1 , 2);
    }

    connect(btnPause,SIGNAL(clicked()),this,SLOT(pauseClicked()));
    connect(btnSavePic ,SIGNAL(clicked()),this,SLOT(savePicClicked()));
    connect(plotTimer,SIGNAL(timeout()),this,SLOT(updatePlots()));
    connect(OnOff[0] , SIGNAL(clicked(bool)) , this , SLOT(AddDelChnl0(bool)));
    connect(OnOff[1] , SIGNAL(clicked(bool)) , this , SLOT(AddDelChnl1(bool)));
    connect(OnOff[2] , SIGNAL(clicked(bool)) , this , SLOT(AddDelChnl2(bool)));
    connect(OnOff[3] , SIGNAL(clicked(bool)) , this , SLOT(AddDelChnl3(bool)));
    connect(OnOff[4] , SIGNAL(clicked(bool)) , this , SLOT(AddDelCustom0Chnl(bool)));
    connect(OnOff[5] , SIGNAL(clicked(bool)) , this , SLOT(AddDelCustom1Chnl(bool)));
    connect(OnOff[6] , SIGNAL(clicked(bool)) , this , SLOT(AddDelCustom2Chnl(bool)));
    connect(OnOff[7] , SIGNAL(clicked(bool)) , this , SLOT(AddDelCustom3Chnl(bool)));
    connect(OnOff[8] , SIGNAL(clicked(bool)) , this , SLOT(AddDelCustom4Chnl(bool)));
    connect(OnOff[9] , SIGNAL(clicked(bool)) , this , SLOT(AddDelCustom5Chnl(bool)));
    connect(OnOff[10] , SIGNAL(clicked(bool)) , this , SLOT(AddDelCustom6Chnl(bool)));
    connect(OnOff[11] , SIGNAL(clicked(bool)) , this , SLOT(AddDelCustom7Chnl(bool)));

    OnOff[4]->setChecked(true);
    AddDelCustom0Chnl(true);

    OnOff[5]->setChecked(true);
    AddDelCustom1Chnl(true);

    OnOff[6]->setChecked(true);
    AddDelCustom2Chnl(true);

    OnOff[7]->setChecked(true);
    AddDelCustom3Chnl(true);
}

CPlotWidget::~CPlotWidget(){
    delete OnOff[CHANNELS_COUNT];
    for( int i=CHANNELS_COUNT-1 ; i>=0 ; i-- )
        delete OnOff[i];
    for( int i=4-1 ; i>=0 ; i-- ){
        delete btnColor[i];
        delete cmbXYZ[i];
        delete cmbPosVelAcc[i];
        delete cmbID[i];
        delete cmbTeam[i];
        delete cmbMode[i];
    }
    delete graph;
    delete btnPause;
    delete btnSavePic;
    delete looptmr;
    delete plotTimer;
}


void CPlotWidget::updatePlots(){
    for( int i=0 ; i<4 ; i++ ){
        if( OnOff[i]->isChecked() ){
            graph->setValueForChnl(QString("%1").arg(i) , desiredValue(i));
        }
    }
    for( int i=4 ; i<=CHANNELS_COUNT ; i++ ){
        if( OnOff[i] )
            graph->setValueForChnl(QString("custom%1").arg(i-4) , custom[i-4]);
    }
}

template <class T>
double CPlotWidget::recognizeDir(T last , int index){
    if( cmbXYZ[index]->currentText() == "X" )
        return last.x;
    else if( cmbXYZ[index]->currentText() == "Y" )
        return last.y;
    else
        return last.length();
}

template <class T>
double CPlotWidget::recognizeVar(T *member , int index ){
    if( cmbPosVelAcc[index]->currentText() == "Position" )
        return recognizeDir(member->pos , index);
    else if( cmbPosVelAcc[index]->currentText() == "Velocity" )
        return recognizeDir(member->vel , index);
    else if( cmbPosVelAcc[index]->currentText() == "Direction" )
        return member->dir.th().radian();
    else if( cmbPosVelAcc[index]->currentText() == "Angular Velocity" )
        return member->angularVel*_DEG2RAD;
    else
        return recognizeDir(member->acc , index);
}

template <class T>
double CPlotWidget::recognizeTeam(T *source , int index ){
    if( (cmbTeam[index]->currentText() == "Blue" && source->getTeamColor() == _COLOR_BLUE) || \
            (cmbTeam[index]->currentText() == "Yellow" && source->getTeamColor() == _COLOR_YELLOW))
        return recognizeVar(source->our[cmbID[index]->currentIndex()] , index);
    if( cmbTeam[index]->currentText() == "Ball" )
        return recognizeVar(source->ball , index);
    return recognizeVar(source->opp[cmbID[index]->currentIndex()] , index);
}

double CPlotWidget::desiredValue( int index ){

    if( cmbMode[index]->currentText() == "Real Time" )
        return recognizeTeam(wm , index);
    else
        return recognizeTeam(simul , index);

}

void CPlotWidget::AddDelChnl0(bool state){
    graph->delChnl("0");
    if( state )
        graph->addChnl("0" , QColor(Qt::red) , GAIN);
}

void CPlotWidget::AddDelChnl1(bool state){
    graph->delChnl("1");
    if( state )
        graph->addChnl("1" , QColor(Qt::green) , GAIN);
}
void CPlotWidget::AddDelChnl2(bool state){
    graph->delChnl("2");
    if( state )
        graph->addChnl("2" , QColor(Qt::blue) , GAIN);
}
void CPlotWidget::AddDelChnl3(bool state){
    graph->delChnl("3");
    if( state )
        graph->addChnl("3" , QColor(Qt::yellow) , GAIN);
}

void CPlotWidget::AddDelCustom0Chnl( bool state ){
    graph->delChnl("custom0");
    if( state )
        graph->addChnl("custom0" , QColor("white") , GAIN);
}

void CPlotWidget::AddDelCustom1Chnl( bool state ){
    graph->delChnl("custom1");
    if( state )
        graph->addChnl("custom1" , QColor("magenta") , GAIN);
}

void CPlotWidget::AddDelCustom2Chnl( bool state ){
    graph->delChnl("custom2");
    if( state )
        graph->addChnl("custom2" , QColor("yellow") , GAIN);
}

void CPlotWidget::AddDelCustom3Chnl( bool state ){
    graph->delChnl("custom3");
    if( state )
        graph->addChnl("custom3" , QColor("cyan") , GAIN);
}

void CPlotWidget::AddDelCustom4Chnl( bool state ){
    graph->delChnl("custom4");
    if( state )
        graph->addChnl("custom4" , QColor("orange") , GAIN);
}


void CPlotWidget::AddDelCustom5Chnl( bool state ){
    graph->delChnl("custom5");
    if( state )
        graph->addChnl("custom5" , QColor("pink") , GAIN);
}

void CPlotWidget::AddDelCustom6Chnl( bool state ){
    graph->delChnl("custom6");
    if( state )
        graph->addChnl("custom6" , QColor("gray") , GAIN);
}

void CPlotWidget::AddDelCustom7Chnl( bool state ){
    graph->delChnl("custom7");
    if( state )
        graph->addChnl("custom7" , QColor("violet") , GAIN);
}

void CPlotWidget::pauseClicked(){
    if(isPaused == false){
        isPaused=true;
        btnPause->setText("Resume");
        graph->updater->stop();
    }else{
        isPaused=false;
        btnPause->setText("Pause");
        graph->updater->start();
    }
}

void CPlotWidget::savePicClicked()
{
    //    graph->save("cycle"+QString::number(cycleCounter)+".jpg",QSize(640,480));
}

CMonitorWidget::CMonitorWidget(CDrawer *_drawerBuffer, QWidget *parent)
    : QGLWidget(parent)
{

    this->setFocusPolicy(Qt::ClickFocus);

    //    verScrl = new QScrollBar(Qt::Vertical, this);

    drawerBuffer = _drawerBuffer;
    drawMouseRect = false;

    list = glGenLists(0);
    glNewList(list, GL_COMPILE);

    stadiumSize.setWidth((_STADIUM_WIDTH));
    stadiumSize.setHeight((_STADIUM_HEIGHT));
    setViewportWidth(conf()->Common_Viewport_Width());

    // TODO: Recode from Field Class
    field.setRect(-_FIELD_WIDTH/2.0, -_FIELD_HEIGHT/2.0, _FIELD_WIDTH, _FIELD_HEIGHT);
    fieldCenter.setRect(-_CENTER_CIRCLE_RAD, _CENTER_CIRCLE_RAD, 2*_CENTER_CIRCLE_RAD, 2*_CENTER_CIRCLE_RAD);
    leftPenalty.setRect(-_FIELD_WIDTH/2.0, - _PENALTY_WIDTH/2.0, _GOAL_RAD, _PENALTY_WIDTH);
    rightPenalty.setRect(_FIELD_WIDTH/2.0 - _GOAL_RAD, -_PENALTY_WIDTH/2.0, _GOAL_RAD, _PENALTY_WIDTH);
    this->setMouseTracking(true);
    //    this->setFixedSize((viewportSize.width()) , (viewportSize.height()));
    object = 0;

    stadiumGreen = QColor::fromRgbF(0.34,0.78,0.18,1.0);//QColor::fromRgbF(0.50,0.25,0.0,1.0);//
    fieldGreen = QColor::fromRgbF(0.27,0.76,0.10,1.0);

    //    verScrl->setGeometry(this->width(), 0, 10, this->height());

    recShowTimer = new QTimer();
    recShowTimer->setInterval(500);
    recShowTimer->start();
    recShowBool = false;
    connect(recShowTimer , SIGNAL(timeout()) , this , SLOT(showHideRec()));
}

void CMonitorWidget::showHideRec(){
    recShowBool = !recShowBool;
}

void CMonitorWidget::setDrawerBuffer(CDrawer *_drawerBuffer){
    drawerBuffer = _drawerBuffer;
}

void CMonitorWidget::setViewportWidth(int width)
{
    viewportWidth = width;
    WH_RATIO= (double)stadiumSize.width()/(double)stadiumSize.height();
    viewportSize.setWidth(viewportWidth);
    viewportSize.setHeight(double(viewportWidth / WH_RATIO));
    resizeGL(viewportWidth, ((double) viewportWidth)/WH_RATIO); //Dont care inputs
    setFixedSize(viewportSize.width(), viewportSize.height());
    //    setFixedSize(800, 600);

}

void CMonitorWidget::changeViewportWidth(VarType* v)
{
    setViewportWidth(((VarTypes::VarInt*)v)->getInt());
}

int CMonitorWidget::getViewportWidth()
{
    return viewportWidth;
}


CMonitorWidget::~CMonitorWidget()
{
    makeCurrent();
    glDeleteLists(object, 1);
}

void CMonitorWidget::paintGL()
{
    bool isLoggerMutexLocked = false;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glLoadIdentity();
    glTranslated(0, 0, -10.0);
    drawField();

    loggerMutex->lock();

    if( gameLogger->getIsLogMode() && recShowBool){
        // Show Log Mode
        drawArc(-3.600 , 2.500 , 0.08 , 0 , 360 , QColor("red") , true);
        drawText(-3.480 , 2.400 , "REC" , QColor("red") , 14);
    }
    loggerMutex->unlock();

    loggerMutex->lock();
    if( gameLogger->getIsReplayMode() ){
        isLoggerMutexLocked = true;
        drawText(-3.480 , 2.400 , "REPLAY" , QColor("blue") , 14);
    }
    else{
        loggerMutex->unlock();
    }

    drawLock();

    try{

        if (conf()->Performance_Monitor_drawRobots())
        {
            CGraphicalRobot rob;
            while (!drawerBuffer->robotBuffer.isEmpty())
            {
                rob = drawerBuffer->robotBuffer.dequeue();
                drawRobot(rob.pos.x,
                          rob.pos.y,
                          rob.dir.th().degree(),
                          rob.ID,
                          rob.comID,
                          rob.color,
                          rob.str,
                          rob.newRobots);
            }
        }
        else
        {
            drawerBuffer->robotBuffer.clear();
        }


        if (conf()->Performance_Monitor_drawArcs())
        {
            CGraphicalArc arc;
            while (!drawerBuffer->arcBuffer.isEmpty())
            {
                arc = drawerBuffer->arcBuffer.dequeue();
                drawArc(arc.circle.center().x,
                        arc.circle.center().y,
                        arc.circle.radius(),
                        arc.startAng,
                        arc.endAng,
                        arc.color,
                        arc.filled);
            }
        }
        else
        {
            // TODO: Draw Ball Anyway among robots
            /* Comment by Mani: Sometimes we must TOFMAL! */
            if (( wm->ball->inSight > 0 ) && (conf()->Performance_Monitor_drawRobots()))
            {
                drawArc(wm->ball->pos.x,
                        wm->ball->pos.y,
                        wm->ball->radius,
                        0,
                        360,
                        QColor("orange"),
                        true);
            }
            drawerBuffer->arcBuffer.clear();
        }

        if (conf()->Performance_Monitor_drawPolygons())
        {
            CGraphicalPolygon polygon;
            while (!drawerBuffer->polygonBuffer.isEmpty())
            {
                polygon = drawerBuffer->polygonBuffer.dequeue();
                glColor4f(polygon.color.redF(),polygon.color.greenF(),polygon.color.blueF(),polygon.color.alphaF());
                if(polygon.filled)
                {
                    glBegin(GL_TRIANGLE_FAN);
                }
                else
                    glBegin(GL_LINE_LOOP);

                std::vector<Vector2D> vertexes = polygon.polygon.vertex();
                for(unsigned int i=0; i < vertexes.size(); i++)
                {
                    //Vector2D ver1 = vertexes.at(i);
                    //Vector2D ver2 = vertexes.at(i+1);
                    //drawLine(ver1.x, ver1.y,
                    //         ver2.x, ver2.y,
                    //         polygon.color);
                    glVertex2f(vertexes.at(i).x, -vertexes.at(i).y);
                }
                glEnd();
            }
        }
        else
        {
            drawerBuffer->pointBuffer.clear();
        }

        if (conf()->Performance_Monitor_drawRects())
        {
            CGraphicalRect rec;
            while(!drawerBuffer->rectBuffer.isEmpty())
            {
                rec = drawerBuffer->rectBuffer.dequeue();
                drawRect(rec.rect.topLeft().x,
                         rec.rect.topLeft().y,
                         rec.rect.bottomRight().x,
                         rec.rect.bottomRight().y,
                         rec.color,
                         rec.filled);
            }
        }
        else
        {
            drawerBuffer->rectBuffer.clear();
        }

        if (conf()->Performance_Monitor_drawSegments())
        {
            CGraphicalSegment seg;
            while(!drawerBuffer->segBuffer.isEmpty())
            {
                seg = drawerBuffer->segBuffer.dequeue();
                drawLine(seg.seg.a().x, seg.seg.a().y,
                         seg.seg.b().x, seg.seg.b().y,
                         seg.color);
            }
        }
        else
        {
            drawerBuffer->segBuffer.clear();
        }

        if (conf()->Performance_Monitor_drawPoints())
        {
            CGraphicalPoint pnt;
            //int sds=drawerBuffer->pointBuffer.size();
            while(!drawerBuffer->pointBuffer.isEmpty())
                //while(sds>0)
            {
                pnt = drawerBuffer->pointBuffer.dequeue();
                // pnt = drawerBuffer->pointBuffer[sds-1];
                drawLine(pnt.point.x-0.050, pnt.point.y+0.050,
                         pnt.point.x+0.050, pnt.point.y-0.050,
                         pnt.color);
                drawLine(pnt.point.x+0.050, pnt.point.y+0.050,
                         pnt.point.x-0.050, pnt.point.y-0.050,
                         pnt.color);
                //sds--;
            }
        }
        else
        {
            drawerBuffer->pointBuffer.clear();
        }

        if (conf()->Performance_Monitor_drawTexts())
        {
            CGraphicalText txt;
            while(!drawerBuffer->textBuffer.isEmpty())
            {
                txt = drawerBuffer->textBuffer.dequeue();
                drawText(txt.pos.x, txt.pos.y, txt.text, txt.color, txt.size);
            }
        }
        else
        {
            drawerBuffer->textBuffer.clear();
        }

        drawUnlock();

        glDeleteLists(object, 1);

        if( isLoggerMutexLocked ){
            loggerMutex->unlock();
        }
    }
    catch( exception* e){
        debug("Exception in updating monitor ! :( ",D_ERROR, "red");
    }
}

void CMonitorWidget::drawText(double x, double y, QString text, QColor color, int size)
{
    glColor3f(color.redF(),color.greenF(),color.blueF());
    QFont font("Times", size);
    //QFontMetrics fm(font);
    //double pixelsWide = fm.width(text);

    renderText(((x + stadiumSize.width() / 2.0)* (double(viewportSize.width()) / double(stadiumSize.width()))),
               ((-1.0*y + stadiumSize.height() / 2.0) * (double(viewportSize.height()) / double(stadiumSize.height()))),
               text, font);
}
void CMonitorWidget::drawRobot(double x, double y, double ang, int ID, int comID, QColor color, QString str, bool newRobots)
{    
    double rad = newRobots ? CRobot::robot_radius_new : CRobot::robot_radius_old;
    if ( newRobots)
    {
        glCallList(drawArc(x, y, rad ,ang+140, ang+400, color, true, true));
        glCallList(drawLine(x, y, x + 0.05*cos(ang*_DEG2RAD) , y + 0.05*sin(ang*_DEG2RAD) , QColor("darkcyan")));
    }
    else
    {
        color.setGreen( (color.green() + 45) < 255 ? (color.green() + 45) : 255);
        color.setBlue( (color.blue() + 45) < 255 ? (color.blue() + 45) : 255);
        color.setRed( (color.red() + 45) < 255 ? (color.red() + 45) : 255);
        glCallList(drawArc(x, y, rad ,0, 360, color, true));
        glCallList(drawArc(x, y, rad ,0, 360, QColor(0, 0, 0), false));
        glCallList(drawLine(x, y, x + rad*cos(ang*_DEG2RAD), y + rad*sin(ang*_DEG2RAD), QColor(0, 0, 0)));
    }
    drawText(x,y+rad,QString("%1 %2").arg(ID).arg(str),QColor(0,0,0),10);
    if(comID!=-1){
        drawText(x,y-rad-0.100,QString::number(comID),QColor(255,0,0),10);
    }
}

void CMonitorWidget::drawField()
{
    glCallList(drawLine(0, field.top(), 0, field.bottom()));
    glCallList(drawArc(0, 0, fieldCenter.width()/2, 0, 360));

    glCallList(drawLine(field.left(), field.top(), field.right(), field.top()));
    glCallList(drawLine(field.left(), field.bottom(), field.right(), field.bottom()));
    glCallList(drawLine(field.left(), field.top(), field.left(), field.bottom()));
    glCallList(drawLine(field.right(), field.top(), field.right(), field.bottom()));

    glCallList(drawArc(leftPenalty.left(), -1*(leftPenalty.bottom()-leftPenalty.width()), leftPenalty.width(), 0, 90));
    glCallList(drawArc(leftPenalty.left(), -1*(leftPenalty.top()+leftPenalty.width()), leftPenalty.width(), 90, 180));
    glCallList(drawLine(leftPenalty.left()+leftPenalty.width(), leftPenalty.bottom()-leftPenalty.width(), field.left()+leftPenalty.width(), leftPenalty.top()+leftPenalty.width() ));

    glCallList(drawArc(rightPenalty.right(), -1*(rightPenalty.bottom()-rightPenalty.width()), rightPenalty.width(), 270, 360));
    glCallList(drawArc(rightPenalty.right(), -1*(rightPenalty.top()+rightPenalty.width()), rightPenalty.width(), 180, 270));
    glCallList(drawLine(rightPenalty.right()-rightPenalty.width(), rightPenalty.bottom()-rightPenalty.width(), rightPenalty.right()-rightPenalty.width(), rightPenalty.top()+rightPenalty.width() ));

    glCallList(drawLine(field.left(), - _GOAL_WIDTH / 2.0, field.left() , _GOAL_WIDTH / 2.0, QColor("black")));
    glCallList(drawLine(field.right(), - _GOAL_WIDTH / 2.0, field.right() , _GOAL_WIDTH / 2.0, QColor("black")));
}

GLuint CMonitorWidget::drawArc(double centerX, double centerY, double radius, int start, int end, QColor color, bool fill, bool fullFill)
{
    glColor4f(color.redF(),color.greenF(),color.blueF(),color.alphaF());
    centerY = -1*centerY;
    if(fill && fullFill)
    {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX+sin(start * _DEG2RAD)*radius, centerY + cos(start * _DEG2RAD)*radius);
    }
    else if(fill)
    {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(centerX, centerY);
    }
    else
        glBegin(GL_LINE_STRIP);

    for( int angle=start; angle<=end; angle+=10 )
        glVertex2f(centerX+sin(angle * _DEG2RAD)*radius, centerY + cos(angle * _DEG2RAD)*radius);
    glEnd();

    glEndList();
    return list;
}

GLuint CMonitorWidget::drawRect(double topLeftX, double topLeftY, double buttomRightX, double buttomRightY, QColor color, bool fill)
{
    //glColor3f(color.redF(),color.greenF(),color.blueF());
    glColor4f(color.redF(),color.greenF(),color.blueF(),color.alphaF());
    topLeftY = -1 * topLeftY;
    buttomRightY= -1 * buttomRightY;
    if(fill)
    {
        glBegin(GL_TRIANGLE_FAN);
    }
    else
        glBegin(GL_LINE_LOOP);

    glVertex2f(topLeftX, topLeftY);
    glVertex2f(buttomRightX, topLeftY);
    glVertex2f(buttomRightX, buttomRightY);
    glVertex2f(topLeftX, buttomRightY);

    glEnd();

    glEndList();
    return list;

}

GLuint CMonitorWidget::drawLine(double x1, double y1, double x2, double y2, QColor color)
{
    glColor3f(color.redF(),color.greenF(),color.blueF());
    glBegin(GL_LINES);
    glVertex2f(x1, -1*y1);
    glVertex2f(x2, -1*y2);
    glEnd();

    glEndList();
    return list;
}

GLuint CMonitorWidget::drawPoint(double x, double y, QColor color)
{
    glColor3f(color.redF(),color.greenF(),color.blueF());
    glBegin(GL_POINTS);
    glVertex2f(x, -1*y);
    glEnd( );

    glEndList();
    return list;
}


void CMonitorWidget::initializeGL()
{
    qglClearColor(stadiumGreen);
    glDisable(GL_DEPTH_TEST);
}


void CMonitorWidget::resizeGL(int width, int height)
{
    //glViewport(0, 0, int(viewportSize.width()), int(viewportSize.height()));
    viewportSize.setWidth(width);
    viewportSize.setHeight(height);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(double(-1.0*stadiumSize.width()/2.0), double(stadiumSize.width()/2.0), double(stadiumSize.height()/2.0), double(-1*stadiumSize.height()/2.0), 4.0, 15.0);
    glMatrixMode(GL_MODELVIEW);

}

void CMonitorWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint tempMousePressPos = event->pos();
    mousePressPos.x = ( ((double(tempMousePressPos.x()) / double(viewportSize.width())) * double(stadiumSize.width())) - (double(stadiumSize.width())/2.0));
    mousePressPos.y = (-1.0*(((double(tempMousePressPos.y()) / double(viewportSize.height())) * double(stadiumSize.height())) - (double(stadiumSize.height())/2.0)));
    emit mousePosChanged(Vector2D(mousePos.x(), mousePos.y()), event->button());
    if( event->button() == Qt::MiddleButton){
        simulatorControlWidget->locateBall(Vector2D(mousePos.x(), mousePos.y()));
    }
}

void CMonitorWidget::mouseReleaseEvent (QMouseEvent */*event*/)
{
    emit mouseReleasedInmonitor();
    drawMouseRect = false;
}

void CMonitorWidget::mouseMoveEvent(QMouseEvent *event)
{

    mousePos = event->pos();

    mousePos.setX( ((double(mousePos.x()) / double(viewportSize.width())) * double(stadiumSize.width())) - (double(stadiumSize.width())/2.0));
    mousePos.setY(-1.0*(((double(mousePos.y()) / double(viewportSize.height())) * double(stadiumSize.height())) - (double(stadiumSize.height())/2.0)));
    viewportSize.setHeight(double(viewportWidth / WH_RATIO));

    emit mouseMovePos( Vector2D(mousePos.x(), mousePos.y()));
}

void CMonitorWidget::wheelEvent(QWheelEvent *event)
{
    if( event->delta() > 0 ){
        setViewportWidth( getViewportWidth()+12);
    }
    else{
        setViewportWidth( getViewportWidth()-12);
    }
}

void CMonitorWidget::keyPressEvent(QKeyEvent *event)
{

    if( event->key() == Qt::Key_B && event->modifiers() == Qt::ControlModifier){
        simulatorControlWidget->locateBall();
    }

    QStringList strRefNames, strRefCommands;
    //              0      1       2      3        4      5       6        7       8       9     10       11
    strRefNames << "H" << "FS" << "S" << "NS" << "FK" << "IK" << "KO" << "PK" << "FK" << "IK" << "KO" << "PK";
    strRefCommands << "H" << "s" << "S" << " " << "F" << "I" << "K" << "P" << "f" << "i" << "k" << "p";

    if( event->modifiers() ==  Qt::ControlModifier )
    {
        addToSelection = true;
    }
    // Abbas
    // Referree Quick Keyes
    if( knowledge->getRefShortcuts())
    {
        if( event->key() == Qt::Key_Enter)
        {
            wm->gs->transition(strRefCommands.at(3).toAscii().at(0));
        }
        if( event->key() == Qt::Key_Period || knowledge->joystick->getButton1())
        {
            wm->gs->transition(strRefCommands.at(0).toAscii().at(0));
        }
        if( event->key() == Qt::Key_0 || knowledge->joystick->getButton4())
        {
            wm->gs->transition(strRefCommands.at(2).toAscii().at(0));
        }
        if( event->key() == Qt::Key_5|| knowledge->joystick->getButton2())
        {
            wm->gs->transition(strRefCommands.at(1).toAscii().at(0) );
        }
        if( event->key() == Qt::Key_9)
        {
            wm->gs->transition(strRefCommands.at(4).toAscii().at(0));
        }
        if( event->key() == Qt::Key_6 || knowledge->joystick->getButton3())
        {
            wm->gs->transition(strRefCommands.at(5).toAscii().at(0));
        }
        if( event->key() == Qt::Key_3)
        {
            wm->gs->transition(strRefCommands.at(6).toAscii().at(0));
        }
        if( event->key() == Qt::Key_7)
        {
            wm->gs->transition(strRefCommands.at(8).toAscii().at(0));
        }
        if( event->key() == Qt::Key_4)
        {
            wm->gs->transition(strRefCommands.at(9).toAscii().at(0));
        }
        if( event->key() == Qt::Key_1)
        {
            wm->gs->transition(strRefCommands.at(10).toAscii().at(0));
        }
    }
}

void CMonitorWidget::keyReleaseEvent(QKeyEvent *event)
{
    //    qDebug() << event->key();
    if( event->key() == Qt::Key_Control)
    {
        addToSelection = false;
    }
}

void CMonitorWidget::closeEvent(QCloseEvent *event)
{
    emit closeSignal(false);
    event->accept();
}

void CMonitorWidget::save()
{
    if (timer.isNull()) timer.start();
    FrameData f;
    f.img =  new QImage (this->grabFrameBuffer(false));
    f.msecs = timer.elapsed();
    images.append(f);
}

void CMonitorWidget::saveAll()
{
    for (int i = 0;i< images.count();i++)
        images[i].img->save(QString("./pix/frame-%1-%2.bmp").arg(i).arg(images[i].msecs), 0, -1);
}

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

/* ============================================================================
 *  PRIVATE Consts
 */
//#define GRAPH_MARGIN_Y			(10)
//#define GRAPH_MARGIN_X			(10)
//#define GRAPH_MARGIN_AREA		(5)
//#define GRAPH_VALUE_HEIGHT		(20)
//#define GRAPH_VALUE_XPAD		(18)
//#define GRAPH_VALUE_YPAD		(14)
//#define GRAPH_POINT_SIZE		(2)

/* ============================================================================
 *  PRIVATE Methods
 */


/* ============================================================================
 *  PRIVATE Class
 */

struct PlotterChannelType{
    QList<THLineGraphData> *data;
    QColor *color;
    float *_startX,*_endX,*_startY,*_endY;
    bool *active;
};
class THLineGraphPrivate {
public:
    QList<PlotterChannelType*> plotCH;
    QColor graphBorder;
    QColor textColor;
    QColor bgBorder;
    QColor graphBg;
    QColor bgColor;
    QImage buffer;

    int maxDataTime;

public:
    void clearBuffer (void);
    void render (const QSize& size);
    void render (QPainter *p, const QSize& size);
    bool save (const QString& fileName, const QSize& size);
};

bool THLineGraphPrivate::save (const QString& fileName, const QSize& size) {
    QImage img(size, QImage::Format_ARGB32_Premultiplied);

    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing);
    render(&p, size);
    p.end();

    return(img.save(fileName));
}


void THLineGraphPrivate::clearBuffer (void) {
    if (!buffer.isNull())
        buffer = QImage();
}

void THLineGraphPrivate::render (const QSize& size) {
    buffer = QImage(size, QImage::Format_ARGB32_Premultiplied);

    QPainter p(&buffer);
    p.setRenderHint(QPainter::Antialiasing);
    render(&p, size);
    p.end();
}

void THLineGraphPrivate::render (QPainter *p, const QSize& size) {
    // Draw Background
    p->setBrush(bgColor);
    p->setPen(bgBorder);
    p->drawRect(QRect(QPoint(0, 0), size));
    // Setup Font
    p->setFont(QFont(p->font().family(), 8));
    // Draw Area
    p->setPen(Qt::NoPen);
    p->setBrush(graphBg);
    p->drawRect(QRect(QPoint(0, 0), size));

    int Margin = 40;
    //bool drawTexts=true;
    int rightTMarg=0;
    if(plotCH.count()>0)
        for(int i=0;i<plotCH.count();i++)
            if(*(plotCH[i]->active)) rightTMarg+=35;
    QSize sizeRP;
    sizeRP.setWidth(size.width() - 2*Margin - rightTMarg);
    sizeRP.setHeight(size.height() - 2*Margin);

    //background
    float startX;
    float endX;
    if(plotCH.count()>0){
        startX= -1;
        endX  = -1;
        for(int i=0;i<plotCH.count();i++){
            if(((startX==-1)||(startX > *plotCH[i]->_startX))&&(*plotCH[i]->active)) startX = *plotCH[i]->_startX;
            if(((endX  ==-1)||(endX   < *plotCH[i]->_endX  ))&&(*plotCH[i]->active)) endX   = *plotCH[i]->_endX;
        }
        if(startX==-1) startX=0;
        if(endX  ==-1) endX  =0;
    }
    if( fabs(endX - startX) > maxDataTime ){
        startX = endX - maxDataTime;
    }

    p->setPen(graphBorder);
    for(int i=0; i<=10;i++){
        p->drawLine(Margin+ 0 + rightTMarg,
                    Margin+ (sizeRP.height() / 10.0) * i,
                    Margin+  sizeRP.width() + rightTMarg,
                    Margin+ (sizeRP.height() / 10.0) * i);

        p->drawLine(Margin+ (sizeRP.width() / 10.0) * i + rightTMarg,
                    Margin+  0,
                    Margin+ (sizeRP.width() / 10.0) * i + rightTMarg,
                    Margin+  sizeRP.height());
        if(i%3==0)
            p->drawText((sizeRP.width() / 10.0) * i + p->fontMetrics().width(QString::number(startX + (endX-startX)/10.0 * i)) + rightTMarg,
                        Margin*(4.0/3.0) + (sizeRP.height()) + p->fontMetrics().height(),
                        QString::number(startX + (endX-startX)/10.0 * i));
    }
    rightTMarg = 0;
    if(plotCH.count()>0)
        for(int jk=0;jk<plotCH.count();jk++){
            if(*(plotCH[jk]->active)){
                p->setPen(QPen(*plotCH[jk]->color, 3));
                p->setBrush(Qt::transparent);
                p->drawText((Margin + rightTMarg - p->fontMetrics().width(QString::number(*plotCH[jk]->_startY))/2.0),
                            Margin + sizeRP.height(),
                            QString::number(*plotCH[jk]->_startY));
                p->drawText((Margin + rightTMarg - p->fontMetrics().width(QString::number((*plotCH[jk]->_endY+*plotCH[jk]->_startY)/2.0))/2.0),
                            Margin + sizeRP.height()/2.0 ,
                            QString::number(((*plotCH[jk]->_endY+*plotCH[jk]->_startY)/2.0)));
                p->drawText((Margin + rightTMarg - p->fontMetrics().width(QString::number(*plotCH[jk]->_endY))/2.0),
                            Margin,
                            QString::number(*plotCH[jk]->_endY));
                rightTMarg+=35;
            }
        }
    for(int iii=0;iii<plotCH.count();iii++){
        float startY = *plotCH[iii]->_startY;
        float endY   = *plotCH[iii]->_endY;
        QSize sizeR;
        sizeR.setWidth(sizeRP.width());
        sizeR.setHeight(sizeRP.height());
        QPainterPath linePath;
        if(plotCH[iii]->data->size()>0){
            linePath.moveTo(Margin+ rightTMarg + (sizeR.width() / (endX - startX)) * (plotCH[iii]->data->first().x()-startX),
                            Margin+  sizeR.height() - (sizeR.height() / (endY - startY)) * (plotCH[iii]->data->first().y()-startY));
            for(int i = 1; i < plotCH[iii]->data->size(); i++){
                linePath.lineTo(Margin+ rightTMarg + (sizeR.width() / (endX - startX)) * (plotCH[iii]->data->at(i).x()-startX),
                                Margin+  sizeR.height() - (sizeR.height() / (endY - startY)) * (plotCH[iii]->data->at(i).y()-startY));
            }
            if(*(plotCH[iii]->active)){
                p->setClipRect(Margin + rightTMarg ,Margin,sizeR.width(), sizeR.height());
                p->setPen(QPen(*plotCH[iii]->color, 1));
                p->setBrush(Qt::transparent);
                p->drawPath(linePath);
            }
        }
    }
}


void THLineGraph::setMaxDataTime(int maxDataTime){
    d->maxDataTime = maxDataTime;
}

/* ============================================================================
 *  PUBLIC Constructors/Destructors
 */

THLineGraph::THLineGraph (QWidget *parent)
    : QGLWidget(parent), d(new THLineGraphPrivate)
{
    d->graphBorder = QColor(0x86, 0x86, 0x86);
    d->textColor = QColor(0xFF, 0xFF, 0xFF);
    d->graphBg = QColor(0x3f, 0x3f, 0x3f);
    d->bgBorder = QColor(0xFF, 0xFF, 0xFF);
    d->bgColor = QColor(0x00, 0x00, 0x00);
}

THLineGraph::~THLineGraph() {
    delete d;
}

void THLineGraph::append (const THLineGraphData& data,int CH) {
    d->plotCH[CH]->data->append(data);
    d->clearBuffer();
}

void THLineGraph::insert (int index, const THLineGraphData& data,int CH) {
    d->plotCH[CH]->data->insert(index, data);
    d->clearBuffer();
}

void THLineGraph::delfirst (int num,int CH) {
    d->plotCH[CH]->data->erase(d->plotCH[CH]->data->begin(),d->plotCH[CH]->data->begin()+num);
    d->clearBuffer();
}

void THLineGraph::delall (int CH) {
    d->plotCH[CH]->data->erase(d->plotCH[CH]->data->begin(),d->plotCH[CH]->data->end());
    d->clearBuffer();
}

int THLineGraph::datalen (int CH) {
    return d->plotCH[CH]->data->length();
}

void THLineGraph::appendCH(QList<THLineGraphData> *dataCH, QColor *color,float *startX,float *endX,float *startY,float *endY, bool *active){
    PlotterChannelType *newData = new PlotterChannelType;
    newData->data  = dataCH;
    newData->color = color;
    newData->_startX = startX;
    newData->_endX   = endX;
    newData->_startY = startY;
    newData->_endY   = endY;
    newData->active  = active;
    d->plotCH.append(newData);
}

bool THLineGraph::save (const QString& fileName, const QSize& size) {
    return(d->save(fileName, size));
}


void THLineGraph::setGraphBorder (const QColor& graphBorder) {
    d->graphBorder = graphBorder;
    d->clearBuffer();
}

void THLineGraph::setGraphBackground (const QColor& graphBg) {
    d->graphBg = graphBg;
    d->clearBuffer();
}

void THLineGraph::setTextColor (const QColor& textColor) {
    d->textColor = textColor;
    d->clearBuffer();
}

void THLineGraph::setBackground (const QColor& bgColor) {
    d->bgColor = bgColor;
    d->clearBuffer();
}

void THLineGraph::setBorder (const QColor& bgBorder) {
    d->bgBorder = bgBorder;
    d->clearBuffer();
}

/* ============================================================================
 *  PROTECTED Methods
 */
void THLineGraph::resizeEvent (QResizeEvent *event) {
    d->clearBuffer();

    QGLWidget::resizeEvent(event);
}

void THLineGraph::paintEvent (QPaintEvent *event) {
    /*	if (d->buffer.isNull())
                d->render(event->rect().size());

        QPainter p(this);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.drawImage(event->rect(), d->buffer, event->rect());
        p.end();
*/
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    d->render(&p, event->rect().size());
    p.end();
    QGLWidget::paintEvent(event);
}


CKeyboardWidget::CKeyboardWidget(CAgent**_agents, QWidget *parent)
{
    this->setFixedSize(200, 250);
    agents = _agents;

    currentAgent = 0;
    speed = 25;
    kickSpeed = 50;
    isKeyboardFree = true;
    QSize btnSize = QSize(20, 20);


    hDLayout = new QGridLayout();;
    selectionLayout = new QGridLayout();;
    actionsLayout = new QGridLayout();;
    vDLayout = new QGridLayout(this);
    navigationLayout = new QGridLayout();


    agentsLbl = new QLabel("Agent : ");
    speedLbl = new QLabel("Speed : ");
    kickSpeedLbl = new QLabel("Kick Speed : ");
    joystickLbl = new QLabel("Active Joystic :");
    joystickChk = new QCheckBox(this);

    agentsCombo = new QComboBox();
    for(int i =0; i< _NUM_PLAYERS; i++){
        agentsCombo->addItem(QString("%1").arg(i));
    }

    agentsCombo->setCurrentIndex(currentAgent);
    agentsCombo->setFocusPolicy(Qt::NoFocus);

    speedLineedit = new QLineEdit(QString::number(speed));
    speedLineedit->setFocusPolicy(Qt::NoFocus);

    kickSpeedLineedit = new QLineEdit(QString::number(kickSpeed));
    kickSpeedLineedit->setFocusPolicy(Qt::ClickFocus);

    upBtn = new QPushButton();
    upBtn->setFocusPolicy(Qt::NoFocus);
    upBtn->setIconSize(btnSize);
    upBtn->setIcon(QIcon("./icons/small/red_up.png"));

    downBtn = new QPushButton();
    downBtn->setFocusPolicy(Qt::NoFocus);
    downBtn->setIconSize(btnSize);
    downBtn->setIcon(QIcon("./icons/small/red_down.png"));

    leftBtn = new QPushButton();
    leftBtn->setFocusPolicy(Qt::NoFocus);
    leftBtn->setIconSize(btnSize);
    leftBtn->setIcon(QIcon("./icons/small/red_left.png"));

    rightBtn = new QPushButton();
    rightBtn->setFocusPolicy(Qt::NoFocus);
    rightBtn->setIconSize(btnSize);
    rightBtn->setIcon(QIcon("./icons/small/red_right.png"));

    cwBtn = new QPushButton();
    cwBtn->setFocusPolicy(Qt::NoFocus);
    cwBtn->setIconSize(btnSize);
    cwBtn->setIcon(QIcon("./icons/small/red_cw.png"));

    ccwBtn = new QPushButton();
    ccwBtn->setFocusPolicy(Qt::NoFocus);
    ccwBtn->setIconSize(btnSize);
    ccwBtn->setIcon(QIcon("./icons/small/red_ccw.png"));

    kickBtn = new QPushButton();
    kickBtn->setFocusPolicy(Qt::NoFocus);
    kickBtn->setIconSize(btnSize);
    kickBtn->setIcon(QIcon("./icons/small/kick_off.png"));


    chipBtn = new QPushButton();
    chipBtn->setFocusPolicy(Qt::NoFocus);
    chipBtn->setIconSize(btnSize);
    chipBtn->setIcon(QIcon("./icons/small/chip_off.png"));

    rollerBtn = new QPushButton();
    rollerBtn->setFocusPolicy(Qt::NoFocus);
    rollerBtn->setIconSize(btnSize);
    rollerBtn->setIcon(QIcon("./icons/small/roller_off.png"));

    haltBtn = new QPushButton();
    haltBtn->setFocusPolicy(Qt::NoFocus);
    haltBtn->setIconSize(btnSize);
    haltBtn->setIcon(QIcon("./icons/small/stop_off.png"));

    navigationLayout->addWidget(ccwBtn, 0, 0);
    navigationLayout->addWidget(upBtn, 0, 1);
    navigationLayout->addWidget(cwBtn, 0, 2);
    navigationLayout->addWidget(leftBtn, 1, 0);
    navigationLayout->addWidget(downBtn, 1, 1);
    navigationLayout->addWidget(rightBtn, 1, 2);

    actionsLayout->addWidget(kickBtn, 1, 0);
    actionsLayout->addWidget(chipBtn, 1, 1);
    actionsLayout->addWidget(rollerBtn, 1, 2);
    actionsLayout->addWidget(haltBtn, 1, 3);

    selectionLayout->addWidget(agentsLbl, 0, 0);
    selectionLayout->addWidget(agentsCombo, 0, 1);
    selectionLayout->addWidget(speedLbl, 1, 0);
    selectionLayout->addWidget(speedLineedit, 1, 1);
    selectionLayout->addWidget(kickSpeedLbl, 2, 0);
    selectionLayout->addWidget(kickSpeedLineedit, 2, 1);
    selectionLayout->addWidget(joystickLbl, 3, 0);
    selectionLayout->addWidget(joystickChk, 3, 1);

    vDLayout->addLayout(selectionLayout, 0, 0);
    vDLayout->addLayout(navigationLayout, 1, 0);
    vDLayout->addLayout(actionsLayout, 2, 0);

    jsTimer = new QTimer(this);
    jsTimer->stop();

    connect(jsTimer, SIGNAL(timeout()), this, SLOT(translateJs()));
    connect(agentsCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentAgent(int)));
    connect(kickSpeedLineedit , SIGNAL(editingFinished()) , this , SLOT(kickSpeedChanged()));
    connect(upBtn , SIGNAL(pressed()) , this , SLOT(forward()));
    connect(downBtn , SIGNAL(pressed()) , this , SLOT(backward()));
    connect(leftBtn , SIGNAL(pressed()) , this , SLOT(left()));
    connect(rightBtn , SIGNAL(pressed()) , this , SLOT(right()));
    connect(cwBtn , SIGNAL(pressed()) , this , SLOT(turnCW()));
    connect(ccwBtn , SIGNAL(pressed()) , this , SLOT(turnCCW()));
    connect(kickBtn , SIGNAL(pressed()) , this , SLOT(kick()));
    connect(chipBtn , SIGNAL(pressed()) , this , SLOT(chip()));
    connect(rollerBtn , SIGNAL(pressed()) , this , SLOT(roller()));

    connect(upBtn , SIGNAL(released()) , this , SLOT(click_released()));
    connect(downBtn , SIGNAL(released()) , this , SLOT(click_released()));
    connect(leftBtn , SIGNAL(released()) , this , SLOT(click_released()));
    connect(rightBtn , SIGNAL(released()) , this , SLOT(click_released()));
    connect(cwBtn , SIGNAL(released()) , this , SLOT(click_released()));
    connect(ccwBtn , SIGNAL(released()) , this , SLOT(click_released()));
    connect(kickBtn , SIGNAL(released()) , this , SLOT(click_released()));
    connect(chipBtn , SIGNAL(released()) , this , SLOT(click_released()));
    connect(rollerBtn , SIGNAL(released()) , this , SLOT(click_released()));
}

CKeyboardWidget::~CKeyboardWidget()
{
    delete agentsLbl;
    delete speedLbl;
    delete kickSpeedLbl;
    delete agentsCombo;
    delete speedLineedit;
    delete kickSpeedLineedit;
    delete upBtn;
    delete downBtn;
    delete leftBtn;
    delete rightBtn;
    delete cwBtn;
    delete ccwBtn;
    delete kickBtn;
    delete chipBtn;
    delete rollerBtn;
    delete haltBtn;
    delete selectionLayout;
    delete actionsLayout;
    delete navigationLayout;
    delete vDLayout;
    delete hDLayout;

}

void CKeyboardWidget::takeAction(EActionType action)
{
    if(knowledge->joystick->getAxes(1)<-0.8)
        action = _FORWARD;
    switch(action)
    {
    case _STOP_NAVIGATION:
        for (int i=0;i<_NUM_PLAYERS;i++)
        {
            char r = 0;
            if (i==currentAgent) r = agents[i]->getRoller();
            agents[i]->waitHere();
            agents[i]->setBeep(true);
            if (i==currentAgent) agents[i]->setRoller(r);
        }

        haltBtn->setIcon(QIcon("./icons/small/stop_on.png"));
        upBtn->setIcon(QIcon("./icons/small/red_up.png"));
        leftBtn->setIcon(QIcon("./icons/small/red_left.png"));
        downBtn->setIcon(QIcon("./icons/small/red_down.png"));
        rightBtn->setIcon(QIcon("./icons/small/red_right.png"));
        ccwBtn->setIcon(QIcon("./icons/small/red_ccw.png"));
        cwBtn->setIcon(QIcon("./icons/small/red_cw.png"));


        break;

    case _STOP_OTHER:
        agents[currentAgent]->setKick(0);
        agents[currentAgent]->setForceKick(false);

        kickBtn->setIcon(QIcon("./icons/small/kick_off.png"));
        chipBtn->setIcon(QIcon("./icons/small/chip_off.png"));

        break;

    case _FORWARD:
        upBtn->setIcon(QIcon("./icons/small/green_up.png"));
        agents[currentAgent]->setRobotVel(speed*4.2 / 127.0, 0.0, 0.0);
        break;

    case _BEEP:
        agents[currentAgent]->beep = !agents[currentAgent]->beep;
        break;

    case _BACKWARD:
        downBtn->setIcon(QIcon("./icons/small/green_down.png"));
        agents[currentAgent]->setRobotVel(-speed*4.2 / 127.0, 0.0, 0.0);
        break;

    case _LEFT:
        leftBtn->setIcon(QIcon("./icons/small/green_left.png"));
        agents[currentAgent]->setRobotVel(0.0, speed*4.2 / 127.0, 0.0);

        break;

    case _RIGHT:
        rightBtn->setIcon(QIcon("./icons/small/green_right.png"));
        agents[currentAgent]->setRobotVel(0.0, -speed*4.2 / 127.0, 0.0);
        break;

    case _TURN_CCW:
        ccwBtn->setIcon(QIcon("./icons/small/green_ccw.png"));
        agents[currentAgent]->setRobotVel(0.0, 0.0, speed/20 );
        break;

    case _TURN_CW:
        cwBtn->setIcon(QIcon("./icons/small/green_cw.png"));
        agents[currentAgent]->setRobotVel(0.0, 0.0, -speed/20);
        //		agents[currentAgent]->setRobotVel(0.0, 0.0, -speed*90.0 / 127.0);
        break;

    case _KICK:

        kickBtn->setIcon(QIcon("./icons/small/kick_on.png"));
        agents[currentAgent]->setKick(kickSpeed);
        break;

    case _CHIP:
        chipBtn->setIcon(QIcon("./icons/small/chip_on.png"));
        agents[currentAgent]->setChip(kickSpeed);
        break;

    case _ROLLER:
        if ( agents[currentAgent]->getRoller() )
        {
            rollerBtn->setIcon(QIcon("./icons/small/roller_off.png"));
            agents[currentAgent]->setRoller(0);
        }
        else
        {
            rollerBtn->setIcon(QIcon("./icons/small/roller_on.png"));
            agents[currentAgent]->setRoller(7);
        }
        break;

    case _STOP_ALL:
        break;

    }

    for (int i=0;i<_NUM_PLAYERS;i++)
        if (i!=currentAgent)
            agents[i]->waitHere();

}

void CKeyboardWidget::keyPressEvent(QKeyEvent *event)
{

    isKeyboardFree = false;

    if( event->key() == Qt::Key_Space )
    {
        takeAction(_STOP_NAVIGATION);
    }

    else if( event->key() == Qt::Key_K )
    {
        takeAction(_KICK);
    }

    else if( event->key() == Qt::Key_R )
    {
        takeAction(_ROLLER);
    }

    else if( event->key() == Qt::Key_C )
    {
        takeAction(_CHIP);
    }

    else if( event->key() == Qt::Key_W )
    {
        takeAction(_FORWARD);
    }
    else if( event->key() == Qt::Key_A )
    {
        takeAction(_LEFT);
    }
    else if( event->key() == Qt::Key_S )
    {
        takeAction(_BACKWARD);
    }
    else if( event->key() == Qt::Key_D )
    {
        takeAction(_RIGHT);
    }
    else if( event->key() == Qt::Key_Q )
    {
        takeAction(_TURN_CCW);
    }
    else if( event->key() == Qt::Key_E )
    {
        takeAction(_TURN_CW);
    }
    else if( event->key() == Qt::Key_U )
    {
        takeAction(_BEEP);
    }

    else if( event->key() == Qt::Key_Up )
    {
        agentsCombo->setCurrentIndex(agentsCombo->currentIndex() + 1 > _NUM_PLAYERS-1 ? _NUM_PLAYERS-1 : agentsCombo->currentIndex() + 1);
    }
    else if( event->key() == Qt::Key_Down )
    {
        agentsCombo->setCurrentIndex(agentsCombo->currentIndex() - 1 < 0 ? 0 : agentsCombo->currentIndex() - 1);
    }

    else if (event->key() == Qt::Key_PageUp )
    {
        speedLineedit->setText(QString::number(speed + 1 > _BIT_RESOLUTION ? speed : ++speed));
    }

    else if (event->key() == Qt::Key_PageDown )
    {
        speedLineedit->setText(QString::number(speed - 1 < 0 ? speed : --speed));
    }
    else if (event->key() == Qt::Key_Home )
    {
        if (kickSpeed < 1024 - 1)
            kickSpeed += 50;
        kickSpeedLineedit->setText(QString::number(kickSpeed));
    }

    else if (event->key() == Qt::Key_End )
    {
        if (kickSpeed >= 1)
            kickSpeed -= 50;

        kickSpeedLineedit->setText(QString::number(kickSpeed));
    }



}

void CKeyboardWidget::keyReleaseEvent(QKeyEvent *event)
{
    isKeyboardFree = true;

    if( event->isAutoRepeat() )
        return;


    if(event->key() == Qt::Key_J || event->key() == Qt::Key_K || event->key() == Qt::Key_C || event->key() == Qt::Key_R)
    {
        takeAction(_STOP_OTHER);
        return;
    }

    takeAction(_STOP_NAVIGATION);
    return;
}

void CKeyboardWidget::wheelEvent(QWheelEvent *event)
{
    if( event->delta() > 0 )
        speedLineedit->setText(QString::number(speed + 1 > _BIT_RESOLUTION ? speed : ++speed));
    else
        speedLineedit->setText(QString::number(speed - 1 < -1 * _BIT_RESOLUTION ? speed : --speed));
}

void CKeyboardWidget::setCurrentAgent(int num)
{
    currentAgent = num;
}

void CKeyboardWidget::translateJs()
{
#ifndef Q_WS_MAC
#ifndef NO_JS
    if ( !knowledge->joystick->isJsConnected() || !joystickChk->isChecked())
        return;
    bool nav = false;
    if( ! isKeyboardFree )
    {
        return;
    }

    if( knowledge->joystick->getButton6())
    {
        takeAction(_ROLLER);
    }

    if( knowledge->joystick->getAxes(1) < -0.8)
    {
        takeAction(_FORWARD);
        nav = true;
    }
    else if( knowledge->joystick->getAxes(0) < -0.8)
    {
        takeAction(_LEFT);
        nav = true;
    }
    if( knowledge->joystick->getAxes(1) > 0.8)
    {
        takeAction(_BACKWARD);
        nav = true;
    }
    else if( knowledge->joystick->getAxes(0) > 0.8)
    {
        takeAction(_RIGHT);
        nav = true;
    }
    else if( knowledge->joystick->getAxes(2) < -0.8)
    {
        takeAction(_TURN_CCW);
        nav = true;
    }
    else if( knowledge->joystick->getAxes(2) > 0.8)
    {
        takeAction(_TURN_CW);
        nav = true;
    }
    else if( knowledge->joystick->getButton5())
    {
        takeAction(_KICK);
    }
    else if( knowledge->joystick->getButton7())
    {
        takeAction(_CHIP);
    }
    else
        takeAction(_STOP_OTHER);
    return;
#endif
#endif
}

void CKeyboardWidget::closeEvent(QEvent*)
{
    emit closeSignal(false);
}

void CKeyboardWidget::kickSpeedChanged(){
    QRegExp regx("[0-9]+");
    if( regx.exactMatch(kickSpeedLineedit->text()) ){
        int val = kickSpeedLineedit->text().toInt();
        if( val < 0 || val > 1023 ){
            kickSpeedLineedit->setText(QString("%1").arg(kickSpeed));
        }
        else
            kickSpeed = val;
    }
    else
        kickSpeedLineedit->setText(QString("%1").arg(kickSpeed));
    kickSpeedLineedit->clearFocus();
    this->setFocus();
}

CInfoWidget::CInfoWidget(QWidget *parent):QGLWidget(parent)
{
}

CInfoWidget::~CInfoWidget()
{
}

void CInfoWidget::setSenders(QHostAddress *_refSender, QHostAddress *_visSender){
    refSender = _refSender;
    visSender = _visSender;
}

void CInfoWidget::setWorldModel(CWorldModel *_wm){
    wm = _wm;
}

void CInfoWidget::setBallsCount(int _ballsCount){
    ballsCount = _ballsCount;
}

void CInfoWidget::render(QPainter *p, const QSize& size){
    p->setBrush(QColor("lightgray"));
    p->setPen(QColor("lightgray"));
    p->drawRect(QRect(QPoint(0, 0), size));

    p->setPen(QColor("black"));
    //    p->drawText(10,20,"Kalman:");
    p->drawText(10,40,"BangBang:");
    p->drawText(10,100,"Our Team Side:");
    p->drawText(10,120,"Our Color:");
    p->drawText(10,140,"Vision Server:");
    p->drawText(10,160,"Referee Server:");

    //kalman
    //    if(conf()->Kalman_UseKalman())
    //    {
    //        p->setBrush(QColor("green"));
    //        p->setPen(QColor("green"));
    //    }else{
    //        p->setBrush(QColor("red"));
    //        p->setPen(QColor("red"));
    //    }
    //    p->drawEllipse(QPoint(200,15),7,7);


    //Side
    p->setBrush(QColor("black"));
    p->setPen(QPen(QColor("black"),2));
    p->drawLine(200,100,240,100);
    p->drawText(250,100,QString(conf()->LocalSettings_OurTeamSide().c_str()));
    if(conf()->LocalSettings_OurTeamSide()=="Right")
    {
        p->drawLine(240,100,235,105);
        p->drawLine(240,100,235,95);
    }else if(conf()->LocalSettings_OurTeamSide()=="Left"){
        p->drawLine(200,100,205,105);
        p->drawLine(200,100,205,95);
    }
    //Color
    if(conf()->LocalSettings_OurTeamColor()=="Blue")
    {
        p->setBrush(QColor("blue"));
        p->setPen(QColor("blue"));
    }else if(conf()->LocalSettings_OurTeamColor()=="Yellow"){
        p->setBrush(QColor("yellow"));
        p->setPen(QColor("yellow"));
    }
    p->drawEllipse(QPoint(200,115),7,7);

    //Vision
    p->setBrush(QColor("black"));
    p->setPen(QColor("black"));
    p->drawText(200,140,QString("%1:%2  Sender:%3 ballsCount:%4").arg(QString(conf()->LocalSettings_SSLVisionMulticastAddr().c_str())).arg(conf()->LocalSettings_SSLVisionMulticastPort()).arg(visSender->toString()).arg(ballsCount));

    //Referee
    QString strcomms="";
    for(int i=0; i<wm->refCommand.count();i++){
        strcomms += wm->refCommand[i] + " ";
    }
    p->setBrush(QColor("black"));
    p->setPen(QColor("black"));
    p->drawText(200,160,QString("%1:%2  Result:%5 Sender:%3 Hist:%4").arg(QString(conf()->LocalSettings_RefereeMulticastAddr().c_str())).arg(conf()->LocalSettings_RefereeMulticastPort()).arg(refSender->toString()).arg(strcomms).arg(QString("%1-%2").arg(wm->gs->ourScore()).arg(wm->gs->oppScore())));
    p->drawText(20,180,QString("Status: %1").arg(status));
}

void CInfoWidget::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    render(&p, event->rect().size());
    p.end();
    QGLWidget::paintEvent(event);
}

void CInfoWidget::closeEvent(QCloseEvent* /*event*/)
{
    emit closeSignal(false);
}

void CInfoWidget::resizeEvent(QResizeEvent* event)
{
    this->resize(600,190);
    QGLWidget::resizeEvent(event);
}

void CInfoWidget::update(){
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    render(&p, this->size());
    p.end();
}

CAgentsStatusWidget::CAgentsStatusWidget(CCoach* _coach, CWorldModel* _wm, CAgent **_agents)
{
    coach = _coach;
    wm = _wm;
    agents = _agents;
    QGridLayout *layout = new QGridLayout(this);
    l = new QLabel* [_MAX_NUM_PLAYERS];
    v = new QLabel* [_MAX_NUM_PLAYERS];
    r = new QLabel* [_MAX_NUM_PLAYERS];
    ballInfo = new QLabel("",this);
    for (int i=0;i<_MAX_NUM_PLAYERS;i++)
    {
        l[i] = new QLabel(this);
        v[i] = new QLabel(this);
        r[i] = new QLabel(this);
        layout->addWidget(l[i],i,0);
        layout->addWidget(r[i],i,1);
        layout->addWidget(v[i],i,2);
    }
    refCommand = new QLineEdit(this);
    layout->addWidget(ballInfo,_NUM_PLAYERS,0,1,3);
    layout->addWidget(refCommand,_NUM_PLAYERS+1,0,1,3);
    setLayout(layout);

}

CAgentsStatusWidget::~CAgentsStatusWidget()
{
    delete []l;
    delete []v;
    delete []r;
    delete ballInfo;
}

void CAgentsStatusWidget::update()
{
    for (int i=0;i<_MAX_NUM_PLAYERS;i++)
    {
        if (wm->our[i]->inSight>0)
        {
            QString task = "-";
            v[i]->setText("");
            l[i]->setText(QString("Agent %1(%2): ").arg(i).arg(task));
            r[i]->setText(agents[i]->skillName);
        }
        else
        {
            l[i]->setText("");
            v[i]->setText("");
            r[i]->setText("");
        }
    }
}

void CAgentsStatusWidget::updateBallInfo(QString _ballInfo){
    ballInfo->setText(_ballInfo);
}

CSimulatorControllerWidget::CSimulatorControllerWidget(CWorldModel *_wm ){
    wm = _wm;
    QGridLayout *layout = new QGridLayout(this);
    lblFPS = new QLabel("Desired FPS" , this);
    lblDETStp = new QLabel("OpenDETimeStep" , this);
    lblTeam = new QLabel("Locate an Agent : ", this);
    lblBall = new QLabel("Locate Ball : ", this);
    lbAgentData = new QLabel("", this);
    lbAgentPos = new QLabel("", this);
    btnDETStp = new QPushButton("Change" , this);
    btnFPS = new QPushButton("Change" , this);
    btnBallLocate = new QPushButton("Locate Ball", this);
    txtFPS = new QLineEdit(this);
    txtDETStp = new QLineEdit(this);
    OGLstate = new QCheckBox("OpenGL On/Off State",this);
    lblteams = new QLabel("Team" , this);
    lblagents = new QLabel("Agent" , this);
    cmbteams = new QComboBox(this);
    cmbagents = new QComboBox(this);
    btnlocate = new QPushButton("Locate" , this);
    cmbteams->addItem("Blue");
    cmbteams->addItem("Yellow");
    for( int i=0 ; i<12 ; i++ )
        cmbagents->addItem(QString("%1").arg(i));
    layout->addWidget(lblFPS , 0 , 0 );
    layout->addWidget(txtFPS , 0 , 1 );
    layout->addWidget(btnFPS , 0 , 2);
    layout->addWidget(lblDETStp , 1 , 0 );
    layout->addWidget(txtDETStp , 1 , 1);
    layout->addWidget(btnDETStp , 1 , 2);
    layout->addWidget(OGLstate ,2 , 0 );
    layout->addWidget(lblTeam, 3, 0);
    layout->addWidget(lblteams , 4 , 0);
    layout->addWidget(cmbteams , 4 , 1);
    layout->addWidget(lbAgentData, 6, 0);
    layout->addWidget(lbAgentPos, 6, 1);
    layout->addWidget(btnlocate , 6 , 2);
    layout->addWidget(lblagents, 5 , 0);
    layout->addWidget(cmbagents , 5 , 1);
    layout->addWidget(lblBall, 7, 0);
    layout->addWidget(btnBallLocate, 7, 1);
    layout->setAlignment(Qt::AlignTop);
    connect(btnFPS , SIGNAL(clicked()) , this , SLOT(SetSimulatorFPS()));
    connect(btnDETStp , SIGNAL(clicked()) , this , SLOT(SetSimulatorOpenDETimeStep()));
    connect(OGLstate , SIGNAL(clicked()) , this , SLOT(SetSimulatorOGLState()));
    connect(btnlocate , SIGNAL(clicked()) , this , SLOT(locateRobot()));
    connect(btnBallLocate, SIGNAL(clicked()), this, SLOT(locateBall()));
}

CSimulatorControllerWidget::~CSimulatorControllerWidget(){
    delete cmbagents;
    delete cmbteams;
    delete lblagents;
    delete lblteams;
    delete OGLstate;
    delete txtDETStp;
    delete txtFPS;
    delete btnFPS;
    delete btnDETStp;
    delete lblDETStp;
    delete lblFPS;
}


void CSimulatorControllerWidget::SetSimulatorFPS(){
    int _FPS = txtFPS->text().toInt();
    if( _FPS >=0 && _FPS <= 1000 )
        wm->setSimulatorDesiredFPS(_FPS);
}

void CSimulatorControllerWidget::SetSimulatorOGLState(){
    bool _State = OGLstate->isChecked();
    wm->setSimulatorOpenGLState(_State);
}

void CSimulatorControllerWidget::SetSimulatorOpenDETimeStep(){
    double _Step = txtDETStp->text().toDouble();
    if( _Step>=0 && _Step<=1 )
        wm->setSimulatorOpenDETimeStep(_Step);
}

void CSimulatorControllerWidget::locateRobot()
{
    Vector2D MPos =  knowledge->getMousePos();
    double TS = (wm->getTeamSide() == _SIDE_RIGHT)? -1.0f : 1.0f;
    MPos *= TS;
    lbAgentData->setText(QString("Agent %1 of Team %2").arg(cmbagents->currentText()).arg(cmbteams->currentText()));
    lbAgentPos->setText(QString("X: %1 , Y: %2").arg(float(int(MPos.x*100)/100.0)).arg(float(int(MPos.y*100)/100.0)));
    if( (wm->getTeamQColor() == "blue" && cmbteams->currentIndex() == 0) || (wm->getTeamQColor() == "yellow" && cmbteams->currentIndex() == 1 ) ){
        wm->our[cmbagents->currentIndex()]->setReplace(MPos , 180*(1-TS)/2.0);
    }
    else{
        wm->opp[cmbagents->currentIndex()]->setReplace(MPos , 180*(1-TS)/2.0);
    }
    return;
}

void CSimulatorControllerWidget::locateBall()
{
    Vector2D MPos =  knowledge->getMousePos();
    double TS = (wm->getTeamSide() == _SIDE_RIGHT)? -1.0f : 1.0f;
    MPos *= TS;
    wm->ball->setReplace(MPos,Vector2D(0,0));
}

void CSimulatorControllerWidget::locateBall( Vector2D _newPos )
{
    Vector2D MPos =  _newPos;
    double TS = (wm->getTeamSide() == _SIDE_RIGHT)? -1.0f : 1.0f;
    MPos *= TS;
    wm->ball->setReplace(MPos,Vector2D(0,0));
}

CLoggerWidget::CLoggerWidget(){
    this->setGeometry(520 , 480 , 450 , 300);
    isLoggerWorking = false;
    btnPlay = new QPushButton(this);
    btnPlay->setIcon(QIcon("./icons/Play-Hot-icon.png"));
    btnNextFrame = new QPushButton(this);
    btnNextFrame->setIcon(QIcon("./icons/Next-Hot-icon.png"));
    btnNextFrame->setShortcut(QKeySequence(tr("Right", "NextFrame")));
    btnPreviousFrame = new QPushButton(this);
    btnPreviousFrame->setIcon(QIcon("./icons/Previous-Hot-icon.png"));
    btnPreviousFrame->setShortcut(QKeySequence(tr("Left", "PreviousFrame")));
    slider = new QSlider(Qt::Horizontal , this);
    dialog = new QFileDialog(this);
    btnBrowse = new QPushButton("Browse" , this);
    cmbList = new QComboBox(this);
    timer = new QTimer(this);
    lblTime = new QLabel(this);
    lblRefCmd = new QLabel("    RefCmd" , this);
    debugTexts = new QTextEdit(this);
    btnClear = new QPushButton("Clear" , this);
    chbxDraws = new QCheckBox("Draws" , this);
    chbxDebugs = new QCheckBox("Debugs" , this);

    chbxDebug[0] = new QCheckBox("DON_MHMMD" , this);
    chbxDebug[1] = new QCheckBox("Mahi" , this);
    chbxDebug[2] = new QCheckBox("Hamed" , this);
    chbxDebug[3] = new QCheckBox("Nadia" , this);
    chbxDebug[4] = new QCheckBox("Fatemeh" , this);
    chbxDebug[5] = new QCheckBox("Atousa" , this);
    chbxDebug[6] = new QCheckBox("Mahmood" , this);
    chbxDebug[7] = new QCheckBox("AHZ" , this);
    chbxDebug[8] = new QCheckBox("Amin" , this);
    chbxDebug[9] = new QCheckBox("AmiR" , this);
    chbxDebug[10] = new QCheckBox("Game" , this);
    chbxDebug[11] = new QCheckBox("Ali" , this);
    chbxDebug[12] = new QCheckBox("Arash" , this);
    chbxDebug[13] = new QCheckBox("Mani" , this);
    chbxDebug[14] = new QCheckBox("Masoud" , this);
    chbxDebug[15] = new QCheckBox("Sepehr" , this);
    chbxDebug[16] = new QCheckBox("Mohammed" , this);
    chbxDebug[17] = new QCheckBox("Hossein" , this);
    chbxDebug[18] = new QCheckBox("KK" , this);
    chbxDebug[19] = new QCheckBox("Erfan" , this);

    tcolor.insert(D_ERROR,QColor(Qt::red));
    tcolor.insert(D_MAHI,QColor(Qt::green));
    // TODO:insert color for other Types
    txtFPS = new QLineEdit("60" , this);
    lblFPS = new QLabel("FPS" , this);

    chbxDraws->setChecked(true);
    chbxDebugs->setChecked(true);
    lblTime->setText("00:00:00:00");
    lblRefCmd->setStyleSheet("background: gray; color: blue;");
    btnPlay->setMaximumSize(30 , 30);
    btnPlay->setShortcut(QKeySequence(tr("Space", "PlayPause")));
    btnBrowse->setMaximumSize(100 , 30);
    cmbList->setMaximumSize(350 , 30);
    timer->setInterval(17);
    slider->setRange(0 , 0);
    txtFPS->setMaximumSize(65 , 30);
    txtFPS->setFocusPolicy(Qt::ClickFocus);
    pause = true;
    for( int i=0 ; i<8 ; i++ )
        chbxDebug[i]->setChecked(true);

    QGridLayout *l = new QGridLayout(this);
    l->addWidget(btnBrowse , 1  , 0 , 1 , 3);
    l->addWidget(cmbList , 2 , 0 , 1 , 8);
    l->addWidget(chbxDraws , 1 , 8);
    l->addWidget(chbxDebugs , 2 , 8);
    l->addWidget(slider , 3 , 0 , 1 , 8);
    l->addWidget(txtFPS , 3 , 8);
    l->addWidget(lblFPS , 3 , 9);
    l->addWidget(btnPlay , 4 , 0);
    l->addWidget(lblTime , 4 , 1 , 1 , 3);
    l->addWidget(btnPreviousFrame , 4 , 6);
    l->addWidget(btnNextFrame , 4 , 7);
    l->addWidget(lblRefCmd , 4 , 8);
    l->addWidget(debugTexts , 5 , 0 , 7 , 8);
    QWidget* DebugNames=new QWidget();
    QVBoxLayout *DebugNamesLayout=new QVBoxLayout;
    QScrollArea *scrollArea=new QScrollArea;
    for( int i=0 ; i<20 ; i++ )
        DebugNamesLayout->addWidget(chbxDebug[i]);

    DebugNames->setLayout(DebugNamesLayout);

    scrollArea->setWidget(DebugNames);
    l->addWidget(scrollArea,6,8);

    l->addWidget(btnClear , 12 , 0 , 1 , 3);
    reBuildWidget();

    connect(btnBrowse , SIGNAL(pressed()) , this , SLOT(browseDialog()));
    connect(btnPlay , SIGNAL(pressed()) , this , SLOT(playLog()));
    connect(btnPreviousFrame , SIGNAL(pressed()) , this, SLOT(goPreviousFrame()));
    connect(btnNextFrame , SIGNAL(pressed()) , this , SLOT(goNextFrame()));
    connect(slider , SIGNAL(sliderMoved(int)) , this , SLOT(seekChange(int)));
    connect(timer , SIGNAL(timeout()) , this , SLOT(cursorIncrement()));
    connect(btnClear , SIGNAL(pressed()) , this , SLOT(clearDebugTexts()));
    connect(cmbList , SIGNAL(currentIndexChanged(int)) , this , SLOT(playThisFile(int)));
    connect(chbxDraws , SIGNAL(clicked(bool)) , this , SLOT(showHideDraws(bool)));
    connect(chbxDebugs , SIGNAL(clicked(bool)) , this , SLOT(showHideDebugs(bool)));
    for( int i=0 ; i<20 ; i++ )
        connect(chbxDebug[i] , SIGNAL(clicked()) , this , SLOT(debugTypeChanged()));
    connect(txtFPS , SIGNAL(editingFinished()) , this , SLOT(replayFPSChanged()));
    debugTypeChanged();
    btnBrowse->setAutoDefault(false);
    btnClear->setAutoDefault(false);
    btnPlay->setAutoDefault(false);
    slider->setMouseTracking(true);

    fBfStep = 0;
}

CLoggerWidget::~CLoggerWidget(){
    delete lblFPS;
    delete txtFPS;
    for( int i=20 ; i>=0 ; i-- )
        delete chbxDebug[i];
    delete chbxDebugs;
    delete chbxDraws;
    delete btnClear;
    delete debugTexts;
    delete lblRefCmd;
    delete lblTime;
    delete timer;
    delete cmbList;
    delete btnBrowse;
    delete dialog;
    delete slider;
    delete btnNextFrame;
    delete btnPlay;
}

void CLoggerWidget::reBuildWidget(){
    this->setGeometry(520 , 480 , 450 , 300);

    QDir sourceDir;
#ifndef Q_OS_MACX
    sourceDir.setPath("logs/");
#else
    sourceDir.setPath("../../../logs/");
#endif
    QStringList filter;
    filter << "*.log";
    sourceDir.setNameFilters(filter);
    sourceDir.setSorting(QDir::Name);
    cmbList->clear();
    fileNames.clear();
    QFileInfoList list = sourceDir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        fileNames.append(fileInfo.filePath());
        cmbList->addItem(fileInfo.fileName());
    }
    if( fileNames.size() ){
        playThisFile(cmbList->count()-1);
        playLog();
    }
}

void CLoggerWidget::browseDialog(){
    QStringList files = dialog->getOpenFileNames(this, tr("Open LogFile") ,"log" , tr("Log Files (*.log)"));
    //qDebug()<<files;

    if( files.size() == 0 )
        return;

    fileNames = files;
    cmbList->clear();

    QString str , name;
    int sz;
    for( int i=0 ; i<fileNames.size() ; i++ ){
        str = fileNames.at(i);
        sz = str.toStdString().size();
        name = "";
        if( sz ){
            for( int j=sz-1 ; j>=0 && str[j] != '/' ; j-- ){
                name += str[j];
            }
            reverse(name.begin() , name.end());
            cmbList->addItem(name);
        }
    }
    playThisFile(0);
}

void CLoggerWidget::playThisFile( int index ){
    clearDebugTexts();
    if( index < 0 || index >= fileNames.size() ){
        btnPlay->setIcon(QIcon("./icons/Play-Hot-icon.png"));
        timer->stop();
        slider->setValue(0);
        pause = true;

        loggerMutex->lock();
        gameLogger->playPauseMode = false;
        gameLogger->seek = 0;
        loggerMutex->unlock();
        return;
    }
    cmbList->setCurrentIndex(index);
    int maxRange = 0;

    btnPlay->setIcon(QIcon("./icons/Pause-Hot-icon.png"));

    loggerMutex->lock();
    gameLogger->playPauseMode = true;
    gameLogger->openFileToReplay(fileNames.at(index));
    maxRange = gameLogger->framenum;
    loggerMutex->unlock();

    pause = false;
    timer->start();
    slider->setRange(0 , maxRange);
    slider->setValue(0);
}

void CLoggerWidget::playLog(){
    fBfStep = 0;
    if( pause ){
        pause = false;
        btnPlay->setIcon(QIcon("./icons/Pause-Hot-icon.png"));
        timer->start();

        loggerMutex->lock();
        gameLogger->playPauseMode = true;
        gameLogger->fBfMode = false;
        loggerMutex->unlock();
    }
    else{
        pause = true;
        btnPlay->setIcon(QIcon("./icons/Play-Hot-icon.png"));
        timer->stop();

        loggerMutex->lock();
        gameLogger->playPauseMode = false;
        gameLogger->fBfMode = false;
        loggerMutex->unlock();
    }
}

void CLoggerWidget::goPreviousFrame(){
    if( pause == false ){
        pause = true;
        btnPlay->setIcon(QIcon("./icons/Play-Hot-icon.png"));
        timer->stop();

        loggerMutex->lock();
        gameLogger->playPauseMode = false;
        loggerMutex->unlock();
    }
    if( fBfStep != -1 ){
        fBfStep = -1;
        loggerMutex->lock();
        gameLogger->playPauseMode = false;
        gameLogger->fBfMode = true;
        loggerMutex->unlock();
    }
    debugTexts->clear();
    cursorIncrement();
}

void CLoggerWidget::goNextFrame(){
    if( pause == false ){
        pause = true;
        btnPlay->setIcon(QIcon("./icons/Play-Hot-icon.png"));
        timer->stop();

        loggerMutex->lock();
        gameLogger->playPauseMode = false;
        loggerMutex->unlock();
    }
    if( fBfStep != 1 ){
        fBfStep = 1;
        loggerMutex->lock();
        gameLogger->playPauseMode = false;
        gameLogger->fBfMode = true;
        loggerMutex->unlock();
    }

    debugTexts->clear();
    cursorIncrement();
}

void CLoggerWidget::seekChange(int newSeek){
    loggerMutex->lock();
    gameLogger->seek = newSeek;
    loggerMutex->unlock();
    debugTexts->clear();
}

void CLoggerWidget::cursorIncrement(){
    if( slider->value() >= slider->maximum() ){
        playThisFile(cmbList->currentIndex());
        playLog();
        return;
    }
    int seek;
    loggerMutex->lock();
    if( gameLogger->fBfMode )
        gameLogger->seek += fBfStep;
    if( gameLogger->seek < 0 )
        gameLogger->seek = 0;
    seek = gameLogger->seek;
    lblRefCmd->setText(QString("RefCom: %1\n RefComCnt: %2\n goals_b: %3\n goals_y: %4\n time: %5")
                       .arg(gameLogger->lastLogData.refcom)
                       .arg(gameLogger->lastLogData.refcomcnt)
                       .arg(gameLogger->lastLogData.goals_blue)
                       .arg(gameLogger->lastLogData.goals_yellow)
                       .arg(gameLogger->lastLogData.time_remaining));


    loggerMutex->unlock();
    bool typeState=false;
    qint16 DType;
    if( chbxDebugs->isChecked() ){
        loggerMutex->lock();



        for( int i=0 ; i<gameLogger->debugList.size() ; i++ ){
            DType=gameLogger->debugList.at(i).type;
            if(DType<0 && DType>-32768){
                DType+=65536;
                if((DType & type1) > 32768)
                typeState=true;

            }
            else if(DType<32768 && (DType & type)){
                typeState=true;
            }
            if( typeState || DType == 0){
                debugTexts->setTextColor(tcolor.value(DType));
                debugTexts->append(QString("%1 --> ").arg(gameLogger->counter) + gameLogger->debugList.at(i).debug);
            }
            typeState=false;
        }
        loggerMutex->unlock();
    }

    int tt = seek*(0.016666667);
    QString hh = QString("%1").arg((int)tt/3600);
    QString mm = QString("%1").arg((int)(tt%3600)/60);
    QString ss = QString("%1").arg((int)(tt%3600)%60);
    QString ms = QString("%1").arg((int)((slider->value()*0.016666 - tt)*100));
    QString currentTime = QString("%1:%2:%3:%4").arg(hh,2,'0').arg(mm,2,'0').arg(ss,2,'0').arg(ms,2,'0');

    slider->setValue(seek);
    lblTime->setText(currentTime);
}

void CLoggerWidget::clearDebugTexts(){
    debugTexts->clear();
}

void CLoggerWidget::showHideDraws(bool state){
    loggerMutex->lock();
    gameLogger->showDraws = state;
    loggerMutex->unlock();
}

void CLoggerWidget::showHideDebugs(bool state){
    loggerMutex->lock();
    gameLogger->showDebugs = state;
    loggerMutex->unlock();
}

void CLoggerWidget::debugTypeChanged(){
    type = 0;
    type1=0;
    if( chbxDebug[10]->isChecked() ){
        type |= D_GAME;
    }
    if( chbxDebug[11]->isChecked() ){
        type |= D_ALI;
    }
    if( chbxDebug[12]->isChecked() ){
        type |= D_ARASH;
    }
    if( chbxDebug[13]->isChecked() ){
        type |= D_MANI;
    }
    if( chbxDebug[14]->isChecked() ){
        type |= D_MASOOD;
    }
    if( chbxDebug[15]->isChecked() ){
        type |= D_SEPEHR;
    }
    if( chbxDebug[16]->isChecked() ){
        type |= D_MOHAMMED;
    }
    if( chbxDebug[17]->isChecked() ){
        type |= D_HOSSEIN;
    }
    if( chbxDebug[18]->isChecked() ){
        type |= D_KK;
    }
    if( chbxDebug[19]->isChecked() ){
        type |= D_ERF;
    }
    if( chbxDebug[6]->isChecked() ){
        type |= D_MAHMOOD;
    }
    if( chbxDebug[0]->isChecked() ){
        type |= D_MHMMD;
    }
    if( chbxDebug[1]->isChecked() ){
        type |= D_MAHI;
    }
    if( chbxDebug[2]->isChecked() ){
        type1 |= D_HAMED;
    }
    if( chbxDebug[3]->isChecked() ){
        type |= D_NADIA;
    }
    if( chbxDebug[4]->isChecked() ){
        type1 |= D_FATEMEH;
    }
    if( chbxDebug[5]->isChecked() ){
        type1 |= D_ATOUSA;
    }
    if( chbxDebug[7]->isChecked() ){
        type1 |= D_AHZ;
    }
    if( chbxDebug[8]->isChecked() ){
        type1 |= D_AMIN;
    }
    if( chbxDebug[9]->isChecked() ){
        type1 |= D_AMIR;
    }


}

void CLoggerWidget::replayFPSChanged(){
    int fps = txtFPS->text().toInt();
    if( fps < 1 || fps > 600 )
        return;
    loggerFPS = fps;
    loggerMutex->lock();
    gameLogger->setReplayFPS(fps);
    loggerMutex->unlock();
    txtFPS->clearFocus();
}

bool CLoggerWidget::state(){
    return isLoggerWorking;
}

CLogTagWidget::CLogTagWidget(QWidget* parent):QDialog(parent){
    WhoLogs=new QLineEdit("Nadia",this);
    WhereLogs=new QLineEdit("robocup-germany",this);
    DescriptionLogs=new QLineEdit("test plan1",this);
    teamNameLogs=new QLineEdit("parsian-skuba",this);
    TagsInThisLog=new QLineEdit("shoot-direct",this);
    WhoLogsl=new QLabel("Enter your name: ",this);
    WhereLogsl=new QLabel("Where are you logging:");
    DescriptionLogsl=new QLabel("description for this log:",this);
    teamNameLogsl=new QLabel("team Name:",this);
    TagsInThisLogl=new QLabel("Enter Some tags for this Log:",this);
    StartLog=new QPushButton("start LOG",this);
    QGridLayout *LogTagL=new QGridLayout(this);
    LogTagL->addWidget(WhoLogsl,1,1);
    LogTagL->addWidget(WhoLogs,1,2);
    LogTagL->addWidget(WhereLogsl,2,1);
    LogTagL->addWidget(WhereLogs,2,2);
    LogTagL->addWidget(DescriptionLogsl,3,1);
    LogTagL->addWidget(DescriptionLogs,3,2);
    LogTagL->addWidget(teamNameLogsl,4,1);
    LogTagL->addWidget(teamNameLogs,4,2);
    LogTagL->addWidget(TagsInThisLogl,5,1);
    LogTagL->addWidget(TagsInThisLog,5,2);
    LogTagL->addWidget(StartLog,6,2);
    this->setLayout(LogTagL);

    connect(StartLog , SIGNAL(pressed()) , this , SLOT(StartLogfunc()));
}

CLogTagWidget::~CLogTagWidget(){
}
void CLogTagWidget::StartLogfunc(){
    this->close();
    QString totalDescription;
     QChar cc = '0';
    QString baseFileName = QString("%1%2%3-%4%5%6")
            .arg(QString::number(QDate::currentDate().year()) , 4 , cc)
            .arg(QString::number(QDate::currentDate().month()) , 2 , cc)
            .arg(QString::number(QDate::currentDate().day()) , 2 , cc)
            .arg(QString::number(QTime::currentTime().hour()) , 2 , cc)
            .arg(QString::number(QTime::currentTime().minute()) , 2 , cc)
            .arg(QString::number(QTime::currentTime().second()) , 2 , cc);
    totalDescription=baseFileName+"#"+getWhoLogs()+"#"+getWhereLogs()+"#"+getDescriptionLogs()+"#"+getTagsInThisLog();
    if(getDescriptionLogs()!="test plan1"){
        baseFileName+=getDescriptionLogs();
    }

    loggerMutex->lock();
    gameLogger->setIsLogMode(true);
    gameLogger->closeLogger = false;
    gameLogger->logMode = true;
    gameLogger->openFilesToLog(baseFileName,totalDescription);
    loggerMutex->unlock();

    gameLogger->start(QThread::NormalPriority);


}
QString CLogTagWidget::getWhoLogs(){
    return this->WhoLogs->text();
}
QString CLogTagWidget::getWhereLogs(){
    return this->WhereLogs->text();
}
QString CLogTagWidget::getDescriptionLogs(){
    return this->DescriptionLogs->text();
}
QString CLogTagWidget::getTagsInThisLog(){
    return this->TagsInThisLog->text();
}



class PlayFileHightlighter : public QSyntaxHighlighter
{
public:
    PlayFileHightlighter(QTextDocument *document);
protected:
    void highlightBlock(const QString &text);
};

PlayFileHightlighter::PlayFileHightlighter(QTextDocument *document)
    : QSyntaxHighlighter(document)
{

}

void PlayFileHightlighter::highlightBlock(const QString &text)
{
    enum { NormalState = -1, InsideCStyleComment, InsideBrackets };
    int state = previousBlockState();
    int start = 0;
    int qstart = 0;
    QString tr = text.trimmed();
    for (int i = 0; i < text.length(); ++i) {
        if (state == InsideCStyleComment) {
            if (text.mid(i, 2) == "*/") {
                state = NormalState;
                setFormat(start, i - start + 2, Qt::cyan);
            }
        }
        else if (state == InsideBrackets)
        {
            if (text.at(i)==']')
            {
                setFormat(qstart, i - qstart + 1, Qt::green);
                state = NormalState;
            }
        }
        else {
            if (text.mid(i, 2) == "//") {
                setFormat(i, text.length() - i, Qt::red);
                break;
            } else if (text.mid(i, 2) == "/*") {
                start = i;
                state = InsideCStyleComment;
            }
            if (text.at(i)=='[')
            {
                state = InsideBrackets;
                qstart = i;
            }
            else if (text.at(i).isNumber())
            {
                setFormat(i, 1, Qt::cyan);
            }
            else if (text.at(i)=='(' || text.at(i)==')' || text.at(i)=='=')
            {
                setFormat(i, 1, Qt::blue);
            }
            else if (text.at(i)=='|')
            {
                setFormat(i, 1, Qt::blue);
            }
            else if (text.at(i)=='>' || text.at(i)=='<' || text.at(i)=='&')
            {
                setFormat(i, 1, Qt::darkCyan);
            }
        }
    }
    if (tr.count() > 0)
    {
        if (tr[0] == '>' || tr[0] == '<' || tr[0] == '$')
        {
            for (int i=0;i<CGameConditions::conditions.count();i++)
            {
                int k = 0;
                QString str = CGameConditions::conditions.at(i)->getName();
                while ((k=text.indexOf(str, k)) != -1)
                {
                    bool flag = true;
                    if (k>0)
                    {
                        if (text.at(k-1).isLetter())
                        {
                            flag = false;
                        }
                    }
                    if (k+str.length()<text.count()-1)
                    {
                        if (text.at(k+str.length()).isLetter())
                        {
                            flag = false;
                        }
                    }
                    if (flag) {
                        if (tr[0] == '$') setFormat(k, str.length(), QColor("red"));
                        else setFormat(k, str.length(), QColor("orange"));
                    }
                    k = k+str.length();
                }
            }
        }
    }
    for (int i=0;i<CSkills::skillsCount();i++)
    {
        int k=0;
        QString str = CSkills::skill(i)->getName();
        if (CSkills::skill(i)->level() != 2) continue;//check roles only
        if ((k=text.indexOf(str)) != -1)
        {
            bool flag = true;
            if (k>0)
            {
                if (text.at(k-1).isLetter())
                {
                    flag = false;
                }
            }
            if (k+str.length()<text.count()-1)
            {
                if (text.at(k+str.length()).isLetter())
                {
                    flag = false;
                }
            }
            if (flag) setFormat(k, str.length(), QColor("magenta"));
        }
    }
    int k = text.indexOf("else");
    if (k!=-1)
    {
        setFormat(k, 4, Qt::green);
    }
    if (tr.count()>0)
    {
        if (tr[0]=='#')
            setFormat(0, text.count(), Qt::cyan);
    }
    if (state == InsideCStyleComment)
        setFormat(start, text.length() - start, Qt::cyan);
    setCurrentBlockState(state);
}

MyTreeView::MyTreeView(QTextEdit* _txt, QWidget *parent) : QTreeView(parent)
{    
    txt = _txt;
}

void MyTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (selected.indexes().count() > 0)
    {
        if (selected.indexes().at(0).parent().parent() == QModelIndex())
        {
            QString f = selected.indexes().at(0).data().toString();
#ifdef Q_OS_MAC
            f = qApp->applicationDirPath() + QString("/../../../plays/") + f;
#else
            f = qApp->applicationDirPath() + QString("/plays/") + f;
#endif

            QFile file(f);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return;
            QTextStream in(&file);
            txt->setText(in.readAll());
            emit fileChanged(f);
        }
        //while (selected.length() > 1) selected.removeLast();
    }
}


MyTextEdit::MyTextEdit(QWidget *parent) : QTextEdit(parent)
{
    ctrl = false;
}

void MyTextEdit::keyPressEvent(QKeyEvent *e)
{    
    QTextEdit::keyPressEvent(e);
    if (e->text().count() > 0) emit modified();
    if (e->key() == Qt::Key_Control) ctrl = true;
    if (e->key() == Qt::Key_Space)
    {
        if (ctrl)
        {
            QString str = textCursor().block().text();
            int k = textCursor().position() - textCursor().block().position();
            int i = 0;
            for (i=k-1;i>=0;i--)
            {
                if (!str[i].isLetter()) break;
            }
            i++;
            str = str.mid(i, k - i + 1);
            int j = -1;
            int occurances = 0;
            for (int q=0;q<CSkills::skillsCount();q++)
            {
                QString s = CSkills::skill(q)->getName();
                if (s.indexOf(str)==0)
                {
                    occurances ++;
                    j = q;
                }
            }
            if (occurances == 1)
            {
                this->insertPlainText(CSkills::skill(j)->getName().right(CSkills::skill(j)->getName().length()-(k-i)));
            }
        }
    }
}

void MyTextEdit::keyReleaseEvent(QKeyEvent *e)
{
    QTextEdit::keyReleaseEvent(e);
    if (e->key() == Qt::Key_Control) ctrl = false;
}


//CPlaysWidget::CPlaysWidget(bool _treeView)
//{
//	treeView = _treeView;
//    QGridLayout* layout = new QGridLayout(this);
//    m = new QStandardItemModel(this);
//    lblPlay = new QLabel(this);
//    lblCondition = new QLabel(this);
////    lblCondition->setMaximumWidth(100);
//    btnSave = new QPushButton("Save", this);
//    btnLoad = new QPushButton("Load", this);
//    txt = new MyTextEdit(this);//MyTextEdit(this);
//	if (treeView)
//	{
//		t = new MyTreeView(txt, this);
//		t->setModel(m);
//		layout->addWidget(t,0,0,1,2);
//	}
//    layout->addWidget(lblPlay,1,0,1,2);
//    layout->addWidget(lblCondition,2,0,1,2);
//    layout->addWidget(txt,3,0,1,2);
//    layout->addWidget(btnSave,4,0,1,1);
//    layout->addWidget(btnLoad,4,1,1,1);
//    btnSave->setDisabled(true);
//    btnLoad->setDisabled(true);
//    QAction* act = new QAction(this);
//    act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
//    PlayFileHightlighter* highlighter = new PlayFileHightlighter(txt->document());
//    connect(txt, SIGNAL(modified()), this, SLOT(playModified()));
//    connect(t, SIGNAL(fileChanged(QString)), this, SLOT(playFileChanged(QString)));
//    connect(btnSave, SIGNAL(clicked()), this, SLOT(savePlay()));
//    connect(act, SIGNAL(triggered()), this, SLOT(savePlay()));
//    connect(btnLoad, SIGNAL(clicked()), this, SLOT(loadPlay()));
//    txt->setFont(QFont("courier new",10));
//    txt->setFontWeight(QFont::Bold);
//    txt->setTextColor("lightgray");
//    txt->setStyleSheet("background-color:black");
//}

//void CPlaysWidget::reloadCoach()
//{
//    coach->reload();
//    m->clear();
//    initialize();
//}

//void CPlaysWidget::loadPlay()
//{
//    QString f = playfilename;
//    QFile file(f);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//        return;
//    QTextStream in(&file);
//    txt->setText(in.readAll());
//    btnSave->setDisabled(true);
//    reloadCoach();
//}

//void CPlaysWidget::savePlay()
//{
//    QString f = playfilename;
//    QFile file(f);
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//        return;
//    QTextStream out(&file);
//    out << txt->toPlainText();
//    file.close();
//    reloadCoach();
//    btnSave->setDisabled(true);
//}

//void CPlaysWidget::playModified()
//{
//    btnSave->setDisabled(false);
//}

//void CPlaysWidget::playFileChanged(QString filename)
//{
//    playfilename = filename;
//    btnSave->setDisabled(true);
//    btnLoad->setDisabled(false);
//}

//void CPlaysWidget::initialize()
//{
//    QStandardItem *parentItem = m->invisibleRootItem();
//    QStandardItem *nonPlayOnsItem = new QStandardItem("non-play-ons");
//    parentItem->appendRow(nonPlayOnsItem);
//    for (int i=0;i<coach->getNonPlayOn().getPlayList().count();i++)
//    {
//        QStandardItem *s = new QStandardItem(coach->getNonPlayOn().getPlayList()[i]->name);
//        nonPlayOnsItem->appendRow(s);
//        addPlay(s, coach->getNonPlayOn().getPlayList()[i]);
//    }

//    QStandardItem *attackItem = new QStandardItem("attack");
//    parentItem->appendRow(attackItem);
//    for (int i=0;i<coach->getOffense().getAttackPlan().getPlayList().count();i++)
//    {
//        QStandardItem *s = new QStandardItem(coach->getOffense().getAttackPlan().getPlayList()[i]->name);
//        attackItem->appendRow(s);
//        addPlay(s, coach->getOffense().getAttackPlan().getPlayList()[i]);
//    }

//}
//void CPlaysWidget::addPlay(QStandardItem *parentItem, Play *play)
//{
//    play->ui_ptr = (void*) parentItem;
//    for (int j=0;j<play->l.count();j++)
//    {
//        if (play->l[j].subplay == -1)
//        {
//            QString tsk;
//            parentItem->appendRow(new QStandardItem(QString("%1(%2)%3").arg(play->l[j].role->getName()).arg(play->l[j].params.join(",")).arg(tsk) ));
//        }
//        else {
//            QStandardItem *s = new QStandardItem(QString("%1").arg(play->subPlays[play->l[j].subplay]->name) );
//            parentItem->appendRow(s);
//            addPlay(s, play->subPlays[play->l[j].subplay]);
//        }
//    }

//}

//void CPlaysWidget::update()
//{
//    lblPlay->setText(knowledge->executingPlays.join("->"));
//}

//---------------------------------------------------

//CNewRectDialog::CNewRectDialog()
//{
//    this->setWindowTitle("Save The Rect");
//    QGridLayout *layout = new QGridLayout(this);
//    nameL = new QLineEdit(this);
//    okBtn  = new QPushButton("Save", this);
//    cancelBtn  = new QPushButton("Cancel", this);
//    layout->addWidget(nameL,0,0,1,2);
//    layout->addWidget(okBtn,1,0);
//    layout->addWidget(cancelBtn,1,1);
//    connect(okBtn, SIGNAL(clicked()), this, SLOT(saveClicked()));
//    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(cancelClicked()));
//}

//void CNewRectDialog::saveClicked()
//{
//    name = nameL->text();
//    done(QDialog::Accepted);
//}

//void CNewRectDialog::cancelClicked()
//{
//    name = nameL->text();
//    done(QDialog::Rejected);
//}


//CRectsWidget::CRectsWidget(CMonitorWidget *_monitor)
//{
//    monitor = _monitor;
//    saveBtn = new QPushButton("save",this);
//    rectNameDlg = new CNewRectDialog();
//    QGridLayout* layout = new QGridLayout( this);
//    rectsFile = new QFile("rects");
//    layout->addWidget( saveBtn, 0,0);
//    connect( saveBtn, SIGNAL(clicked()), this, SLOT(saveSlt()));

//}

//void CRectsWidget::update()
//{
//    connect( monitor, SIGNAL(mousePosChanged(Vector2D,int)), this, SLOT(startSelecting(Vector2D,int)));
//    connect( monitor, SIGNAL(mouseReleasedInmonitor()), this, SLOT(endSelecting()));
//    //    glEnable(GL_BLEND);
//    QColor color (0, 127, 127, 100);
//    draw( selectionRect, color, true);
//}

//void CRectsWidget::startSelecting(Vector2D _point, int b)
//{
//    if ( b == 1)
//        startPoint = _point;
//    connect( monitor, SIGNAL(mouseMovePos(Vector2D)), this, SLOT(selecting(Vector2D)));
//    topleft = startPoint;
//}

//void CRectsWidget::selecting(Vector2D point)
//{
//    topleft.x =  ( startPoint . x < point.x) ? startPoint.x : point.x;
//    topleft.y =  ( startPoint . y > point.y) ? startPoint.y : point.y;
//    selectionRect.setTopLeft( topleft);
//    selectionRect.setLength( fabs(startPoint.x - point.x));
//    selectionRect.setWidth( fabs(startPoint.y - point.y));
//}

//void CRectsWidget::endSelecting()
//{
//    disconnect( monitor, SIGNAL(mouseMovePos(Vector2D)), this, SLOT(selecting(Vector2D)));
//}

//void CRectsWidget::saveSlt()
//{
//    QTextStream* stream;
//    if (rectNameDlg->exec()==QDialog::Accepted && rectsFile->open(QFile::Append))
//    {
//        stream = new QTextStream(rectsFile);
//        (*stream) << rectNameDlg->name << ":" << "Vector2D(" << selectionRect.topLeft().x << "," << selectionRect.topLeft().y << ")" << "," << "Vector2D(" << selectionRect.bottomRight().x << "," << selectionRect.bottomRight().y << ")" << "\n";
//        stream->flush();
//    }
//}


//CKnowledgeVarsWidget::CKnowledgeVarsWidget()
//{
//    QGridLayout* layout = new QGridLayout(this);
//    label = new QLabel(this);
//    layout->addWidget(label,0,0);
//}

//void CKnowledgeVarsWidget::update()
//{
//    QString txt;
//    for (QMap<QString, QString>::iterator  i=knowledge->variables.begin();
//                                          i!=knowledge->variables.end();i++)
//    {
//        txt = QString("%1%2=%3\n").arg(txt).arg(i.key()).arg(i.value());
//    }
//	txt += QString("\nChip phase : %1\n").arg(knowledge->cornerChipPhase);
//    label->setText(txt);
//}



//CBehavioursWidget::CBehavioursWidget()
//{
//    cmb = new QComboBox(this);
//    txt = new QTextEdit(this);
//    dbg = new QTextEdit(this);
//    btn = new QPushButton("Save", this);
//    QGridLayout* layout = new QGridLayout(this);
//    layout->addWidget(txt, 1, 1);
//    layout->addWidget(btn, 2, 1);
//    layout->addWidget(cmb, 3, 1);
//    layout->addWidget(dbg, 4, 1);
//    QDir sourceDir;
//    sourceDir.setPath(qApp->applicationDirPath()+QString("/behaviours/"));
//    QStringList filter;
//    filter << "*.behaviours";
//    sourceDir.setNameFilters(filter);
//    sourceDir.setSorting(QDir::Name);
//    cmb->clear();
//    QFileInfoList list = sourceDir.entryInfoList();
//    for (int i = 0; i < list.size(); i++) {
//        QFileInfo fileInfo = list.at(i);
//        cmb->addItem(fileInfo.baseName());
//    }
//    QFile file(qApp->applicationDirPath()+QString("/behaviours/current_behaviours"));
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
//    QByteArray line = file.readLine();
//    QString str(line);
//    file.close();
//    cmb->setCurrentIndex(cmb->findText(str.trimmed().toLower()));
//    load();
//    connect(cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(load()));
//    connect(btn, SIGNAL(clicked()), this, SLOT(save()));
//    connect(txt, SIGNAL(textChanged()), this, SLOT(changed()));

//    btn->setDisabled(true);
//}

//void CBehavioursWidget::load()
//{
//    CBehaviours::loadValues(qApp->applicationDirPath()+QString("/behaviours/")+cmb->currentText()+QString(".behaviours"));
//    QString str;
//    for (QMap<QString, BehaviourValue>::iterator i=CBehaviours::values.begin();i!=CBehaviours::values.end();i++)
//    {
//        str += QString("%1=%2,%3\n").arg(i.key()).arg(i.value().reward).arg(i.value().penalty);
//    }
//    txt->setText(str);
//    btn->setDisabled(true);
//}


//void CBehavioursWidget::save()
//{
//    QFile file(qApp->applicationDirPath()+QString("/behaviours/")+cmb->currentText()+QString(".behaviours"));
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//        return;
//    QTextStream out(&file);
//    out << txt->toPlainText();
//    file.close();
//    load();
//}

//void CBehavioursWidget::update()
//{
//    if (dbgString != knowledge->behavioursDebug)
//    {
//        dbgString = knowledge->behavioursDebug;
//        dbg->setText(dbgString);
//    }
//}

//void CBehavioursWidget::changed()
//{
//    btn->setDisabled(false);
//}

//CBehavioursWidget::~CBehavioursWidget()
//{
//    QFile file(qApp->applicationDirPath()+QString("/behaviours/current_behaviours"));
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//        return;
//    QTextStream out(&file);
//    out << cmb->currentText();
//    file.close();
//    delete cmb;
//    delete txt;
//    delete btn;
//}


//CConditionCheckerWidget::CConditionCheckerWidget()
//{
//	conditionTxt = new QLineEdit("always",this);
//	conditionLbl = new QLabel("true",this);
//	btn = new QPushButton("check",this);
//	QVBoxLayout* l = new QVBoxLayout(this);
//	l->addWidget(conditionTxt);
//	l->addWidget(conditionLbl);
//	l->addWidget(btn);
//	connect(btn, SIGNAL(clicked()), this, SLOT(check()));
//}

//void CConditionCheckerWidget::check()
//{
////	debug("CCCCc", D_ERROR);
//	int k = conditionTxt->text().indexOf('(');
//	QString cond = conditionTxt->text();
//	QStringList p;
//	bool flag = false;
//	if (k != -1)
//	{
//		cond = cond.left(k);
//		QString s = conditionTxt->text().right(conditionTxt->text().length() - k);
//		k = s.indexOf(')');
//		if (k != -1)
//		{
//			s = s.left(k);
//			p = s.split(',');
//			flag = true;
//		}
//	}
//	else flag = true;
//	if (flag)
//	{
//		if (CGameConditions::check(cond, p))
//		{
//			conditionLbl->setText("true");
//		}
//		else
//			conditionLbl->setText("false");
//	}
//	else conditionLbl->setText("invalid");
//}

//CConditionCheckerWidget::~CConditionCheckerWidget()
//{

//}

CAgentsWidget::CAgentsWidget(CAgent**_agents)
{
    this->setFixedSize(350,200);
    this->setStyleSheet("color:black; padding: 0; border: 1px solid black; border-radius: 5px; width: 20px");
    infoCount = 6;
    dontUpdate = false;
    agents = _agents;
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(2);

    QStringList headerLabels;
    headerLabels << "Agnt" << "Gyro" << "Kick" << "Chip" << "Spin" << "In";

    lbls[0] = new QLabel(headerLabels.at(0) , this);
    lbls[0]->setStyleSheet("background-color:blue; color:white;");
    lbls[0]->setFixedSize(42,42);
    layout->addWidget(lbls[0] , 0 , 0 , 3 , 1);
    for( int i=1 ; i<infoCount ; i++ ){
        lbls[i] = new QLabel(headerLabels.at(i) , this);
        lbls[i]->setStyleSheet("background-color:blue; color:white;");
        layout->addWidget(lbls[i] , i+2 , 0);
    }

    btnGroupGyro = new QButtonGroup();
    btnGroupKick = new QButtonGroup();
    btnGroupChip = new QButtonGroup();
    btnGroupSpin = new QButtonGroup();
    btnGroupOn = new QButtonGroup();
    btnGroupIn = new QButtonGroup();

    painter = new QPainter();

    for( int i=0 ; i<_MAX_NUM_PLAYERS ; i++ ){
        robotsBlub[i] = new QPixmap("./icons/head.png");
        lblRobotsBlub[i] = new QLabel("");
        lblRobotsBlub[i]->setStyleSheet("border: 0px;");
        lblRobotsBlub[i]->setPixmap(*robotsBlub[i]);
        layout->addWidget(lblRobotsBlub[i] , 0 , i+1 , 3 , 1 );

        btnGyro[i] = new QPushButton("Gy" , this);
        btnGroupGyro->addButton(btnGyro[i] , i);
        layout->addWidget(btnGyro[i] , 3 , i+1);

        btnKick[i] = new QPushButton("Kc" , this);

        btnGroupKick->addButton(btnKick[i] , i);
        layout->addWidget(btnKick[i] , 4 , i+1);

        btnChip[i] = new QPushButton("Ch" , this);
        btnGroupChip->addButton(btnChip[i] , i);
        layout->addWidget(btnChip[i] , 5 , i+1);

        btnSpin[i] = new QPushButton("Sp" , this);
        btnGroupSpin->addButton(btnSpin[i] , i);
        layout->addWidget(btnSpin[i] , 6 , i+1);

        btnOn[i] = new QPushButton(QString("On%1").arg(i) , this);
        btnGroupOn->addButton(btnOn[i] , i);
        layout->addWidget(btnOn[i] , 19 + i/8 , i%8);

        btnIn[i] = new QPushButton("In" , this);
        btnGroupIn->addButton(btnIn[i] , i);
        layout->addWidget(btnIn[i] , 7 , i+1);
    }

    int stRow = 18;

    lblStarter = new QLabel("St:" , this);
    lblStarter->setStyleSheet("background-color:blue; color:white;");
    layout->addWidget(lblStarter , stRow , 0 , 1 , 1);

    cmbStarter = new QComboBox();
    for( int i=0 ; i<12 ; i++ )
        cmbStarter->addItem(QString("%1").arg(i));
    cmbStarter->setStyleSheet("background-color:green;");
    for( int i=0 ; i<_MAX_NUM_PLAYERS ; i++ )
        if( agents[i]->starter ){
            cmbStarter->setCurrentIndex(i);
            break;
        }
    layout->addWidget(cmbStarter , stRow , 1 , 1 , 1);

    btnWarmup = new QPushButton("Warmup" , this);
    btnWarmup->setStyleSheet("background-color:brown; color:white;");
    layout->addWidget(btnWarmup , stRow , 2 , 1 , 2);

    btnLoadConf = new QPushButton("Load Set.");
    btnLoadConf->setStyleSheet("background-color:brown; color:white;");
    layout->addWidget(btnLoadConf , stRow , 4 , 1 , 2);

    btnSaveConf = new QPushButton("Save Set.");
    btnSaveConf->setStyleSheet("background-color:brown; color:white;");
    layout->addWidget(btnSaveConf , stRow , 6 , 1 , 2);

    connect(btnLoadConf,SIGNAL(clicked()),this,SLOT(loadConf()));
    connect(btnSaveConf,SIGNAL(clicked()),this,SLOT(saveConf()));
    connect(btnWarmup , SIGNAL(clicked()) , this , SLOT(setWarmup()));
    connectToSlot();
    coach = NULL;
    setLayout(layout);
    loadConf();
}

CAgentsWidget::~CAgentsWidget()
{
    delete btnSaveConf;
    delete btnLoadConf;
    delete btnWarmup;
    delete cmbStarter;
    delete lblStarter;
    for( int i=_MAX_NUM_PLAYERS-1 ; i>=0 ; i-- ){
        delete btnIn[i];
        delete btnOn[i];
        delete btnSpin[i];
        delete btnChip[i];
        delete btnKick[i];
        delete btnGyro[i];
        delete lblRobotsBlub[i];
        delete robotsBlub[i];
    }
    delete painter;
    delete btnGroupOn;
    delete btnGroupIn;
    delete btnGroupSpin;
    delete btnGroupChip;
    delete btnGroupKick;
    delete btnGroupGyro;
    for( int i=infoCount-1 ; i>=0 ; i-- )
        delete lbls[i];
}

void CAgentsWidget::update()
{
#ifdef Q_OS_MAC
    return;
#endif

    if( dontUpdate )
        return;
    disConnectFromSlot();

    for( int i=0 ; i<_MAX_NUM_PLAYERS ; i++ ){
        int id = i;
        btnOn[id]->setStyleSheet(agents[id]->onOffState ? "background-color:lightgreen" : "background-color: gray;" );
        on[id] = agents[id]->onOffState ? 1 : 2;
    }

    for(int i=0; i<_MAX_NUM_PLAYERS; i++){
        int id = i;

        if( agents[id]->starter )
            cmbStarter->setCurrentIndex(id);

        if( agents[i]->notVisible() ){
            hideColNumber(i);
            continue;
        }
        showColNumber(i);

        btnIn[id]->setStyleSheet(agents[id]->inOutState ? "background-color:lightgreen" : "background-color:gray;");
        in[id] = agents[id]->inOutState ? 1 : 2;

        btnGyro[id]->setStyleSheet(agents[id]->abilities.hasGyro ? "background-color:lightgreen" : "background-color:gray;");
        gyro[id] = agents[id]->abilities.hasGyro ? 1 : 2;

        btnKick[id]->setStyleSheet(agents[id]->abilities.canKick ? "background-color:lightgreen" : "background-color:gray;");
        kick[id] = agents[id]->abilities.canKick ? 1 : 2;

        btnChip[id]->setStyleSheet(agents[id]->abilities.canChip ? "background-color:lightgreen" : "background-color:gray;");
        chip[id] = agents[id]->abilities.canChip ? 1 : 2;

        btnSpin[id]->setStyleSheet(agents[id]->abilities.canSpin ? "background-color:lightgreen" : "background-color:gray;");
        spin[id] = agents[id]->abilities.canSpin ? 1 : 2;

        QPixmap tempPix = *robotsBlub[id];
        painter->begin(&tempPix);

        QPen pen;
        pen.setColor("darkcyan");
        pen.setWidth(2);
        painter->setPen(pen);
        int x = tempPix.width()/2 , y = tempPix.height()/2;

        Vector2D actualVel = agents[id]->vel();
        actualVel.rotate(-1*agents[id]->dir().dir().degree());
        x = tempPix.width()/2 , y = tempPix.height()/2;
        pen.setColor("red");
        painter->setPen(pen);

        QFont font;
        font.setPointSize(8);
        font.setWeight(10);
        font.setFamily("Arial");
        painter->setFont(font);
        if ( knowledge->defenseAgents.contains(agents[id])){
            pen.setColor(Qt::red);
            painter->setPen(pen);
            painter->drawText(QPoint(12,33) , "Def");
        }
        else if ( true ){
            pen.setColor(Qt::blue);
            painter->setPen(pen);
            painter->drawText(QPoint(12,33) , "Att");
        }
        else{
            pen.setColor("purple");
            painter->setPen(pen);
            painter->drawText(QPoint(12,33) , "Mid");
        }

        pen.setColor("black");
        font.setPointSize(10);
        painter->setPen(pen);
        painter->setFont(font);
        painter->drawText(QPoint(0,42) , QString("%1").arg(id));

        pen.setColor("magenta");
        pen.setWidth(2);
        painter->setPen(pen);
        painter->drawArc(4 , 4 , 32 , 32 , 90*16 , 16*(agents[id]->angularVel()/2047)*180);

        pen.setColor("darkgreen");
        pen.setWidth(2);
        painter->setPen(pen);
        painter->drawArc(0 , 0 , 40 , 40 , 90*16 , 16*(agents[id]->vangular/2047)*180);

        painter->end();
        lblRobotsBlub[id]->setPixmap(tempPix);
    }
    connectToSlot();
}

void CAgentsWidget::changed()
{
    dontUpdate = true;
    for(int i=0; i<_MAX_NUM_PLAYERS; i++)
    {
        int id = i;
        //		agents[id]->commandID = cmbCId[id]->currentIndex();
        agents[id]->abilities.hasGyro = (gyro[id] == 1 ? true : false);
        agents[id]->abilities.canKick = (kick[id] == 1 ? true : false);
        agents[id]->abilities.canChip = (chip[id] == 1 ? true : false);
        agents[id]->abilities.canSpin = (spin[id] == 1 ? true : false);
        agents[id]->onOffState = (on[id] == 1 ? true : false);
        agents[id]->inOutState = (in[id] == 1 ? true : false);
        if( cmbStarter->currentIndex() != id )
            agents[id]->starter = false ;
        else
            agents[id]->starter = true;
    }
    dontUpdate = false;
    saveConf();
}

void CAgentsWidget::setCoach(CCoach *_coach)
{
    coach = _coach;
}

void CAgentsWidget::setWarmup()
{
    if(knowledge->warmup)
        knowledge->warmup = false;
    else
        knowledge->warmup = true;
}

void CAgentsWidget::loadConf()
{
    QFile in("robotsSetting.csv");
    if(!in.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Error in opening robots Settings file!";
        return;
    }
    QByteArray ld;
    QList <QByteArray> ls;
    while( !in.atEnd() ){
        ld = in.readLine();
        ls = ld.split(' ');
        if( ls.count() != 9 ){
            debug(QString("Error loading agents info. %1").arg(ls.count()) , 1 , QColor("red"));
            return;
        }
        int ID = ls[0].toInt();
        agents[ID]->abilities.hasGyro = ls[1].toInt();
        agents[ID]->abilities.canKick = ls[2].toInt();
        agents[ID]->abilities.canChip = ls[3].toInt();
        agents[ID]->abilities.canSpin = ls[4].toInt();
        agents[ID]->onOffState = ls[5].toInt();
        agents[ID]->inOutState = ls[6].toInt();
        agents[ID]->starter = ls[7].toInt();
    }
    in.close();
}

void CAgentsWidget::saveConf()
{
    QFile out("robotsSetting.csv");
    out.open(QIODevice::WriteOnly | QIODevice::Text);
    if( out.isOpen() == true ){
        for(int i=0 ; i<_MAX_NUM_PLAYERS ; i++){
            out.write((QString("%1 %2 %3 %4 %5 %6 %7 %8 \n").arg(i).arg(agents[i]->abilities.hasGyro).arg(agents[i]->abilities.canKick).arg(agents[i]->abilities.canChip).arg(agents[i]->abilities.canSpin).arg(agents[i]->onOffState).arg(agents[i]->inOutState).arg(agents[i]->starter)).toStdString().c_str());
        }
        out.close();
    }
}

void CAgentsWidget::hideColNumber(int i){
    lblRobotsBlub[i]->hide();
    btnGyro[i]->hide();
    btnKick[i]->hide();
    btnChip[i]->hide();
    btnSpin[i]->hide();
    //	btnOn[i]->hide();
    btnIn[i]->hide();
}

void CAgentsWidget::showColNumber(int i){
    lblRobotsBlub[i]->show();
    btnGyro[i]->show();
    btnKick[i]->show();
    btnChip[i]->show();
    btnSpin[i]->show();
    //	btnOn[i]->show();
    btnIn[i]->show();
}

void CAgentsWidget::connectToSlot(){
    connect(btnGroupGyro, SIGNAL(buttonClicked(int)) , this , SLOT(gyroChange(int)));
    connect(btnGroupKick , SIGNAL(buttonClicked(int)) , this , SLOT(kickChange(int)));
    connect(btnGroupChip , SIGNAL(buttonClicked(int)) , this , SLOT(chipChange(int)));
    connect(btnGroupSpin , SIGNAL(buttonClicked(int)) , this , SLOT(spinChange(int)));
    connect(btnGroupOn , SIGNAL(buttonClicked(int)) , this , SLOT(onChange(int)));
    connect(cmbStarter , SIGNAL(currentIndexChanged(int)) , this , SLOT(starterChange(int)));
    connect(btnGroupIn , SIGNAL(buttonClicked(int)) , this , SLOT(inChange(int)));
}

void CAgentsWidget::disConnectFromSlot(){
    disconnect(btnGroupGyro , SIGNAL(buttonClicked(int)) , this , SLOT(gyroChange(int)));
    disconnect(btnGroupKick , SIGNAL(buttonClicked(int)) , this , SLOT(kickChange(int)));
    disconnect(btnGroupChip , SIGNAL(buttonClicked(int)) , this , SLOT(chipChange(int)));
    disconnect(btnGroupSpin , SIGNAL(buttonClicked(int)) , this , SLOT(spinChange(int)));
    disconnect(btnGroupOn , SIGNAL(buttonClicked(int)) , this , SLOT(onChange(int)));
    disconnect(cmbStarter , SIGNAL(currentIndexChanged(int)) , this , SLOT(starterChange(int)));
    disconnect(btnGroupIn , SIGNAL(buttonClicked(int)) , this , SLOT(inChange(int)));
}

void CAgentsWidget::gyroChange( int idx ){
    if( gyro[idx] == 1 )
        btnGyro[idx]->setStyleSheet("background-color:gray;") , gyro[idx] = 2;
    else
        btnGyro[idx]->setStyleSheet("background-color: lightgreen;") , gyro[idx] = 1;
    changed();
}

void CAgentsWidget::kickChange( int idx ){
    if( kick[idx] == 1 )
        btnKick[idx]->setStyleSheet("background-color:gray;") , kick[idx] = 2;
    else
        btnKick[idx]->setStyleSheet("background-color: lightgreen;") , kick[idx] = 1;
    changed();
}

void CAgentsWidget::chipChange( int idx ){
    if( chip[idx] == 1 )
        btnChip[idx]->setStyleSheet("background-color:gray;") , chip[idx] = 2;
    else
        btnChip[idx]->setStyleSheet("background-color: lightgreen;") , chip[idx] = 1;
    changed();
}

void CAgentsWidget::spinChange( int idx ){
    if( spin[idx] == 1 )
        btnSpin[idx]->setStyleSheet("background-color:gray;") , spin[idx] = 2;
    else
        btnSpin[idx]->setStyleSheet("background-color: lightgreen;") , spin[idx] = 1;
    changed();
}

void CAgentsWidget::onChange( int idx ){
    if( on[idx] == 1 )
        btnOn[idx]->setStyleSheet("background-color:gray;") , on[idx] = 2;
    else
        btnOn[idx]->setStyleSheet("background-color: lightgreen;") , on[idx] = 1;
    changed();
}

void CAgentsWidget::inChange( int idx ){
    if( in[idx] == 1 )
        btnIn[idx]->setStyleSheet("background-color:gray;") , in[idx] = 2;
    else
        btnIn[idx]->setStyleSheet("background-color: lightgreen;") , in[idx] = 1;
    changed();
}

void CAgentsWidget::starterChange( int idx ){
    changed();
}
