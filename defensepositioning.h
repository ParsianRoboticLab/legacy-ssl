#ifndef DEFENSEPOSITIONING_H
#define DEFENSEPOSITIONING_H

#include <QString>
#include <QList>
#include <QObject>
#include "geom.h"

#define DEFENSE_LOOKUPTBL_MAGIC    (quint32)0xFFEE1122

struct DefenseState {
    float x1, y1, x2, y2, x0, y0; //defender1-defender2-goalie
    DefenseState() {invalidate();}
    void invalidate() {x0 = x1 = x2 = y0 = y1 = y2 = -20;}
    bool valid() {if (x0 != -20) return true;if (x1 != -20) return true;if (x2 != -20) return true;if (y0 != -20) return true;if (y1 != -20) return true;if (y2 != -20) return true;return false;}
};

class CDefensePositioning : public QObject
{
    Q_OBJECT
private:
    int _m, _n;
    QList<QList<DefenseState> > defenseStates;
public:
    CDefensePositioning();    
    void createLookupTable(int defenders, bool goalie, double ballMinDistFromPenaltyArea, int m, int n);
    bool saveLookupTableToFile(QString filename);
    bool loadLookupTableFromFile(QString filename);
    void getPositions(Vector2D ball, Vector2D& goalie, Vector2D& def1, Vector2D& def2);
signals:
    void update();
};

extern CDefensePositioning* defensePositioning;

#endif // DEFENSEPOSITIONING_H
