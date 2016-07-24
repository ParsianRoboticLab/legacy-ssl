#include <QApplication>
#include <QMainWindow>
#include <signal.h>
#include "mainapplication.h"
#include "gamelogger.h"
#include <QProcess>
//#include <QMaageBox.h>

void handle(int)
{
    haltAllRobots();
    policy()->save();
    conf()->save();

//    char *params[] = {"mode", "real"};

//    execve("./ai", params, NULL);
//   QProcess* q = new QProcess;
//   if (knowledge->getGameState() == CKnowledge::Stop)
//   q->startDetached("./ai", QStringList() << "mode" << "real" << "ref" << "S");
//   else
//       q->startDetached("./ai", QStringList() << "mode" << "real" << "ref" << "s");
//   delete q;
//        system("./ai mode real");
    qDebug() << "Code Terminated with Segmentation Fault, all robots are freezed.";
    exit(100);
	//draw(" Segmentation Fault",Vector2D(0,0),"red",0);
}

void handle2(int)
{
	haltAllRobots();
//			system("./ai mode real");
    qDebug() << "GDB Paused program.";
//	system("./ai");
	//draw(" Segmentation Fault",Vector2D(0,0),"red",0);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    startUpMode = "";
    int c = app.arguments().count();
    for (int i=0;i<c;i++)
    {
        if (app.arguments().at(i).toLower() == "mode")
        {
            if (i != c-1)
            {
                QString s = app.arguments().at(i+1).toLower();
                startUpMode = s;
            }
        }
        if (app.arguments().at(i).toLower() == "ref")
        {
            if (i != c-1)
            {
                QString s = app.arguments().at(i+1).toLower();
                coachMode = s;
            }
        }
    }

    signal(SIGSEGV,handle);

    signal(SIGALRM,handle2);

    CMainApplication mApp;

    mApp.show();

    return app.exec();
}
