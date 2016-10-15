#include <skills.h>
#include <QLabel>
#include <QGridLayout>
#include <QValidator>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

#include <simulation/simulator.h>


CSkillConfigWidget* CSkillGotoPosMV::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillGotoPosMV(NULL);
    w->skill->configWidget = w;
    QDoubleValidator* XValidator = new QDoubleValidator(parent);
    XValidator->setRange(-1.0 * _FIELD_WIDTH / 2.0, _FIELD_WIDTH / 2.0, 1);
    QDoubleValidator* YValidator = new QDoubleValidator(parent);
    YValidator->setRange(-1.0 * _FIELD_HEIGHT / 2.0, _FIELD_HEIGHT / 2.0, 1);
    QDoubleValidator* TValidator = new QDoubleValidator(parent);
    TValidator->setRange(-180, 180, 1);
    w->checkBoxs.append(new QCheckBox("Follow Mouse"));
    w->checkBoxs[0]->setChecked(true);
    QLabel* lblTargetX = new QLabel("X : ");
    QLabel* lblTargetY = new QLabel("Y : ");
    QLabel* lblTargetTeta = new QLabel("Teta : ");
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits[0]->setValidator(XValidator);
    w->lineEdits[1]->setValidator(YValidator);
    w->lineEdits[2]->setValidator(TValidator);
    QGridLayout *layoutTarget = new QGridLayout;
    layoutTarget->addWidget(lblTargetX, 0, 0);
    layoutTarget->addWidget(w->lineEdits[0], 0, 1);
    layoutTarget->addWidget(lblTargetY, 0, 2);
    layoutTarget->addWidget(w->lineEdits[1], 0, 3);
    layoutTarget->addWidget(lblTargetTeta, 0, 4);
    layoutTarget->addWidget(w->lineEdits[2], 0, 5);
    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->addLayout(layoutTarget, 4);
    layoutMain->addWidget(w->checkBoxs[0]);
    w->setLayout(layoutMain);
    return w;
}

void CSkillGotoPosMV::generateFromConfig(CAgent *a)
{

    if (configWidget->checkBoxs[0]->isChecked())
    {
        target = * (configWidget->mousePos);
    }
    else {
        target.x = configWidget->lineEdits[0]->text().toDouble();
        target.y = configWidget->lineEdits[1]->text().toDouble();
    }

    init(a , target , Vector2D(0,0));

    if (lastMousePos != target)
    {
        lastMousePos = target;
        profile = new CMotionProfile(agent);
        profile->init(a->id());
    }
}



CSkillConfigWidget* CSkillGotoPoint::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillGotoPoint(NULL);
    w->skill->configWidget = w;
    QDoubleValidator* XValidator = new QDoubleValidator(parent);
    XValidator->setRange(-1.0 * _FIELD_WIDTH / 2.0, _FIELD_WIDTH / 2.0, 1);
    QDoubleValidator* YValidator = new QDoubleValidator(parent);
    YValidator->setRange(-1.0 * _FIELD_HEIGHT / 2.0, _FIELD_HEIGHT / 2.0, 1);
    QDoubleValidator* TValidator = new QDoubleValidator(parent);
    TValidator->setRange(-180, 180, 1);
    w->checkBoxs.append(new QCheckBox("Follow Mouse"));
    QLabel* lblTargetX = new QLabel("X : ");
    QLabel* lblTargetY = new QLabel("Y : ");
    QLabel* lblTargetTeta = new QLabel("Teta : ");
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits[0]->setValidator(XValidator);
    w->lineEdits[1]->setValidator(YValidator);
    w->lineEdits[2]->setValidator(TValidator);
    QGridLayout *layoutTarget = new QGridLayout;
    layoutTarget->addWidget(lblTargetX, 0, 0);
    layoutTarget->addWidget(w->lineEdits[0], 0, 1);
    layoutTarget->addWidget(lblTargetY, 0, 2);
    layoutTarget->addWidget(w->lineEdits[1], 0, 3);
    layoutTarget->addWidget(lblTargetTeta, 0, 4);
    layoutTarget->addWidget(w->lineEdits[2], 0, 5);
    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->addLayout(layoutTarget, 4);
    layoutMain->addWidget(w->checkBoxs[0]);
    w->setLayout(layoutMain);

    return w;
}

