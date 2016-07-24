#include <roles.h>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QValidator>
#include <QLineEdit>
#include <QGridLayout>

CSkillConfigWidget* CRolePosition::generateConfigWidget(QWidget *parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CRolePosition(NULL);
    w->skill->configWidget = w;    
    w->lineEdits.append(new QLineEdit("oppfield"));
    QGridLayout *layoutMain = new QGridLayout;
    layoutMain->addWidget(w->lineEdits[0],0,1);layoutMain->addWidget(new QLabel("Params"),0,0);
    w->setLayout(layoutMain);
    return w;
}

void CRolePosition::generateFromConfig(CAgent *a)
{
    agent = a;
    parse(configWidget->lineEdits[0]->text().split(','));
}


CSkillConfigWidget* CRoleBlock::generateConfigWidget(QWidget */*parent*/)
{
    return NULL;
}

void CRoleBlock::generateFromConfig(CAgent *a)
{
    agent = a;
}

CSkillConfigWidget* CRoleGaurd::generateConfigWidget(QWidget */*parent*/)
{
    return NULL;
}

void CRoleGaurd::generateFromConfig(CAgent *a)
{
    agent = a;
}


#include <opponent.h>

CSkillConfigWidget* CRoleMark::generateConfigWidget(QWidget *parent)
{
    CSkillConfigWidget* w = new CSkillConfigWidget(parent);
    w->skill = new CRoleMark(NULL);
    w->skill->configWidget = w;
    w->lineEdits.append(new QLineEdit("-1"));
    w->lineEdits.append(new QLineEdit("0.7"));
    w->lineEdits.append(new QLineEdit("0.16"));
    w->comboBoxs.append(new QComboBox());
    QGridLayout *layoutMain = new QGridLayout;
    QIntValidator* intValidator = new QIntValidator(-1,12,parent);
    QDoubleValidator* dblValidator1 = new QDoubleValidator(-1.0,1.0,3,parent);
    QDoubleValidator* dblValidator2 = new QDoubleValidator(-1.0,2.0,3,parent);
    w->lineEdits[0]->setValidator(intValidator);
    w->lineEdits[1]->setValidator(dblValidator1);
    w->lineEdits[2]->setValidator(dblValidator2);
    for (int i=0;i<opponent->roles.count();i++)
        w->comboBoxs[0]->addItem(opponent->roles[i]->name());
    layoutMain->addWidget(w->lineEdits[0],0,1);layoutMain->addWidget(new QLabel("Opponent Id"),0,0);
    layoutMain->addWidget(w->lineEdits[1],1,1);layoutMain->addWidget(new QLabel("Gamma"),1,0);
    layoutMain->addWidget(w->lineEdits[2],2,1);layoutMain->addWidget(new QLabel("Distance"),2,0);
    layoutMain->addWidget(w->comboBoxs[0],3,1);layoutMain->addWidget(new QLabel("Opponent"),3,0);
    w->setLayout(layoutMain);
    return w;
}

void CRoleMark::generateFromConfig(CAgent *a)
{
    agent = a;
//    setMarkingAgent(configWidget->lineEdits[0]->text().toInt());
    toBeMarkedRole.clear();
    toBeMarkedRole.append(configWidget->comboBoxs[0]->currentText());
}

CSkillConfigWidget* CRoleWarmup::generateConfigWidget(QWidget */*parent*/)
{
    return NULL;
}

void CRoleWarmup::generateFromConfig(CAgent *a)
{
    agent = a;
}

CSkillConfigWidget* CRoleSupport::generateConfigWidget(QWidget */*parent*/)
{
    return NULL;
}

void CRoleSupport::generateFromConfig(CAgent *a)
{
    agent = a;
}
