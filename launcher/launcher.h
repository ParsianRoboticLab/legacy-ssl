#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QObject>
#include <QProcess>

class CLauncher : public QObject
{
    Q_OBJECT
public:
    QProcess* process;
    explicit CLauncher(QObject *parent = 0);
    void run();
signals:
    void quit();
public slots:
    void finished(int,QProcess::ExitStatus);

};

#endif // LAUNCHER_H
