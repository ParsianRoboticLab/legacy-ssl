#include "logger.h"
#include "drawer.h"
#include "ui/widgets.h"
#include "gamelogger.h"
#include "base.h"

QMutex drawMutex;
CDrawer* drawer;  
CInfoWidget* info;  
QQueue<CStatusText>* logger;  

int loglevel=-1000;
int drawlevel=-1000;


bool drawLock(int ms)
{
	return true;
	if (ms < 0)
	{
		drawMutex.lock();
		return true;
	}
	else return drawMutex.tryLock(ms);
}

void drawUnlock()
{
	return;
	drawMutex.unlock();
}


void initInfo(void *_info)
{
    info = (CInfoWidget*) _info;
}

void initLogger(void* _logger)
{
    logger = (QQueue<CStatusText>*) _logger;
}

void initDrawer(void* _drawer)
{
    drawer = (CDrawer*) _drawer;
}

void setDebugLevel(int l)
{
    loglevel = l;
}

void analyze(QString key,QVariant value, bool SaveSameValues){
    gameLogger->AddToAnalyze(key,value,SaveSameValues);
}



void debug(QString text, long type, QColor color)
{
#ifdef GAME_MODE
    return;
#endif
 long mask = 0;
 long mask1 =0;
 bool typeState=0;
    if (conf() == NULL) return;
    if (conf()->Performance_Debug_debugGame())       mask  = mask  | 1;
    if (conf()->Performance_Debug_debugExperiment()) mask  = mask  | 2;
    if (conf()->Performance_Debug_debugDebug())      mask  = mask  | 4;
    if (conf()->Performance_Debug_debugNadia())      mask  = mask  | 8;
    if (conf()->Performance_Debug_debugMani())       mask  = mask  | 16;
    if (conf()->Performance_Debug_debugArash())      mask  = mask  | 32;
    if (conf()->Performance_Debug_debugAli())        mask  = mask  | 64;
    if (conf()->Performance_Debug_debugSepehr())     mask  = mask  | 128;
    if (conf()->Performance_Debug_debugMasood())     mask  = mask  | 256;
    if (conf()->Performance_Debug_debugMohammed())   mask  = mask  | 512;
    if (conf()->Performance_Debug_debugHossein())    mask  = mask  | 1024;
    if (conf()->Performance_Debug_debugKK())         mask  = mask  | 2048;
    if (conf()->Performance_Debug_debugDONMHMMD())   mask  = mask  | 4096;
    if (conf()->Performance_Debug_debugERF())        mask  = mask  | 8192;
    if (conf()->Performance_Debug_debugMAHI())       mask  = mask  | 16384;
    if (conf()->Performance_Debug_debugMahmood())    mask  = mask  | 32768;
    if (conf()->Performance_Debug_debugFatemeh())    mask1 = mask1 | 1;
    if (conf()->Performance_Debug_debugAtousa())     mask1 = mask1 | 2;
    if (conf()->Performance_Debug_debugAHZ())        mask1 = mask1 | 4;
    if (conf()->Performance_Debug_debugAmin())       mask1 = mask1 | 8;
    if (conf()->Performance_Debug_debugAmiR())       mask1 = mask1 | 16;
    if (conf()->Performance_Debug_debugHamed())      mask1 = mask1 | 32;

    if(type>32768 && (type & mask1)) {
        typeState=true;

    } else if(type<=32768 && (type & mask)) {
        typeState=true;

    }

    if (typeState || (type==D_ERROR)) {
        logger->enqueue(CStatusText(text,color));

    }

    loggerMutex->lock();

    if( gameLogger->getIsLogMode() ){
        gameLogger->addToDebugs(text , type);

    }

    loggerMutex->unlock();
}

