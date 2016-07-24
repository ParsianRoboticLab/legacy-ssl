#include "joystick.h"
#include <QDebug>
#include <knowledge.h>

#include <QApplication>


const int CJoystick::BUTTON_1 =  (1 << 0);
const int CJoystick::BUTTON_2 =  (1 << 1);
const int CJoystick::BUTTON_3 =  (1 << 2);
const int CJoystick::BUTTON_4 =  (1 << 3);
const int CJoystick::BUTTON_5 =  (1 << 4);
const int CJoystick::BUTTON_6 =  (1 << 5);
const int CJoystick::BUTTON_7 =  (1 << 6);
const int CJoystick::BUTTON_8 =  (1 << 7);
const int CJoystick::BUTTON_9 =  (1 << 8);
const int CJoystick::BUTTON_10 =  (1 << 9);
const int CJoystick::BUTTON_11 =  (1 << 10);
const int CJoystick::BUTTON_12 =  (1 << 11);

void CJoystick::initJs()
{
    fd = open ("/dev/input/js0", O_RDONLY);
    ioctl(fd, JSIOCGAXES, &axes);
    ioctl(fd, JSIOCGBUTTONS, &buttons);
}

CJoystick::CJoystick()
{
//#ifndef NO_JS
    axes = 8;
    buttons =11;
    initJs();
    axis = new int[axes];
    button = new int [buttons];
//#endif
}

bool CJoystick::isJsConnected()
{
    initJs();
    if(fd == -1)
        return false;
    else
        return true;
}

void CJoystick::run()
{
//#ifndef NO_JS
#ifdef GAME_MODE
   return;
#endif

   while(1)
   {
       //msleep(90);
       read(fd, &js, sizeof(struct js_event));
       mutex.lock();
       //draw(QString("js: %1").arg(js.type),Vector2D(0,-1));
       switch (js.type & ~JS_EVENT_INIT) {
       case JS_EVENT_BUTTON:
           button[js.number]=js.value;
           break;
       case JS_EVENT_AXIS:
           axis[js.number]=js.value;
           break;
       }
       mutex.unlock();
   }


//#endif
}
//#ifndef NO_JS

bool CJoystick::getButton1() { return button[0]; }
bool CJoystick::getButton2() { return button[1]; }
bool CJoystick::getButton3() { return button[2]; }
bool CJoystick::getButton4() { return button[3]; }
bool CJoystick::getButton5() { return button[4]; }
bool CJoystick::getButton6() { return button[5]; }
bool CJoystick::getButton7() { return button[6]; }
bool CJoystick::getButton8() { return button[7]; }
bool CJoystick::getButton9() { return button[8]; }
bool CJoystick::getButton10() { return button[9]; }
bool CJoystick::getButton11() { return button[10]; }
bool CJoystick::getButton12() { return button[11]; }


float CJoystick::getAxes(int i)
{
	float a = 0;
    mutex.lock();
	a = (float)(axis[i]) / 32767.0;
    mutex.unlock();
    return a;
}
//#else

//bool CJoystick::getButton1() { return false; }
//bool CJoystick::getButton2() { return false; }
//bool CJoystick::getButton3() { return false; }
//bool CJoystick::getButton4() { return false; }
//bool CJoystick::getButton5() { return false; }
//bool CJoystick::getButton6() { return false; }
//bool CJoystick::getButton7() { return false; }
//bool CJoystick::getButton8() { return false; }
//bool CJoystick::getButton9() { return false; }
//bool CJoystick::getButton10() { return false; }
//bool CJoystick::getButton11() { return false; }
//bool CJoystick::getButton12() { return false; }

//float CJoystick::getAxes(int i)
//{
//    float a = 0;
//    mutex.lock();
//    if ( jsAxes.count() > i)
//        a = jsAxes.at(i);
//    mutex.unlock();
//    return a;
//}

//#endif
