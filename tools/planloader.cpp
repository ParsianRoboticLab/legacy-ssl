#include "planloader.h"

CPlanLoader::CPlanLoader(QObject* parent) : QObject(parent){
    watcher = new FileWatcher(this);
    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(slt_fileChanged(QString)));
}

CPlanLoader::CPlanLoader(const QString &_folderDirectory, QObject *parent) : QObject(parent) {

    qDebug() << "A Plan Loader Started";

    watcher = new FileWatcher(this);

    m_directory = _folderDirectory;
    QDir directory(m_directory);

    m_dirList     = directory.entryList();
    m_fileInfoList = directory.entryInfoList();
    Q_FOREACH(QString file, m_dirList) {
        if (file.endsWith(".json")) {
            if (watcher->addFile(m_directory + QDir::separator() + file)) {
                qDebug() << file << "Added Succecfully and is be watching";
            }
            else {
                qDebug() << file << "founded but can't be wathced :( so it's removed !";
                m_dirList.removeOne(file);
            }
        }
    }


    QDirIterator it(QDir::currentPath() + QString("/playoff"), QDirIterator::FollowSymlinks | QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString temp = it.next();
        if (temp.endsWith(".json")) {
            if (it.fileInfo().isSymLink())
                temp = it.fileInfo().symLinkTarget();
            qDebug() << " --- > "<< temp;
        }
    }

    connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(slt_fileChanged(QString)));
}

//Public Slots

void CPlanLoader::slt_fileChanged(const QString &_file) {
//    if ()
    qDebug() << _file << "Changed.";
}