#include <profiler.h>

void CSkillGotoPoint::generateFromConfig(CAgent *a)
{
    agent = a;
    pos1 = a->pos();
    dir1 = a->dir();
    vel1 = a->vel();
    if (configWidget->checkBoxs[0]->isChecked())
    {
        targetPos = * (configWidget->mousePos);
        targetDir = Vector2D::unitVector(configWidget->lineEdits[2]->text().toDouble());
    }
    else {
        targetPos.x = configWidget->lineEdits[0]->text().toDouble();
        targetPos.y = configWidget->lineEdits[1]->text().toDouble();
        targetDir = Vector2D::unitVector(configWidget->lineEdits[2]->text().toDouble());
    }


    this->setDynamicStart(true);
    this->init( getFinalPos(), Vector2D(0,0));
    this->setLookAt(wm->ball->pos);

}




CSkillConfigWidget* CSkillGotoPointAvoid::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillGotoPointAvoid(NULL);
    w->skill->configWidget = w;
    QDoubleValidator* XValidator = new QDoubleValidator(parent);
    XValidator->setRange(-1.0 * _FIELD_WIDTH / 2.0, _FIELD_WIDTH / 2.0, 1);
    QDoubleValidator* YValidator = new QDoubleValidator(parent);
    YValidator->setRange(-1.0 * _FIELD_HEIGHT / 2.0, _FIELD_HEIGHT / 2.0, 1);
    QDoubleValidator* TValidator = new QDoubleValidator(parent);
    TValidator->setRange(-180, 180, 1);
    w->checkBoxs.append(new QCheckBox("Follow Mouse"));
    QLabel* lblTargetX = new QLabel("X : ");
    QLabel* lblTargetY = new QLabel("Y : ");
    QLabel* lblTargetTeta = new QLabel("Teta : ");
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits[0]->setValidator(XValidator);
    w->lineEdits[1]->setValidator(YValidator);
    w->lineEdits[2]->setValidator(TValidator);
    QGridLayout *layoutTarget = new QGridLayout;
    layoutTarget->addWidget(lblTargetX, 0, 0);
    layoutTarget->addWidget(w->lineEdits[0], 0, 1);
    layoutTarget->addWidget(lblTargetY, 0, 2);
    layoutTarget->addWidget(w->lineEdits[1], 0, 3);
    layoutTarget->addWidget(lblTargetTeta, 0, 4);
    layoutTarget->addWidget(w->lineEdits[2], 0, 5);
    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->addLayout(layoutTarget, 4);
    layoutMain->addWidget(w->checkBoxs[0]);
    w->setLayout(layoutMain);
    return w;
}

#include <profiler.h>

void CSkillGotoPointAvoid::generateFromConfig(CAgent *a)
{
    agent = a;
    avoidPenaltyArea = true;
    pos1 = a->pos();
    dir1 = a->dir();
    vel1 = a->vel();
    if (configWidget->checkBoxs[0]->isChecked())
    {
        targetPos = * (configWidget->mousePos);
        targetDir = Vector2D::unitVector(configWidget->lineEdits[2]->text().toDouble());
    }
    else {
        targetPos.x = configWidget->lineEdits[0]->text().toDouble();
        targetPos.y = configWidget->lineEdits[1]->text().toDouble();
        targetDir = Vector2D::unitVector(configWidget->lineEdits[2]->text().toDouble());
    }
    timeStarted = CProfiler::getTime();
    timeEstimated = simulator->timeNeededForGotoPoint(targetPos,targetDir,0.05,10,a->id());

    avoidPenaltyArea = true;
    this->setADiveMode(false);
    //	setFinalVel(Vector2D(1,0.5));
    //this->setFastW(true);
    //    this->lookat=wm->ball->pos;
    //    this->setNoAvoid(true);
    //    this->setNoAvoid(true);
    //vel2 = Vector2D(2,0);
}



