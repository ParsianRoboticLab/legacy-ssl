#include "simulator.h"

CSimulator* simulator;

CSimulator::CSimulator()
{
    wm0 = NULL;
    wm = new CWorldModel(true);
    gotopoint = NULL;
    angTol = 10.0;
    posTol = 0.05;
}

void CSimulator::load()
{
    wm->ball->acc = ::wm->ball->acc;
    wm->ball->vel = ::wm->ball->vel;
    wm->ball->pos = ::wm->ball->pos;
    wm->ball->inSight = ::wm->ball->inSight;
    for (int i = 0;i<_NUM_PLAYERS;i++)
    {
        wm->our[i]->inSight = ::wm->our[i]->inSight;
        wm->our[i]->acc = ::wm->our[i]->acc;
        wm->our[i]->vel = ::wm->our[i]->vel;
        wm->our[i]->pos = ::wm->our[i]->pos;
        wm->our[i]->dir = ::wm->our[i]->dir;
        wm->opp[i]->inSight = ::wm->opp[i]->inSight;
        wm->opp[i]->acc = ::wm->opp[i]->acc;
        wm->opp[i]->vel = ::wm->opp[i]->vel;
        wm->opp[i]->pos = ::wm->opp[i]->pos;
        wm->opp[i]->dir = ::wm->opp[i]->dir;
    }
}

void CSimulator::startSimulation()
{
    wm0 = ::wm;
    ::wm = wm;
}

void CSimulator::doneSimulation()
{
    if (wm0 != NULL) ::wm = wm0;
}

void CSimulator::velocityTransferFunction(Vector2D v_desired, double w_desired, Vector2D v_current, double w_current, double dt, Vector2D& v_actual, double &w_actual)
{
    v_actual = v_desired;
    w_actual = w_desired;
}

void CSimulator::step(double dt)
{
    for (int i = 0;i<_NUM_PLAYERS;i++)
    {
        if (wm->our[i]->inSight > 0)
        {
            double vx =  knowledge->getAgent(i)->vforward;
            double vy =  knowledge->getAgent(i)->vnormal;
            double w =  knowledge->getAgent(i)->vangular;
            double ang = knowledge->getAgent(i)->dir().th().radian();
            double vxx = (cos(ang) * vx) - (sin(ang) * vy);
            double vyy = (sin(ang) * vx) + (cos(ang) * vy);
            velocityTransferFunction(Vector2D(vxx,vyy), w, wm->our[i]->vel, wm->our[i]->angularVel, dt, wm->our[i]->vel, wm->our[i]->angularVel);
            wm->our[i]->pos += wm->our[i]->vel * dt;
            wm->our[i]->dir.rotate(wm->our[i]->angularVel*dt);
        }
        if (wm->opp[i]->inSight > 0)
        {
			wm->opp[i]->pos += wm->opp[i]->vel * dt;
        }
    }
    wm->ball->pos += wm->ball->vel * dt;
    if (wm->ball->vel.length() > 0.001)
    {
        Vector2D vball = wm->ball->vel;
        wm->ball->vel -= BallFriction() * Gravity * wm->ball->vel.norm();
        if (wm->ball->vel.innerProduct(vball) < 0)
            wm->ball->vel.assign(0.0, 0.0);
    }
}

double CSimulator::timeNeededForGotoPoint(Vector2D target, Vector2D targetDir, double posTolerance, double angTolerance, int agent, double dt, int maxIters)
//above parameters are in [seconds, meters, unit vector, meters, angles, id] respectively.
{
	return 0;
    load();
    startSimulation();

	gotopoint = new CSkillGotoPoint(knowledge->getAgent(agent));
    double time = 0;
    int i = 0;
    posTol = posTolerance;
    angTol = angTolerance;
	gotopoint->init(target, targetDir);
    while (1)
    {
        gotopoint->execute();
        step(dt);
		draw(knowledge->getAgent(agent)->pos(), 1, "red");
        if (isReached(target, targetDir, agent))
        {
            break;
        }
        if (i > maxIters)
        {
            break;
        }
        i++;
        time += dt;
    }
	delete gotopoint;

    doneSimulation();
    return time;
}

bool CSimulator::isReached(Vector2D target, Vector2D targetDir, int agent) //uses tolerances set in timeNeededForGotoPoint
{
    return ( ((knowledge->getAgent(agent)->pos() - target).length() < posTol)
                 &&
                 (fabs(Vector2D::angleBetween(knowledge->getAgent(agent)->dir(), targetDir).degree()) < angTol) );
}
