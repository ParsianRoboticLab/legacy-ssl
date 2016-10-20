#ifndef LOADPLAYOFFJSON_H
#define LOADPLAYOFFJSON_H

#include "planloader.h"
#include "playoff.h"

class CLoadPlayOffJson : public CPlanLoader
{
public:
    CLoadPlayOffJson(QObject* = 0);
    CLoadPlayOffJson(const QString& _folderDirectory, QObject* parent = 0);



    void loadAll();
    void load(QString _file);
    void save(QString _file);
    void saveAll();

protected:
    //All of Them !
    QList< QList< SPlayOffPlan* > > m_plans;



private:
};

#endif // LOADPLAYOFFJSON_H