CSkillConfigWidget* CSkillIntercept::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillIntercept(NULL);
    w->skill->configWidget = w;
    QDoubleValidator* XValidator = new QDoubleValidator(parent);
    XValidator->setRange(-1.0 * _FIELD_WIDTH / 2.0, _FIELD_WIDTH / 2.0, 1);
    QDoubleValidator* YValidator = new QDoubleValidator(parent);
    YValidator->setRange(-1.0 * _FIELD_HEIGHT / 2.0, _FIELD_HEIGHT / 2.0, 1);
    QDoubleValidator* VXValidator = new QDoubleValidator(parent);
    VXValidator->setRange(-3.0, 3.0, 1);
    QDoubleValidator* VYValidator = new QDoubleValidator(parent);
    VYValidator->setRange(-3.0, 3.0, 1);
    QDoubleValidator* TValidator = new QDoubleValidator(parent);
    TValidator->setRange(-180, 180, 1);
    QDoubleValidator* TFValidator = new QDoubleValidator(parent);
    TFValidator->setRange(0.0, 10.0, 2);
    w->checkBoxs.append(new QCheckBox("Follow Mouse"));
    w->checkBoxs.append(new QCheckBox("Follow Ball"));
    QLabel* lblTargetX = new QLabel("X : ");
    QLabel* lblTargetY = new QLabel("Y : ");
    QLabel* lblTargetTeta = new QLabel("Teta : ");
    QLabel* lblTargetVX = new QLabel("VX : ");
    QLabel* lblTargetVY = new QLabel("VY : ");
    QLabel* lblTargetTimeFactor = new QLabel("TimeFactor : ");
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("0.5"));
    w->lineEdits.append(new QLineEdit("0"));
    w->lineEdits.append(new QLineEdit("1.0"));
    w->lineEdits[0]->setValidator(XValidator);
    w->lineEdits[1]->setValidator(YValidator);
    w->lineEdits[2]->setValidator(TValidator);
    w->lineEdits[3]->setValidator(VXValidator);
    w->lineEdits[4]->setValidator(VYValidator);
    w->lineEdits[5]->setValidator(TFValidator);
    QGridLayout *layoutTarget = new QGridLayout;
    layoutTarget->addWidget(lblTargetX, 0, 0);
    layoutTarget->addWidget(w->lineEdits[0], 0, 1);
    layoutTarget->addWidget(lblTargetY, 0, 2);
    layoutTarget->addWidget(w->lineEdits[1], 0, 3);
    layoutTarget->addWidget(lblTargetTeta, 0, 4);
    layoutTarget->addWidget(w->lineEdits[2], 0, 5);
    layoutTarget->addWidget(lblTargetVX, 1, 0);
    layoutTarget->addWidget(w->lineEdits[3], 1, 1);
    layoutTarget->addWidget(lblTargetVY, 1, 2);
    layoutTarget->addWidget(w->lineEdits[4], 1, 3);
    layoutTarget->addWidget(lblTargetTimeFactor, 1, 4);
    layoutTarget->addWidget(w->lineEdits[5], 1, 5);
    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->addLayout(layoutTarget, 4);
    layoutMain->addWidget(w->checkBoxs[0]);
    layoutMain->addWidget(w->checkBoxs[1]);
    w->setLayout(layoutMain);
    return w;
}

void CSkillIntercept::generateFromConfig(CAgent *a)
{
    agent = a;
    setTimeFactor(configWidget->lineEdits[5]->text().toDouble());
    behindTargetAcc = 1.0;
    behindTargetDist = 0.4;
    behindTargetAngle = 40;
    if (configWidget->checkBoxs[1]->isChecked())
    {
        setTarget(wm->ball);
        targetDir = Vector2D::unitVector(configWidget->lineEdits[2]->text().toDouble());
        setKickMode(true);
    }
    else
    {
        setKickMode(false);
        target = new CMovingObject(false);
        if (target != NULL && target!=wm->ball) delete target;
        if (configWidget->checkBoxs[0]->isChecked())
        {
            target->pos = * (configWidget->mousePos);
            targetDir = Vector2D::unitVector(configWidget->lineEdits[2]->text().toDouble());
        }
        else {
            target->pos.x = configWidget->lineEdits[0]->text().toDouble();
            target->pos.y = configWidget->lineEdits[1]->text().toDouble();
            targetDir = Vector2D::unitVector(configWidget->lineEdits[2]->text().toDouble());
        }
        target->vel.x = 0.0;
        target->vel.y = 0.0;
        target->acc.invalidate();
        setTarget(target);
    }
}


