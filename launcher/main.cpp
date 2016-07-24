#include <QtCore/QCoreApplication>
#include <QProcess>
#include <QTextStream>
#include <iostream>
#include <QDebug>
#include <QTimer>
#include "launcher.h";
#include <QDir>
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int code = 0;
#ifdef Q_WS_MACX
        QDir::setCurrent("../ai.app/Contents/MacOS/");
#else
        QDir::setCurrent("../");
#endif
	do {
        qDebug() << "Launching...";
        QProcess* process = new QProcess;
        process->setWorkingDirectory("./");
        code = process->execute("./ai", QStringList() << "mode" << "real" );
		if (code != 0) qDebug() << "Process Crashed, running again.";
        delete process;
    } while (code!=0);
    return 0;
}
