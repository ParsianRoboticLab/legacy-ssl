#ifndef LOADPLAYOFFJSON_H
#define LOADPLAYOFFJSON_H


#include <QVariant>

#include "planloader.h"
#include "playoff.h"

#define API_VERSION 1.2

class CLoadPlayOffJson : public CPlanLoader
{
public:
    CLoadPlayOffJson(QObject* = 0);
    CLoadPlayOffJson(const QString& _folderDirectory, QObject* parent = 0);

    bool loadAll();
    bool load(QString _file);
    bool save(QString _file);
    bool saveAll();

protected:
    //All of Them !
    QList< QList< NGameOff::SPlan* > > m_plans;
    bool readPlan(const QVariantMap& _map, const QString& _file);
    void fillMatching(NGameOff::SMatching& _matching, const QVariantMap& _plan, bool* _parsedOK = 0);
    void fillCommon(NGameOff::SCommon& _common, const QVariantMap& _plan, bool* _parsedOk = 0);
    void fillExecution(NGameOff::SExecution& _execution, const QVariantMap& _plan, bool* _parsedOk = 0);
    void fillGUI(NGameOff::SGUI& _gui, const QFileInfo& _fileInfo, bool* _parsedOk = 0);
    POffSkills strToEnum(const QString& _str);
    QString getPackageName(QString _path);

private:


};

#endif // LOADPLAYOFFJSON_H
