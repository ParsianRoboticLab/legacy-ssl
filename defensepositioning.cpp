#include "defensepositioning.h"
#include <knowledge.h>
#include <QApplication>

CDefensePositioning* defensePositioning;

DefensePositions defpos(DefenseState t)
{
    DefensePositions s;
    s.defense.append(Vector2D(t.x1, t.y1));
    s.defense.append(Vector2D(t.x2, t.y2));
    s.goalie = Vector2D(t.x0, t.y0);
    if (s.goalie.x < -10) s.goalie.invalidate();
    if (s.defense[0].x < -10) s.defense[0].invalidate();
    if (s.defense[1].x < -10) s.defense[1].invalidate();
    return s;
}


CDefensePositioning::CDefensePositioning()
{
}
//this is a time burning function; should be called once and the output should be saved
void CDefensePositioning::createLookupTable(int defenders, bool goalie, double ballMinDistFromPenaltyArea, int m, int n)
{    
    if (defenders > 2) return;
    debug("creating defense lookup table", D_ERROR);
    QList<Circle2D> avoid;
    defenseStates.clear();
    double w = wm->field->oppCornerL().x;
    double h = wm->field->oppCornerL().y;
    Vector2D ballpos = wm->ball->pos;
    for (int i=-m;i<=m;i++)
    {
        defenseStates.append(QList<DefenseState>());
        for (int j=0;j<=n;j++)
        {            
            Vector2D point(w*i/m, h*j/n);
            bool flag = false;
            if (!wm->field->isInOurPenaltyArea(point))
            {
                QList<Vector2D> intersects = wm->field->ourPAreaIntersect(Segment2D(point, wm->field->ourGoal()));
                if (intersects.count() > 0)
                {
                    if ((intersects[0] - point).length() > ballMinDistFromPenaltyArea) flag = true;
                }
            }
            draw(point, 1, "red");
            DefenseState s;
            s.invalidate();
            QList<Vector2D> defPos;
            Vector2D goaliePos;
            if (flag)
            {
                wm->ball->pos = point;
                QList<DefensePositions> neighbourhood;
                if (defenseStates.last().count() >= 1)
                    neighbourhood.append(defpos(defenseStates.last().last()));
                if (defenseStates.count() > 1)
                    neighbourhood.append(defpos(defenseStates[defenseStates.count() - 2][defenseStates.last().count()]));
                knowledge->generateDefensePositions(defenders, goalie, avoid, defPos, goaliePos, neighbourhood, 0.02);
                if (defenders >= 1)
                {
                    s.x1 = defPos[0].x;
                    s.y1 = defPos[0].y;
                }
                if (defenders >= 2)
                {
                    s.x2 = defPos[1].x;
                    s.y2 = defPos[1].y;
                }
                if (goalie)
                {
                    s.x0 = goaliePos.x;
                    s.y0 = goaliePos.y;
                }                
            }            
            defenseStates.last().append(s);
            //debug(QString("defense state = %1 %2|state=%3").arg(i).arg(j).arg(defenseStates.count()), D_ERROR);
            qApp->processEvents();
            emit update();
        }

    }    
    wm->ball->pos = ballpos;
}

bool CDefensePositioning::saveLookupTableToFile(QString filename)
{
    if (defenseStates.count() == 0) return false;
    QDataStream s;
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly)) return false;
    s.setDevice(&f);
    quint32 m = (defenseStates.count() - 1) / 2;
    quint32 n = defenseStates[0].count();
    debug(QString("saving this:%1 %2").arg(m).arg(n), D_ALI);
    s << DEFENSE_LOOKUPTBL_MAGIC;
    s << m;
    s << n;
    for (int i=0;i<defenseStates.count();i++)
    {
        for (int j=0;j<defenseStates[0].count();j++)
        {
            s << defenseStates[i][j].x0;
            s << defenseStates[i][j].y0;
            s << defenseStates[i][j].x1;
            s << defenseStates[i][j].y1;
            s << defenseStates[i][j].x2;
            s << defenseStates[i][j].y2;
        }
    }    
    debug(QString("saved defense positionings to file %1").arg(filename), D_ERROR);
    return true;
}

bool CDefensePositioning::loadLookupTableFromFile(QString filename)
{    
    QDataStream s;
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) return false;
    s.setDevice(&f);
    quint32 magic;
    s >> magic;    
    if (magic != DEFENSE_LOOKUPTBL_MAGIC) return false;
    defenseStates.clear();
    quint32 m, n;
    s >> m;
    s >> n;
    debug(QString("m,n=%1 %2").arg(m).arg(n),D_ALI);
    int mm = m;
    int nn = n;    
    for (int i=-mm;i<=mm;i++)
    {
        defenseStates.append(QList<DefenseState> ());
        for (int j=0;j<=nn;j++)
        {
            DefenseState st;
            s >> st.x0;
            s >> st.y0;
            s >> st.x1;
            s >> st.y1;
            s >> st.x2;
            s >> st.y2;
            defenseStates.last().append(st);
        }
    }    
    debug(QString("m,n=%1 %2").arg(defenseStates.count()).arg(defenseStates[0].count()),D_ALI);
    return true;
}

void CDefensePositioning::getPositions(Vector2D ball, Vector2D& goalie, Vector2D& def1, Vector2D& def2)
{
    double w = wm->field->oppCornerL().x;
    double h = wm->field->oppCornerL().y;
    quint32 m = (defenseStates.count() - 1) / 2;    
    if (m == 0) return;
    quint32 n = defenseStates[0].count();
    //debug(QString("def=%1 %2").arg(m).arg(n), D_ALI);
    int x = round(ball.x*m/w) + m;
    int y = round(ball.y*n/h);
    if (y<0) y = -y;
    if (x>=2*m+1) x=2*m;
    if (y>=n) y=n-1;
    if (x<0) x=0;
    if (y<0) y=0;
    //debug(QString("xy = %1 %2 %3 %4").arg(x).arg(y).arg(w).arg(h), D_ALI);
    goalie.assign(defenseStates[x][y].x0, defenseStates[x][y].y0);
    def1.assign(defenseStates[x][y].x1, defenseStates[x][y].y1);
    def2.assign(defenseStates[x][y].x2, defenseStates[x][y].y2);
}