CSkillConfigWidget* CSkillFollowPoints::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillFollowPoints(NULL);
    w->skill->configWidget = w;
    w->checkBoxs.append(new QCheckBox("Look at oppGoal"));
    w->checkBoxs.append(new QCheckBox("Add points on each click"));
    w->checkBoxs.append(new QCheckBox("Remove points on each click"));
    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->addWidget(w->checkBoxs[0]);
    layoutMain->addWidget(w->checkBoxs[1]);
    layoutMain->addWidget(w->checkBoxs[2]);
    w->setLayout(layoutMain);
    return w;
}


void CSkillFollowPoints::generateFromConfig(CAgent *a)
{
    agent = a;
    Vector2D lastMousePos;
    lastMousePos = *(configWidget->mousePos);
    if (configWidget->checkBoxs[1]->isChecked())
    {
        addPoint(lastMousePos);
        configWidget->checkBoxs[2]->setChecked(false);
    }
    else if (configWidget->checkBoxs[2]->isChecked()) {if (!points.empty()) points.removeLast();}
    for (int k=0;k<points.count();k++)
    {
        draw(points[k], 1, QColor("yellow"));
    }

    if (configWidget->checkBoxs[0]->isChecked())
    {
        setLookAt(wm->field->oppGoal());
    }
    else setLookAt(Vector2D::INVALIDATED);
}

CSkillConfigWidget* CSkillGotoBall::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillGotoBall(NULL);
    w->skill->configWidget = w;
    w->checkBoxs.append(new QCheckBox("Look at mouse"));
    w->checkBoxs.append(new QCheckBox("Look at oppGoal"));
    w->checkBoxs.append(new QCheckBox("Look at ourGoal"));
    w->checkBoxs.append(new QCheckBox("Rolling ball only"));
    w->checkBoxs.append(new QCheckBox("Spin"));
    w->checkBoxs.append(new QCheckBox("Slow"));
    w->checkBoxs.append(new QCheckBox("Turn"));
    w->checkBoxs.append(new QCheckBox("Block"));
    w->comboBoxs.append(new QComboBox(w));
    w->comboBoxs[0]->addItem("Track Curve");
    w->comboBoxs[0]->addItem("Follow Points");
    w->comboBoxs[0]->addItem("RRT");
    w->comboBoxs[0]->addItem("Dynamic Points");
    w->comboBoxs[0]->addItem("Controller");
    QGridLayout *layoutMain = new QGridLayout;
    layoutMain->addWidget(w->checkBoxs[0],0,0);
    layoutMain->addWidget(w->checkBoxs[1],1,0);
    layoutMain->addWidget(w->checkBoxs[2],2,0);
    layoutMain->addWidget(w->checkBoxs[3],0,1);
    layoutMain->addWidget(w->checkBoxs[4],1,1);
    layoutMain->addWidget(w->checkBoxs[5],2,1);
    layoutMain->addWidget(w->checkBoxs[6],3,0);
    layoutMain->addWidget(w->checkBoxs[7],3,1);
    layoutMain->addWidget(w->comboBoxs[0],4,0,1,2);
    w->checkBoxs[1]->setChecked(true);
    w->setLayout(layoutMain);
    return w;
}

