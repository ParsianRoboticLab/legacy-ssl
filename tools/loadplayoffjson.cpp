#include "loadplayoffjson.h"

CLoadPlayOffJson::CLoadPlayOffJson(QObject* parent) : CPlanLoader(parent) {
}

CLoadPlayOffJson::CLoadPlayOffJson(const QString &_folderDirectory, QObject *parent)
    : CPlanLoader(_folderDirectory, parent) {

    qDebug() << "Plan Loader -> PlayOff Json ";
}
