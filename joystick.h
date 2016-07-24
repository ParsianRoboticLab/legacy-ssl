#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "js.h"

#include <QObject>
#include <QThread>
#include <QMutex>

#include<linux/input.h>
#include <linux/joystick.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

//#define NO_JS

class CJoystick : public QThread
{
    Q_OBJECT
    QMutex mutex;
public:
    int fd;
    unsigned char axes,buttons;
    int *axis;
    int *button;
    js_event js;


    static const int BUTTON_1;
    static const int BUTTON_2;
    static const int BUTTON_3;
    static const int BUTTON_4;
    static const int BUTTON_5;
    static const int BUTTON_6;
    static const int BUTTON_7;
    static const int BUTTON_8;
    static const int BUTTON_9;
    static const int BUTTON_10;
    static const int BUTTON_11;
    static const int BUTTON_12;

    CJoystick();
    void run();
    void initJs();
    bool isJsConnected();

    bool getButton1();
    bool getButton2();
    bool getButton3();
    bool getButton4();
    bool getButton5();
    bool getButton6();
    bool getButton7();
    bool getButton8();
    bool getButton9();
    bool getButton10();
    bool getButton11();
    bool getButton12();

    float getAxes( int i);

};

#endif  //JOYSTICK_H