void debug0(QString text, long type, QColor color)
{
	static int cycleCnt = 0;
	cycleCnt++;
	if( cycleCnt < 60 ){
		return;
	}
	cycleCnt = 0;
#ifdef GAME_MODE
	return;
#endif
        long mask = 0;
	if (conf() == NULL) return;
	if (conf()->Performance_Debug_debugGame()) mask = mask | 1;
	if (conf()->Performance_Debug_debugExperiment()) mask = mask | 2;
	if (conf()->Performance_Debug_debugDebug()) mask = mask | 4;
        if (conf()->Performance_Debug_debugNadia()) mask = mask | 8;
	if (conf()->Performance_Debug_debugMani()) mask = mask | 16;
	if (conf()->Performance_Debug_debugArash()) mask = mask | 32;
	if (conf()->Performance_Debug_debugAli()) mask = mask | 64;
	if (conf()->Performance_Debug_debugSepehr()) mask = mask | 128;
	if (conf()->Performance_Debug_debugMasood()) mask = mask | 256;
	if (conf()->Performance_Debug_debugMohammed()) mask = mask | 512;
	if (conf()->Performance_Debug_debugHossein()) mask = mask | 1024;
        if (conf()->Performance_Debug_debugKK()) mask = mask | 2048;
        if (conf()->Performance_Debug_debugDONMHMMD()) mask = mask | 4096;
        if (conf()->Performance_Debug_debugERF()) mask = mask | 8192;
        if (conf()->Performance_Debug_debugMAHI()) mask = mask | 16384;
        if (conf()->Performance_Debug_debugMahmood()) mask = mask | 32768;
        if (conf()->Performance_Debug_debugFatemeh()) mask = mask | 65536;
        if (conf()->Performance_Debug_debugAtousa()) mask = mask | 131072;
        if (conf()->Performance_Debug_debugAHZ()) mask = mask | 262144;
        if (conf()->Performance_Debug_debugAmin()) mask = mask | 524288;
        if (conf()->Performance_Debug_debugAmiR()) mask = mask | 1048576;
        if (conf()->Performance_Debug_debugHamed()) mask = mask | 2097152;


	if ((type & mask) != 0 || (type==D_ERROR))
	{

		logger->enqueue(CStatusText(text,color));
	}
	loggerMutex->lock();
	if( gameLogger->getIsLogMode() ){
		gameLogger->addToDebugs(text , type);
	}
	loggerMutex->unlock();
}

void setDrawLevel(int l)
{
    drawlevel = l;
}

void draw (Rect2D _rect, QColor _color, bool _filled, int level , bool toBeLoged )
{
	drawLock();
    if (level>=drawlevel)
        drawer->draw(_rect,_color,_filled);

    if( toBeLoged ){
        loggerMutex->lock();
        if( gameLogger->getIsLogMode() ){
            gameLogger->addToDraws(_rect , _color , _filled);
        }
        loggerMutex->unlock();
    }
	drawUnlock();
}

void draw (QString _text, Vector2D _pos, QColor _color, int _size, int level, bool toBeLoged )
{
	drawLock();
	if (level>=drawlevel)
        drawer->draw(_text, _pos, _color, _size);
    if( toBeLoged ){
        loggerMutex->lock();
        if( gameLogger->getIsLogMode() ){
            gameLogger->addToDraws(_text , _pos , _color , _size);
        }
        loggerMutex->unlock();
    }
	drawUnlock();
}

void draw (Circle2D _circle, int _startAng, int _endAng, QColor _color, bool _filled, int level, bool toBeLoged )
{
	drawLock();
    if (level>=drawlevel)
        drawer->draw(_circle, _startAng, _endAng, _color, _filled);

    if( toBeLoged ){
        loggerMutex->lock();
        if( gameLogger->getIsLogMode() ){
            gameLogger->addToDraws(_circle , _startAng , _endAng , _color , _filled );
        }
        loggerMutex->unlock();
    }
	drawUnlock();
}

void draw (Circle2D _circle, QColor _color, bool _filled, int level, bool toBeLoged )
{
	drawLock();
    if (level>=drawlevel)
        drawer->draw(_circle, 0, 360, _color, _filled);

    if( toBeLoged ){
        loggerMutex->lock();
        if( gameLogger->getIsLogMode() ){
            gameLogger->addToDraws(_circle , 0, 360 , _color , _filled );
        }
        loggerMutex->unlock();
    }
	drawUnlock();
}

void draw (Polygon2D _polygon, QColor _color, bool _filled, int level, bool toBeLoged )
{
	drawLock();
    if (level>=drawlevel)
        drawer->draw(_polygon, _color, _filled);    
	drawUnlock();
}

void draw (Segment2D _seg, QColor _color, int level, bool toBeLoged )
{
	drawLock();
    if (level>=drawlevel)
        drawer->draw(_seg, _color);

    if( toBeLoged ){
        loggerMutex->lock();
        if( gameLogger->getIsLogMode() ){
            gameLogger->addToDraws(_seg , _color);
        }
        loggerMutex->unlock();
    }
	drawUnlock();
}

void draw (Vector2D _point, int _type, QColor _color, int level, bool toBeLoged )
{
	drawLock();
    if (level>=drawlevel)
        drawer->draw(_point, _type, _color);

    if( toBeLoged ){
        loggerMutex->lock();
        if( gameLogger->getIsLogMode() ){
            gameLogger->addToDraws(_point , _color);
        }
        loggerMutex->unlock();
    }
	drawUnlock();
}
