#ifndef STATISTICALANALYSIS_H
#define STATISTICALANALYSIS_H

#include <QList>
#include <QString>
#include "geom.h"

struct statStat
{
    double count;
    Vector2D mean;
    Vector2D variance;
    Vector2D m2;
};

struct statRegion
{
    QString name;
    Rect2D rect;
    statStat stat;
};

struct statVar
{
    QString name;
    QList<statRegion> regions;
    int numOfRegions;
    statStat stat;
};

class CStatisticalAnalysis
{
private:
    QList<statVar> vars;
    int numOfVars;

   /* Incremental mean and variance update for stat */
    void updateMeanAndVariance(statStat &stat, Vector2D newVal);
public:
    CStatisticalAnalysis();
    void addStatVariable(QString name);
    void addRegionToVariable(QString name, QString regName, Rect2D rect);

    void updateVar(QString name, Vector2D pos);
    void resetVar( QString name);
    void randomizeVar(QString name);
    statVar getVar(QString name);

    QString statToString(statStat var);

};

#endif // STATISTICALANALYSIS_H