void CSkillGotoBall::generateFromConfig(CAgent *a)
{
    agent = a;
    if (configWidget->checkBoxs[0]->isChecked())
    {
        if (configWidget->checkBoxs[1]->isChecked()) configWidget->checkBoxs[1]->setChecked(false);
        if (configWidget->checkBoxs[2]->isChecked()) configWidget->checkBoxs[2]->setChecked(false);
        setGoal(* (configWidget->mousePos));
    }
    else if (configWidget->checkBoxs[1]->isChecked())
    {
        if (configWidget->checkBoxs[0]->isChecked()) configWidget->checkBoxs[0]->setChecked(false);
        if (configWidget->checkBoxs[2]->isChecked()) configWidget->checkBoxs[2]->setChecked(false);
        setGoal(wm->field->oppGoal());
    }
    else if (configWidget->checkBoxs[2]->isChecked())
    {
        if (configWidget->checkBoxs[1]->isChecked()) configWidget->checkBoxs[1]->setChecked(false);
        if (configWidget->checkBoxs[0]->isChecked()) configWidget->checkBoxs[0]->setChecked(false);
        setGoal(wm->field->ourGoal());
    }
    setSpin(configWidget->checkBoxs[4]->isChecked());
    setSlow(configWidget->checkBoxs[5]->isChecked());
    setTurn(configWidget->checkBoxs[6]->isChecked());
    setBlock(configWidget->checkBoxs[7]->isChecked());
    setRollingBallOnly(configWidget->checkBoxs[3]->isChecked());
    if (configWidget->comboBoxs[0]->currentIndex()==0)
    {
        setUseCurve(true);
        setUsePoints(false);
        setUseDynamicPoints(false);
        setUseControler(false);
    }
    if (configWidget->comboBoxs[0]->currentIndex()==1)
    {
        setUseCurve(false);
        setUsePoints(true);
        setUseDynamicPoints(false);
        setUseControler(false);
    }
    if (configWidget->comboBoxs[0]->currentIndex()==2)
    {
        setUseCurve(false);
        setUsePoints(false);
        setUseDynamicPoints(false);
        setUseControler(false);
    }
    if (configWidget->comboBoxs[0]->currentIndex()==3)
    {
        setUseCurve(false);
        setUsePoints(false);
        setUseDynamicPoints(true);
        setUseControler(false);
    }
    if (configWidget->comboBoxs[0]->currentIndex()==4)
    {
        setUseCurve(false);
        setUsePoints(false);
        setUseDynamicPoints(false);
        setUseControler(true);
    }
}

CSkillConfigWidget* CSkillHitTheBall::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillHitTheBall(NULL);
    w->skill->configWidget = w;
    w->checkBoxs.append(new QCheckBox("HitTheBall to mouse"));
    w->checkBoxs.append(new QCheckBox("HitTheBall to oppGoal"));
    w->checkBoxs.append(new QCheckBox("HitTheBall to ourGoal"));
    w->checkBoxs.append(new QCheckBox("Through"));
    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->addWidget(w->checkBoxs[0]);
    layoutMain->addWidget(w->checkBoxs[1]);
    layoutMain->addWidget(w->checkBoxs[2]);
    layoutMain->addWidget(w->checkBoxs[3]);
    w->checkBoxs[1]->setChecked(true);
    w->setLayout(layoutMain);
    return w;
}

void CSkillHitTheBall::generateFromConfig(CAgent *a)
{
    agent = a;
    if (configWidget->checkBoxs[0]->isChecked())
    {
        if (configWidget->checkBoxs[1]->isChecked()) configWidget->checkBoxs[1]->setChecked(false);
        if (configWidget->checkBoxs[2]->isChecked()) configWidget->checkBoxs[2]->setChecked(false);
        setTarget(* (configWidget->mousePos));
    }
    else if (configWidget->checkBoxs[1]->isChecked())
    {
        if (configWidget->checkBoxs[0]->isChecked()) configWidget->checkBoxs[0]->setChecked(false);
        if (configWidget->checkBoxs[2]->isChecked()) configWidget->checkBoxs[2]->setChecked(false);
        setTarget(wm->field->oppGoal());
    }
    else if (configWidget->checkBoxs[2]->isChecked())
    {
        if (configWidget->checkBoxs[1]->isChecked()) configWidget->checkBoxs[1]->setChecked(false);
        if (configWidget->checkBoxs[0]->isChecked()) configWidget->checkBoxs[0]->setChecked(false);
        setTarget(wm->field->ourGoal());
    }
    setThroughMode(configWidget->checkBoxs[3]->isChecked());
}

