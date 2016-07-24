#include "launcher.h"
#include <QDebug>
CLauncher::CLauncher(QObject *parent) :
    QObject(parent)
{
    process = NULL;
}

void CLauncher::run()
{
    qDebug() << "Launching...";
    process = new QProcess;
//    connect(process, SIGNAL(error(QProcess::ProcessError)))
    process->setWorkingDirectory("../../");
    process->execute("../../ai.app/Contents/MacOS/ai", QStringList() << "mode" << "real" );
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    qDebug() << "Launched.";
}

void CLauncher::finished(int code, QProcess::ExitStatus e)
{
    qDebug() << "Finished!";
    if (code != 0)
    {
        qDebug() << "Process Crashed, running again.";
        disconnect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
        delete process;
        run();
    }
    else {
        qDebug() << "Process Quited Normally, running again.";
        emit quit();
    }
}
