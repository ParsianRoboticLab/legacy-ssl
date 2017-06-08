#include "chipkick.h"
#include <QDebug>
#include <knowledge.h>
#include "skills.h"
#include "playonrole.h"

chipKick *chipKickRobot;
chipKick::chipKick()
{
    qDebug()<<"oomaaaad";
    robotId=2;
//    refrenceRobotId=0;
    prevVel=0;
    states=initChipKick;


    rect_width = _FIELD_WIDTH/8.0;

    foundPos = false;
    foundVel =false;

    vel=0;

    up = new Polygon2D();
    up->addVertex(Vector2D(-_FIELD_WIDTH/2 , _FIELD_HEIGHT/2));
    up->addVertex(Vector2D(-_FIELD_WIDTH/2 , _FIELD_HEIGHT/2-rect_width));
    up->addVertex(Vector2D(_FIELD_WIDTH/2 , _FIELD_HEIGHT/2-rect_width));
    up->addVertex(Vector2D(_FIELD_WIDTH/2 , _FIELD_HEIGHT/2));

    down = new Polygon2D();
    down->addVertex(Vector2D(-_FIELD_WIDTH/2 , -_FIELD_HEIGHT/2));
    down->addVertex(Vector2D(-_FIELD_WIDTH/2 , -_FIELD_HEIGHT/2+rect_width));
    down->addVertex(Vector2D(_FIELD_WIDTH/2 , -_FIELD_HEIGHT/2+rect_width));
    down->addVertex(Vector2D(_FIELD_WIDTH/2 , -_FIELD_HEIGHT/2));

    left = new Polygon2D();
    left->addVertex(Vector2D(-_FIELD_WIDTH/2 , _FIELD_HEIGHT/2));
    left->addVertex(Vector2D(-_FIELD_WIDTH/2+rect_width , _FIELD_HEIGHT/2));
    left->addVertex(Vector2D(-_FIELD_WIDTH/2+rect_width , -_FIELD_HEIGHT/2));
    left->addVertex(Vector2D(-_FIELD_WIDTH/2 , -_FIELD_HEIGHT/2));

    right = new Polygon2D();
    right->addVertex(Vector2D(_FIELD_WIDTH/2 , _FIELD_HEIGHT/2));
    right->addVertex(Vector2D(_FIELD_WIDTH/2-rect_width , _FIELD_HEIGHT/2));
    right->addVertex(Vector2D(_FIELD_WIDTH/2-rect_width , -_FIELD_HEIGHT/2));
    right->addVertex(Vector2D(_FIELD_WIDTH/2 , -_FIELD_HEIGHT/2));

}

void chipKick::stateController(){
    switch(states){
    case initChipKick:

        receiver=new CSkillGotoPointAvoid(knowledge->getAgent(robotId));
        refrenceRobot=new CRobot(wm->our.active(0)->id,false);

        pos1=wm->our.active(0)->dir;
        pos2=wm->our.active(0)->pos;
        receiver->init(pos2+pos1.norm()*1,-pos1);
        states=beforeChip;
    case beforeChip:
        //        qDebug()<<QString("pos1:x=%1 y= %2 ,x= %3 y=%4").arg(pos1.x).arg(pos1.y).arg(pos2.x).arg(pos2.y);
        if(Circle2D(wm->our.active(0)->pos,1).contains(wm->ball->pos))
            states=Placement;
        if((up->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.y>0)
                || (down->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.y<0 )
                || (left->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.x<0)
                || (down->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.x>0 )
                ){
            pos1=-wm->our.active(0)->dir;
        }
        else
            pos1=wm->our.active(0)->dir;
        pos2=wm->our.active(0)->pos;
        primaryPlacement(1);
        debug("beforeChip",D_NADIA);
        break;
    case Placement:
        if(Circle2D(wm->our.active(0)->pos,0.2).contains(wm->ball->pos))
            states=predictState;
        if((up->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.y>0)
                || (down->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.y<0 )
                || (left->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.x<0)
                || (down->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.x>0 )
                ){
            pos1=-wm->our.active(0)->dir;
        }
        else
            pos1=wm->our.active(0)->dir;
        pos2=wm->our.active(0)->pos;
        primaryPlacement(2.5);
        debug("Placement",D_NADIA);
        break;
    case predictState:
        r=predictChipPos();

        if(wm->ball->vel.length()<0.1){
            if((up->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.y>0)
                    || (down->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.y<0 )
                    || (left->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.x<0)
                    || (down->contains(wm->our.active(0)->pos) && wm->our.active(0)->dir.x>0 )
                    ){
                pos1=-wm->our.active(0)->dir;
            }
            else
                pos1=wm->our.active(0)->dir;
            pos2=wm->our.active(0)->pos;
        }

        r=predictChipPos();

//        prevVel = wm->ball->vel.length();

        debug(QString("ballVel:%1").arg(r),D_NADIA);
        primaryPlacement(r);
        debug(QString("PredictChip:%1").arg(r),D_NADIA);
        if(Circle2D(ss,0.1).contains(wm->ball->pos))
            states=KickSkillSet;
        break;
    case KickSkillSet:
        //setkickSkill;
            static CSkillKickOneTouch *onChip = new CSkillKickOneTouch(knowledge->getAgent(robotId));

            onChip->setAgent(knowledge->getAgent(robotId));
            onChip->setWaitPos(Vector2D(-2 , 2));
            onChip->setTarget(wm->field->oppGoal());
            onChip->setKickSpeed(1023/123);
            onChip->setChip(false);
            onChip->setAvoidPenaltyArea(true);


            onChip->execute();
            break;
    }
    if(states!=initChipKick)
        receiver->execute();


}

void chipKick::primaryPlacement(double radius){

    ss = pos2+pos1.norm()*radius;
    receiver->init(ss, wm->field->oppGoal());

}
double chipKick::predictChipPos(){

    if(vel < wm->ball->vel.length())
        vel = wm->ball->vel.length();

    if(ballposss.size()==0)
        ballposss.append(wm->ball->pos);

    if(fabs(wm->ball->pos.x - ballposss.last().x) > 0.003 && fabs(wm->ball->pos.y - ballposss.last().y) > 0.003)
        ballposss.append(wm->ball->pos);

    if(ballposss.size() > 10)
        for(int i=0; i<ballposss.size() - 5; i+=2){
            tan = v12.th().tan();
            v12 = ballposss.at(i) - ballposss.at(i+5);
            if(tan*v12.th().tan() < 0 ){
                save =ballposss.at(i);
                foundPos = true;
                break;
            }
        }


    ballVel1= (wm->ball->vel-pos2).norm();
    velBallX1=ballVel1.angleWith(pos1).cos()*wm->ball->vel.length();

    if(ballVel.size()>6){
        for(int i=0;i<6;i++){
            if(ballVel.at(i)<velBallX1){
                foundVel=true;
            }
        }
        ballVel.pop_front();
    }
    ballVel.append(velBallX1);



    if(foundPos){
        Vector2D vatar=(save-pos2);
        return 2*vatar.angleWith(pos1).cos()*vatar.length();
    }
    else if(foundVel)
    {
        Vector2D vatar=(save-pos2);
        return 2*vatar.angleWith(pos1).cos()*vatar.length();
    }
    else{
        if(vel < 2)
            return 1.5;
        else if(vel < 3.2)
            return 2.5;
        else
            return 3;
    }




//            Vector2D vatar=(wm->ball->pos-pos2);
//            if(vatar.angleWith(pos1).cos()*vatar.length()>0.5)
//                return vatar.angleWith(pos1).cos()*vatar.length()+0.05;
//            else
//                return 1.6;

}