CSkillConfigWidget* CSkillKick::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillKick(NULL);
    w->skill->configWidget = w;
    w->checkBoxs.append(new QCheckBox("Kick to mouse"));
    w->checkBoxs.append(new QCheckBox("Kick to oppGoal"));
    w->checkBoxs.append(new QCheckBox("Kick to ourGoal"));
    w->checkBoxs.append(new QCheckBox("Spin"));w->checkBoxs.back()->setChecked(true);
    w->checkBoxs.append(new QCheckBox("Intercept"));
    w->checkBoxs.append(new QCheckBox("Chip kick"));
    w->checkBoxs.append(new QCheckBox("Parallel"));
    w->checkBoxs.append(new QCheckBox("Slow"));
    w->checkBoxs.append(new QCheckBox("Sag"));
    w->lineEdits.append(new QLineEdit("500"));
    w->lineEdits.append(new QLineEdit("-1"));
    QGridLayout *layoutMain = new QGridLayout();
    QIntValidator* intValidator = new QIntValidator(1,512,parent);
    w->lineEdits[0]->setValidator(intValidator);
    layoutMain->addWidget(w->lineEdits[0],0,0,1,2);
    layoutMain->addWidget(w->checkBoxs[0],1,0);
    layoutMain->addWidget(w->checkBoxs[1],2,0);
    layoutMain->addWidget(w->checkBoxs[2],3,0);
    layoutMain->addWidget(w->checkBoxs[7],3,1);
    layoutMain->addWidget(w->checkBoxs[3],4,0);
    layoutMain->addWidget(w->checkBoxs[5],4,1);
    layoutMain->addWidget(w->checkBoxs[4],5,0);
    layoutMain->addWidget(w->checkBoxs[6],5,1);
    layoutMain->addWidget(w->checkBoxs[8],6,0);
    layoutMain->addWidget(w->lineEdits[1],7,0,1,2);
    w->checkBoxs[1]->setChecked(true);
    w->checkBoxs[4]->setChecked(true);
    w->checkBoxs[6]->setChecked(true);
    w->setLayout(layoutMain);
    return w;
}

void CSkillKick::generateFromConfig(CAgent *a)
{
    agent = a;
    if (configWidget->checkBoxs[0]->isChecked())
    {
        if (configWidget->checkBoxs[1]->isChecked()) configWidget->checkBoxs[1]->setChecked(false);
        if (configWidget->checkBoxs[2]->isChecked()) configWidget->checkBoxs[2]->setChecked(false);
        setTarget(* (configWidget->mousePos));
        //setKickSpeed(agent->kickValueForDistance(wm->ball->pos.dist(*configWidget->mousePos), 0.0));
        // if(configWidget->checkBoxs[5]->isChecked())
        //  setKickSpeed(agent->chipDistanceValue((wm->ball->pos.dist(* (configWidget->mousePos))),configWidget->checkBoxs[3]->isChecked()));
        qDebug() << agent->kickValueForDistance(wm->ball->pos.dist(*configWidget->mousePos), 0.0);
        //        draw(QString("Kick : %1").arg(agent->kickValueForDistance(wm->ball->pos.dist(*configWidget->mousePos), 0.0)),Vector2D(-1,1));
    }
    else if (configWidget->checkBoxs[1]->isChecked())
    {
        if (configWidget->checkBoxs[0]->isChecked()) configWidget->checkBoxs[0]->setChecked(false);
        if (configWidget->checkBoxs[2]->isChecked()) configWidget->checkBoxs[2]->setChecked(false);
        setTarget(wm->field->oppGoal());//
        //        setTarget(wm->field->oppGoalL());// TIRAK
    }
    else if (configWidget->checkBoxs[2]->isChecked())
    {
        if (configWidget->checkBoxs[1]->isChecked()) configWidget->checkBoxs[1]->setChecked(false);
        if (configWidget->checkBoxs[0]->isChecked()) configWidget->checkBoxs[0]->setChecked(false);
        setTarget(wm->field->ourGoal());
    }
    setSagMode(configWidget->checkBoxs[8]->isChecked());
    setSlow(configWidget->checkBoxs[7]->isChecked());
    setSpin(configWidget->checkBoxs[3]->isChecked());
    setThroughMode(false);
    setWaitFrames(6);
    setInterceptMode(configWidget->checkBoxs[4]->isChecked());
    setParallelMode(configWidget->checkBoxs[6]->isChecked());
    setChip(configWidget->checkBoxs[5]->isChecked());
    setKickSpeed(configWidget->lineEdits[0]->text().toInt());
    setTolerance(3);
    if (configWidget->checkBoxs[0]->isChecked())
        setAutoChipSpeed(false);
    setAvoidPenaltyArea(true);
}

