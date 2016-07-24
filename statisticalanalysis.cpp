#include "statisticalanalysis.h"
#include <QDebug>

CStatisticalAnalysis::CStatisticalAnalysis()
{
    numOfVars = 0;
}

void CStatisticalAnalysis::updateMeanAndVariance(statStat &stat, Vector2D newVal)
{
    stat.count++;
    if (stat.count == 1)
    {
        stat.mean = newVal;
        stat.m2 = Vector2D(0.0, 0.0);
        stat.variance = Vector2D(0.0, 0.0);
    }
    else
    {
        Vector2D delta = newVal - stat.mean;
        stat.mean = stat.mean + (delta / stat.count);
        Vector2D temp = newVal - stat.mean;
        stat.m2 = stat.m2 + Vector2D((delta.x * temp.x) , (delta.y * temp.y));
        stat.variance = stat.m2 / (stat.count - 1);
    }
}

void CStatisticalAnalysis::addStatVariable(QString name)
{
    statVar var;

    var.name = name;
    var.stat.count = 0.0;
    var.stat.mean = Vector2D(0.0, 0.0);
    var.stat.variance = Vector2D(0.0, 0.0);
    var.stat.m2 = Vector2D(0.0, 0.0);
    var.numOfRegions = 0;

    this->vars.append(var);
    numOfVars++;
}

void CStatisticalAnalysis::addRegionToVariable(QString name, QString regName, Rect2D rect)
{

    statRegion reg;
    reg.name = regName;
    reg.rect = rect;
    reg.stat.count = 0.0;
    reg.stat.mean = Vector2D(0.0, 0.0);
    reg.stat.variance = Vector2D(0.0, 0.0);
    reg.stat.m2 = Vector2D(0.0, 0.0);


    for (int i = 0; i < numOfVars; i++)
    {
        if (vars.at(i).name == name)
        {
            vars[i].regions.append(reg);
            vars[i].numOfRegions++;
            return;
        }
    }
    qWarning() << "addRegionToVariable | Variable not found! :" << name;
}

void CStatisticalAnalysis::updateVar(QString name, Vector2D pos)
{
    for (int i = 0; i < numOfVars; i++)
    {
        if (vars.at(i).name == name)
        {
            updateMeanAndVariance(vars[i].stat, pos);
            for (int j = 0; j < vars.at(i).numOfRegions; j++)
            {
                if (vars[i].regions[j].rect.contains(pos))
                {
                    updateMeanAndVariance(vars[i].regions[j].stat, pos);
                }
            }
            return;
        }
    }
    qWarning() << "updateVar | Variable not found! :" << name;
}

statVar CStatisticalAnalysis::getVar(QString name)
{
    for (int i = 0; i < numOfVars; i++)
    {
        if (vars.at(i).name == name)
        {
            return vars.at(i);
        }
    }

    statVar var;

    var.name = name;
    var.stat.count = 0.0;
    var.stat.mean = Vector2D(0.0, 0.0);
    var.stat.variance = Vector2D(0.0, 0.0);
    var.stat.m2 = Vector2D(0.0, 0.0);
    var.numOfRegions = 0;

    return var;
}

QString CStatisticalAnalysis::statToString(statStat var)
{
    return QString("Count:%1,Mean:[%2,%3],Variance: Car[%4,%5] Pol[%6,%7]")
            .arg(var.count)
            .arg(var.mean.x)
            .arg(var.mean.y)
            .arg(var.variance.x)
            .arg(var.variance.y)
            .arg(var.variance.length())
            .arg(var.variance.dir().degree());
}

void CStatisticalAnalysis::resetVar( QString name)
{
    for (int i = 0; i < numOfVars; i++)
    {
        if (vars.at(i).name == name)
        {
            vars[i].stat.count = 0.0;
            vars[i].stat.mean = Vector2D(0.0, 0.0);
            vars[i].stat.m2 = Vector2D(0.0, 0.0);
            vars[i].stat.variance = Vector2D(0.0, 0.0);
            for (int j = 0; j < vars[i].numOfRegions; j++)
            {
                vars[i].regions[j].stat.count = 0.0;
                vars[i].regions[j].stat.mean = Vector2D(0.0, 0.0);
                vars[i].regions[j].stat.m2 = Vector2D(0.0, 0.0);
                vars[i].regions[j].stat.variance = Vector2D(0.0, 0.0);

            }
            return;
        }
    }
    qWarning() << "resetVar | Variable not found! :" << name;
}

void CStatisticalAnalysis::randomizeVar(QString name)
{
    double rndCount = floor(drand48() * 100.0);

    for (int i = 0; i < numOfVars; i++)
    {
        if (vars.at(i).name == name)
        {
            vars[i].stat.count = rndCount;
            vars[i].stat.mean = Vector2D(0.0, 0.0);
            vars[i].stat.m2 = Vector2D(0.0, 0.0);
            vars[i].stat.variance = Vector2D(0.0, 0.0);

            double *rndSegments = new double[vars[i].numOfRegions+1];

            rndSegments[0] = 0.0;
            for (int j = 1; j < vars[i].numOfRegions; j++)
            {
                rndSegments[j] = rndSegments[j-1] + (drand48() * (1.0 - rndSegments[j-1]));
            }
            rndSegments[vars[i].numOfRegions] = 1.0;

            for (int j = 0; j < vars[i].numOfRegions; j++)
            {                
                vars[i].regions[j].stat.count = floor((rndSegments[j+1] - rndSegments[j]) * rndCount);
                vars[i].regions[j].stat.mean = Vector2D(0.0, 0.0);
                vars[i].regions[j].stat.m2 = Vector2D(0.0, 0.0);
                vars[i].regions[j].stat.variance = Vector2D(0.0, 0.0);
            }
            delete [] rndSegments;
            return;
        }
    }
    qWarning() << "randmoizeVar | Variable not found! :" << name;
}
