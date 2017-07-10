#ifndef MIXTEAMREADER_H
#define MIXTEAMREADER_H

#include <QObject>
#include <proto/multi_team_communication.pb.h>
#include <QUdpSocket>
#include <varswidget.h>
#include <knowledge.h>



class MixTeamReader : public QObject
{
    Q_OBJECT
public:
    explicit MixTeamReader(QObject *parent = 0);
    virtual ~MixTeamReader();

signals:

public slots:
    void readyRead();

private:
    QUdpSocket *socket;

};

#endif // MIXTEAMREADER_H