CSkillConfigWidget* CSkillKickOneTouch::generateConfigWidget(QWidget *parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillKickOneTouch(NULL);
    w->skill->configWidget = w;
    w->checkBoxs.append(new QCheckBox("Kick to oppGoal"));
    w->checkBoxs.append(new QCheckBox("Kick to ourGoal"));
    w->lineEdits.append(new QLineEdit("500"));
    w->lineEdits.append(new QLineEdit("0.1"));
    w->lineEdits.append(new QLineEdit("0.3"));
    QGridLayout *layoutMain = new QGridLayout;
    QIntValidator* intValidator = new QIntValidator(0,1023,parent);
    QDoubleValidator* dblValidator1 = new QDoubleValidator(0,0.5,3,parent);
    QDoubleValidator* dblValidator2 = new QDoubleValidator(0,0.5,3,parent);
    w->lineEdits[0]->setValidator(intValidator);
    w->lineEdits[1]->setValidator(dblValidator1);
    w->lineEdits[2]->setValidator(dblValidator2);
    layoutMain->addWidget(w->lineEdits[0],0,1);layoutMain->addWidget(new QLabel("Kick Speed"),0,0);
    layoutMain->addWidget(w->lineEdits[1],1,1);layoutMain->addWidget(new QLabel("Distance to ball line"),1,0);
    layoutMain->addWidget(w->lineEdits[2],2,1);layoutMain->addWidget(new QLabel("Velocity on ball line"),2,0);
    layoutMain->addWidget(w->checkBoxs[0],3,1);
    layoutMain->addWidget(w->checkBoxs[1],4,1);
    w->checkBoxs[0]->setChecked(true);
    w->setLayout(layoutMain);
    return w;
}

void CSkillKickOneTouch::generateFromConfig(CAgent *a)
{
    agent = a;
    if (configWidget->checkBoxs[0]->isChecked())
    {
        if (configWidget->checkBoxs[1]->isChecked()) configWidget->checkBoxs[1]->setChecked(false);
        setTarget(wm->field->oppGoal());
    }
    else if (configWidget->checkBoxs[1]->isChecked())
    {
        if (configWidget->checkBoxs[0]->isChecked()) configWidget->checkBoxs[0]->setChecked(false);
        setTarget(wm->field->ourGoal());
    }
    setKickSpeed(configWidget->lineEdits[0]->text().toInt());
    setWaitPos(*(configWidget->mousePos));
    setDistToBallLine(configWidget->lineEdits[1]->text().toDouble());
    setVelocityToBallLine(configWidget->lineEdits[2]->text().toDouble());
}

CSkillConfigWidget* CSkillSpinBack::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CSkillSpinBack(NULL);
    w->skill->configWidget = w;
    w->lineEdits.append(new QLineEdit("50"));
    w->lineEdits.append(new QLineEdit("0.3"));
    w->lineEdits.append(new QLineEdit("90"));
    w->lineEdits.append(new QLineEdit("0.5"));
    w->checkBoxs.append(new QCheckBox("Take Back"));
    w->checkBoxs.append(new QCheckBox("Correct Angle"));
    QIntValidator* intValidator = new QIntValidator(1,200,parent);
    QDoubleValidator* dblValidator1 = new QDoubleValidator(0,2,3,parent);
    QDoubleValidator* dblValidator2 = new QDoubleValidator(0,720,3,parent);
    QDoubleValidator* dblValidator3 = new QDoubleValidator(0,2,3,parent);
    w->lineEdits[0]->setValidator(intValidator);
    w->lineEdits[1]->setValidator(dblValidator1);
    w->lineEdits[2]->setValidator(dblValidator2);
    w->lineEdits[3]->setValidator(dblValidator3);
    QGridLayout *layoutMain = new QGridLayout;
    layoutMain->addWidget(w->lineEdits[0],0,1);layoutMain->addWidget(new QLabel("Wait frames"),0,0);
    layoutMain->addWidget(w->lineEdits[1],1,1);layoutMain->addWidget(new QLabel("Linear velocity"),1,0);
    layoutMain->addWidget(w->lineEdits[2],2,1);layoutMain->addWidget(new QLabel("Angular velocity"),2,0);
    layoutMain->addWidget(w->lineEdits[3],3,1);layoutMain->addWidget(new QLabel("Linear acceleration"),3,0);
    layoutMain->addWidget(w->checkBoxs[0],4,0);
    layoutMain->addWidget(w->checkBoxs[1],5,1);
    w->setLayout(layoutMain);
    return w;
}

