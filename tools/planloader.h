#ifndef PLANLOADER_H
#define PLANLOADER_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QDirIterator>

#include "filewatcher.h"

#define Property(type,name,local) \
        public: inline type& get##name() {return local;} \
        public: inline void set##name(type val) {local = val;} \
        protected: type local


struct SFile {
    QString dir;
    QFileInfo fileInfo;
    QFile file;
};

class CPlanLoader : public QObject
{

    Q_OBJECT

public:
    CPlanLoader(QObject* parent = 0);
    CPlanLoader(const QString& _folderDirectory, QObject * parent = 0);

    bool load(int _index);
    bool load(QString _directory);
    bool save(int _index);
    bool save(QString _directory);

protected:
    FileWatcher* watcher;
    QString m_mainDirectory;
    QList<SFile> m_files;
    QStringList m_dirList;
    QFileInfoList m_fileInfoList;
    QList<QFile> m_fileList;


    Property(bool, AutoUpdate,autoUpdate);

private:

public slots:
    void slt_fileChanged(const QString& _file);
};



#endif // PLANLOADER_H