void CSkillSpinBack::generateFromConfig(CAgent *a)
{
    agent = a;
    setTarget(* (configWidget->mousePos));
    setWaitFrames(configWidget->lineEdits[0]->text().toInt());
    setLinearVelocity(configWidget->lineEdits[1]->text().toDouble());
    setAnglularVelocity(configWidget->lineEdits[2]->text().toDouble());
    setLinearAcceleration(configWidget->lineEdits[3]->text().toDouble());
    setTakeBack(configWidget->checkBoxs[0]->isChecked());
    setCorrectAngleTowardTarget(configWidget->checkBoxs[1]->isChecked());
    setInitialTarget(Vector2D(0,0));
}

CSkillConfigWidget* CSkillTrackCurve::generateConfigWidget(QWidget* parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
//    w->skill = new CSkillTrackCurve(NULL);
//    w->skill->configWidget = w;
//    //QDoubleValidator* v = new QDoubleValidator(parent);
//    w->checkBoxs.append(new QCheckBox("Look at oppGoal"));
//    w->checkBoxs.append(new QCheckBox("Spline"));
//    w->checkBoxs.append(new QCheckBox("Add points on each click"));
//    w->checkBoxs.append(new QCheckBox("Remove points on each click"));
//    w->checkBoxs.append(new QCheckBox("Heart"));
//    w->checkBoxs.append(new QCheckBox("Spiral"));
//    w->lineEdits.append(new QLineEdit("1",parent));
//    QVBoxLayout *layoutMain = new QVBoxLayout;
//    layoutMain->addWidget(w->checkBoxs[0]);
//    layoutMain->addWidget(w->checkBoxs[1]);
//    layoutMain->addWidget(w->checkBoxs[2]);
//    layoutMain->addWidget(w->checkBoxs[3]);
//    layoutMain->addWidget(w->checkBoxs[4]);
//    layoutMain->addWidget(w->checkBoxs[5]);
//    layoutMain->addWidget(w->lineEdits[0]);
//    w->setLayout(layoutMain);
    return w;
}

void CSkillTrackCurve::generateFromConfig(CAgent *a)
{
//    agent = a;
//    r_max = configWidget->lineEdits[0]->text().toDouble();
//    if (configWidget->checkBoxs[1]->isChecked())
//    {
//        configWidget->checkBoxs[4]->setChecked(false);
//        configWidget->checkBoxs[5]->setChecked(false);
//        /*if (curve==NULL) */curve = new CCurveSpline();
//        if (lastMousePos != (*(configWidget->mousePos)))
//        {
//            lastMousePos = *(configWidget->mousePos);
//            if (configWidget->checkBoxs[2]->isChecked())
//            {
//                ((CCurveSpline*) curve)->knots.append(lastMousePos);
//                configWidget->checkBoxs[3]->setChecked(false);
//            }
//            else if (configWidget->checkBoxs[3]->isChecked()) {if (!((CCurveSpline*) curve)->knots.empty()) ((CCurveSpline*) curve)->knots.removeLast();}
//            for (int k=0;k<((CCurveSpline*) curve)->knots.count();k++)
//            {
//                draw(((CCurveSpline*) curve)->knots[k], 1, QColor("yellow"));
//            }
//        }
//        if (((CCurveSpline*) curve)->knots.count() > 3)
//            ((CCurveSpline*) curve)->init();
//        ((CCurveSpline*) curve)->last_t = 0.0;
//    }
//    if (configWidget->checkBoxs[4]->isChecked())
//    {
//        configWidget->checkBoxs[5]->setChecked(false);
//        curve = new CCurveHeart(.5, 1.0, 2.0, 1/(2.0*M_PI));
//        ((CCurveHeart*) curve)->last_t = 0.0;
//        ((CCurveHeart*) curve)->setTFinal(40);
//    }
//    if (configWidget->checkBoxs[5]->isChecked())
//    {
//        curve = new CCurveMarpich(0.05, 0.2, 50.0);
//        ((CCurveHeart*) curve)->last_t = 0.0;
//    }
//    if (configWidget->checkBoxs[0]->isChecked())
//    {
//        setLookAt(wm->field->oppGoal());
//    }
//    else setLookAt(Vector2D::INVALIDATED);
//    trackstarted = false;
}
