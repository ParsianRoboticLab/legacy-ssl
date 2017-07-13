#include "agent.h"
#include <QDebug>
#include <QFile>
#include "kick.h"
#include <time.h>

//#define debug_train
//#define skuba_control
//#define use_ANN

#define errlen 100

double getVar( double data[errlen] )
{
    double mean = 0.0;
    for( int i = 0 ; i < errlen ; i++ )
    {
        mean += data[i];
    }
    mean /= errlen;

    double var = 0.0;
    double error;
    for( int i = 0 ; i < errlen ; i++ )
    {
        error = data[i]-mean;
        var += error*error;
    }
    var /= errlen;
    var = sqrt(var);
    return var;
}

Matrix tansig( Matrix n )
{
    for(int i=0 ; i<n.nrows() ; i++ )
        for( int j=0 ; j<n.ncols() ; j++ )
            n.e(i , j) = 2.0/(1.0+exp(-2.0*n.e(i , j)))-1.0;
    return n;
}

Matrix ANN_forward( Matrix input )
{
    static Matrix b1(10,1),b2(4,1),IW(10,3),LW(4,10);
    static bool run = true;
    if(run)
    {
        fstream file1,file2,file3,file4;
        file1.open("test/b1.txt",fstream::in);
        file2.open("test/b2.txt",fstream::in);
        file3.open("test/IW.txt",fstream::in);
        file4.open("test/LW.txt",fstream::in);
        for( int i = 0; i < 10 ; i++ )
        {
            file1 >> b1.e(i,0);
        }
        for( int i = 0; i < 4 ; i++ )
        {
            file2 >> b2.e(i,0);
        }
        for( int i = 0; i < 10 ; i++ )
        {
            for( int j = 0; j < 3 ; j++ )
            {
                file3 >> IW.e(i,j);
            }
        }
        for( int i = 0; i < 4 ; i++ )
        {
            for( int j = 0; j < 10 ; j++ )
            {
                file4 >> LW.e(i,j);
            }
        }
        run = false;
    }

    Matrix output(4,1);
    Matrix xmin(3,1);
    xmin.e(0,0) = -0.561029;
    xmin.e(1,0) = -0.533322;
    xmin.e(2,0) = -57.9859;
    Matrix xmax(3,1);
    xmax.e(0,0) = 0.57;
    xmax.e(1,0) = 0.5344;
    xmax.e(2,0) = 56.928;

    input = input - xmin;
    for( int i = 0; i<input.nrows() ; i++ )
        for( int j = 0; j<input.ncols() ; j++ )
        {
            input.e(i,j) *= (2.0)/(xmax.e(i,j)-xmin.e(i,j));
            input.e(i,j) -= 1.0;
        }

    output = LW * tansig( IW * input + b1 ) + b2;

    Matrix Xmin(4,1);
    Xmin.e(0,0) = -27.3596;
    Xmin.e(1,0) = -28.2358;
    Xmin.e(2,0) = -28.2358;
    Xmin.e(3,0) = -27.3596;
    Matrix Xmax(4,1);
    Xmax.e(0,0) = 27.3596;
    Xmax.e(1,0) = 28.2358;
    Xmax.e(2,0) = 28.2358;
    Xmax.e(3,0) = 27.3596;

    for( int i = 0; i<output.nrows() ; i++ )
        for( int j = 0; j<output.ncols() ; j++ )
        {
            output.e(i,j) -= -1;
            output.e(i,j) *= (Xmax.e(i,j)-Xmin.e(i,j))/(2.0);
        }

    output = output + Xmin;
    return output;
}

CAgent::CAgent(short int _ID)
{
    srand48(time(0));
    packetNum = 0;
    stopTrain=false;wh1=wh2=wh3=wh4=0.0;startTrain=false;
    selfID = _ID;
    skill = NULL;

    for( int i=0 ; i<_PACKET_SIZE ; i++ )
        outputBuffer[i] == 0x0;

    onOffState = true;
    commandID = selfID;

    chip = false;
    forceKick = false;
    canRecvPass = false;
    roller = 0;
    kickSpeed = 0;

    v1 = 0;
    v2 = 0;
    v3 = 0;
    v4 = 0;
    vforward  = 0;
    vnormal   = 0;
    vangular  = 0;
    lastVf =0;
    lastVn = 0;
    lowSpeedMode = false;
    starter = false;
    beep = false;

    sumEX = sumEY = 0;
    lastEX = lastEY = 0;
    goalVisibility = -1;
    idle = false;
    ffout.open("send.csv",ios::out);

    //Both codes below do the same work!

    loadProfiles();

    agentAngelForGyro = Vector2D(1,0);
    calibrateGyro = false;
    calibrated = 900;
    this->abilities.canChip = true;
    this->abilities.canKick = true;
    this->abilities.canSpin = true;
    this->abilities.hasGyro = false;

    homePos.assign(0 , 0);
    intention = NULL;
    _ACC = 0;
    _DEC = 0;
    agentStopTime.start();

    changeIsNeeded = false;

}

void CAgent::loadProfiles()
{
    //Set shotProfile From File
    {
        for( int i = 0 ; i<32 ; i++ )
            shotProfile[i][0] = shotProfile[i][1] = -1.0;
        QFile file(QString("profiles/robot%1.shot").arg(selfID));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            //            qDebug()<<(QString("Parsing profiles/robot%1.shot").arg(selfID));

            while (!file.atEnd()) {
                QByteArray line = file.readLine();

                QStringList l = QString(line).split(',');
                int index = l[0].toInt();
                shotProfile[index][0] = l[1].toDouble();
                shotProfile[index][1] = l[2].toDouble();
            }

            //            qDebug()<<"shot profile for robot "<<selfID<<":";
            for (int i = 0; i < 32; i++)
            {
                qDebug() << i << " " << shotProfile[i][0] << " " << shotProfile[i][1];
            }
            file.close();
        }else
        {
            //            qDebug()<<(QString("NotFound: profiles/robot%1.shot").arg(selfID))<<file.errorString();
        }
    }

    //Set chipProfile From File
    {
        for( int i = 0 ; i<32 ; i++ )
            chipProfile[i][0] = chipProfile[i][1] = -1.0;
        QFile file(QString("profiles/robot%1.chip").arg(selfID));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            //            qDebug()<<(QString("Parsing profiles/robot%1.chip").arg(selfID));

            while (!file.atEnd()) {
                QByteArray line = file.readLine();

                QStringList l = QString(line).split(',');
                int index = l[0].toInt();
                chipProfile[index][0] = l[1].toDouble();
                chipProfile[index][1] = l[2].toDouble();
            }


            //            qDebug()<<"chip profile for robot "<<selfID<<":";
            for (int i = 0; i < 32; i++)
            {
                qDebug() << i << " " << chipProfile[i][0] << " " << chipProfile[i][1];
            }
            file.close();
        }else
        {
            //            qDebug()<<(QString("NotFound: profiles/robot%1.chip").arg(selfID))<<file.errorString();
        }
    }
}

int CAgent::id()
{
    return selfID;
}

double CAgent::getVisibility(){
    return self()->inSight;
}

void CAgent::setVisibility( const double &inSight ){
    self()->inSight = inSight;
}

bool CAgent::isVisible(){
    return self()->inSight > 0;
}

bool CAgent::notVisible(){
    return self()->inSight <= 0;
}

int CAgent::commandId()
{
    return commandID;
}

//RPM story
/// velocity type 1500
/// default mode  1300

static double decay_x=0.2;
static double decay_y=0.2;
static double decay_w=20;
static double lastVelx[errlen];
static double lastVely[errlen];
static double lastOmega[errlen];
const int decay_accel=10;
static int counter = 0;
static double bvf=0.0,bvn=0.0,bva=0.0;
static Matrix Epsilon(12,1);

bool CAgent::trajectory(double& vf,double& vn,double& va,double w1,double w2,double w3,double w4,bool &stop)
{
    static int error_set = true;
    if(error_set)
    {
        error_set = false;
        for(int i=0 ; i<errlen ; i++)
        {
            lastVelx[i] = lastVely[i] = lastOmega[i] = 0;
        }
        counter = 0;
    }

    double cvx = getVar( lastVelx );
    double cvy = getVar( lastVely );
    double cvw = getVar( lastOmega );
    double thrx = 0.10;//boundTo( 78.0 + 74.0 * Vector2D(vf,vn).length() , 130.0 , 400.0 ) * (errlen/100);
    double thry = 0.10;
    double thrw = 10.0;

#ifdef debug_train
    if( counter == errlen + 10 )
    {
        QFile mfile("./test/error.txt");
        mfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
        QTextStream mts(&mfile);
        mts << vf << " " << cvx << "\t" << vn << " " << cvy << "\t" << va << " " << cvw;
        mts << endl;
        mfile.close();
    }
#endif

    if( cvx < thrx && cvy < thry && cvw < thrw && counter == errlen + 10 )
    {
        QFile file1("./test/wheels.txt");
        QFile file2("./test/observed.txt");
        QFile file3("./test/input.txt");
        file1.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append );
        file2.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append );
        file3.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append );
        QTextStream ts1(&file1);
        QTextStream ts2(&file2);
        QTextStream ts3(&file3);

        double mVelx = 0;
        double mVely = 0;
        double mOmega = 0;
        for( int i = 0 ; i < errlen ; i++ )
        {
            mVelx += lastVelx[i];
            mVely += lastVely[i];
            mOmega += lastOmega[i];
        }
        mVelx /= errlen;
        mVely /= errlen;
        mOmega /= errlen;

        ts1 << w1 << " " << w2 << " " << w3 << " " << w4 << endl;
        ts2 << mVelx << " " << mVely << " " << mOmega << endl;
        ts3 << vf << " " << vn << " " << va << endl;

        file1.close();
        file2.close();
        file3.close();

#ifdef skuba_control
        Matrix iden(3);
        Matrix phi(1,4);
        Matrix zita_captured(3,1);
        Matrix zita_desired(3,1);
        phi.e(0,0) = w1;
        phi.e(0,1) = w2;
        phi.e(0,2) = w3;
        phi.e(0,3) = w4;
        zita_captured.e(0,0) = mVelx;
        zita_captured.e(1,0) = mVely;
        zita_captured.e(2,0) = mOmega;
        zita_desired.e(0,0) = vf;
        zita_desired.e(1,0) = vn;
        zita_desired.e(2,0) = va;
        Epsilon = Epsilon + pseudoinverse(kron(iden,phi))*(zita_desired-zita_captured);
#endif

        for(int i=0 ; i<errlen ; i++)
        {
            lastVelx[i] = lastVely[i] = lastOmega[i] = 0;
        }

        //		vf = (((double)(qrand()%201))/50.0)-2.0;
        //		vn = (((double)(qrand()%201))/50.0)-2.0;
        vf = (((double)(qrand()%201))/100.0)-1.0;
        vn = (((double)(qrand()%201))/100.0)-1.0;
        double speed = Vector2D(vf,vn).r();
        if( speed < 0.5 )
        {
            speed = 0.5;
        }
        va = boundTo((((((double)(qrand()%181))*2)-180.0)/(speed)),-180.0,180.0);
        //		if( va >= 49 )
        //		{
        //			va = -50;
        //			if( vn >= 0.4 )
        //			{
        //				vn = -.5;
        //				if( vf >= 0.4 )
        //				{
        //					return true;
        //				}
        //				else
        //				{
        //					vf += 0.25;
        //				}
        //			}
        //			else
        //			{
        //				vn += 0.25;
        //			}
        //		}
        //		else
        //		{
        //			va += 25;
        //		}
        decay_x = fabs(vf) / decay_accel;
        decay_y = fabs(vn) / decay_accel;
        decay_w = fabs(va) / decay_accel;
        stop = true;
        counter = 0;
    }
    else
    {
        for( int counter = errlen-2 ; counter > -1 ; counter-- )
        {
            lastVelx[counter+1] = lastVelx[counter];
            lastVely[counter+1] = lastVely[counter];
            lastOmega[counter+1] = lastOmega[counter];
        }
        double angle = wm->our[new_com_test_robot_id]->dir.th().radian();
        lastVelx[0] = (cos(-angle) * wm->our[new_com_test_robot_id]->vel.x) - (sin(-angle) * wm->our[new_com_test_robot_id]->vel.y);
        lastVely[0] = (sin(-angle) * wm->our[new_com_test_robot_id]->vel.x) + (cos(-angle) * wm->our[new_com_test_robot_id]->vel.y);
        lastOmega[0] = wm->our[new_com_test_robot_id]->angularVel;
        if( counter < errlen + 10 )
            counter++;
    }
    return false;
}

void CAgent::accelerationLimiter(double vf,bool diveMode)
{
    ////first Stage Accelerate Limit
    double veltan= (vel().x)*cos(dir().th().radian()) + (vel().y)*sin(dir().th().radian());
    double velnorm= -1*(vel().x)*sin(dir().th().radian()) + (vel().y)*cos(dir().th().radian());


    //    if(fabs(veltan - lastVf) > 3)
    //    {
    //        lastVf = veltan;
    //    }
    //    if(fabs(velnorm - lastVn) > 3)
    //    {
    //        lastVn = velnorm;
    //    }

    if(vel().length() > 0.2)
    {
        agentStopTime.restart();
        timerReset = false;
    }
    if(agentStopTime.elapsed() > 100 && timerReset == false)
    {
        lastVn = velnorm;
        lastVf = veltan;
        agentStopTime.restart();
        timerReset = true;
    }

    double lastV,commandV;
    double vCoef = 1;
    double tempVf = vforward , tempVn = vnormal;
    double decCoef = 1.5;

    if(vf == 0)
        decCoef = 2.5;
    ///////////////////////first Stage Acc with true angle

    double accCoef =1,realAcc = 4;
    accCoef = atan(fabs(vforward)/fabs(vnormal))/_PI*2;
    if(diveMode)
    {
        realAcc = 1.5 * accCoef*conf()->BangBang_AccMaxForward() + (1-accCoef)*conf()->BangBang_AccMaxNormal();
    }
    else
    {
        realAcc = accCoef*conf()->BangBang_AccMaxForward() + (1-accCoef)*conf()->BangBang_AccMaxNormal();

    }


#if 1
    commandV = sqrt((vforward*vforward)+(vnormal*vnormal));
    lastV = sqrt((lastVf*lastVf)+(lastVn*lastVn));

    if(commandV > (lastV + realAcc* 0.0166667))
    {
        commandV = lastV + (realAcc * 0.0166667);
    }
    vforward = commandV * sin(atan2(tempVf,tempVn));
    vnormal = commandV * cos(atan2(tempVf,tempVn));

#endif

    /////////////////Second order acc limit for trajectory planning
    if(!diveMode)
    {
        if(vforward >= 0 )
        {
            if(vforward > (lastVf + conf()->BangBang_AccMaxForward()* 0.0166667))
            {
                vforward = lastVf + (conf()->BangBang_AccMaxForward()* 0.0166667)*sign(vforward);
            }
            if(vforward < (lastVf - decCoef*conf()->BangBang_DecMax()* 0.0166667))
            {
                vforward = lastVf - (decCoef*conf()->BangBang_DecMax()* 0.0166667);
            }
        }
        else
        {
            if(vforward < (lastVf - conf()->BangBang_AccMaxForward()* 0.0166667))
            {
                vforward = lastVf - (conf()->BangBang_AccMaxForward()* 0.0166667);
            }
            if(vforward > (lastVf + decCoef*conf()->BangBang_DecMax()* 0.0166667))
            {
                vforward = lastVf + (decCoef*conf()->BangBang_DecMax()* 0.0166667);
            }
        }
    }
    debug(QString("vn: %1 , lVn :%2").arg(vnormal).arg(lastVn),D_MHMMD);
    if(vnormal >= 0)
    {
        if(diveMode)
        {
            if(vnormal > (lastVn + 10* 0.0166667))
            {
                vnormal = lastVn + (10* 0.0166667)*sign(vnormal);
            }
        }
        else
        {
            if(vnormal > (lastVn + conf()->BangBang_AccMaxNormal()* 0.0166667))
            {
                vnormal = lastVn + (conf()->BangBang_AccMaxNormal()* 0.0166667)*sign(vnormal);
            }
        }

        if(!diveMode&&(vnormal < (lastVn - decCoef*conf()->BangBang_DecMax()* 0.0166667)))
        {
            vnormal = lastVn - (decCoef*conf()->BangBang_DecMax()* 0.0166667);
        }
    }
    else
    {
        if(diveMode)
        {
            if(vnormal < (lastVn - 10* 0.0166667))
            {
                vnormal = lastVn + (10* 0.0166667)*sign(vnormal);
            }
        }
        else
        {
            if(vnormal < (lastVn - conf()->BangBang_AccMaxNormal()* 0.0166667))
            {
                vnormal = lastVn + (conf()->BangBang_AccMaxNormal()* 0.0166667)*sign(vnormal);
            }
        }

        if(!diveMode&&(vnormal > (lastVn + decCoef*conf()->BangBang_DecMax()* 0.0166667)))
        {
            vnormal = lastVn + (decCoef*conf()->BangBang_DecMax()* 0.0166667);
        }
    }


    if(vforward - lastVf < - 1)
    {
        vforward = lastVf - 0.085;
    }


    lastVf = vforward;
    lastVn = vnormal;
}

void CAgent::generateRobotCommand()
{

    //accelerationLimiter();
    double veltan= (vel().x)*cos(dir().th().radian()) + (vel().y)*sin(dir().th().radian());
    double velnorm= -1*(vel().x)*sin(dir().th().radian()) + (vel().y)*cos(dir().th().radian());
    //    debug(QString("vf: %1 , Vn :%2").arg(vforward).arg(vnormal),D_MHMMD);

    calibrated++;
    for( int i = 0; i < _PACKET_SIZE; i++)
        outputBuffer[i] = 0x00;
    if ( isnan(vangular)){
        draw(QString("Vel : %1").arg(vangular),Vector2D(0,0),"brown",18);
        vangular = 0.0;
    }
    outputBuffer[0] = 0x99;
    outputBuffer[1] = commandID & 0x0F;
    int kickNumber = round(kickSpeed);//*160.0;
    if (roller != 0)
        outputBuffer[1] = outputBuffer[1] | ((roller & 0x07) << 4);
    outputBuffer[2] = kickNumber & 0x7F;
    outputBuffer[3] = (kickNumber >> 7) & 0x07;

    double ang=-dir().th().radian();

    //setting BU for kalman
    self()->kalman_velocs.vx = (vforward*cos(ang)) + (vnormal*sin(ang));

    self()->kalman_velocs.vy = -(vforward*sin(ang)) + (vnormal*cos(ang));
    self()->kalman_velocs.vw = vangular;

    int vforwardI = floor(vforward * 487.0);
    int vnormalI  = floor(vnormal * 487.0);
    int vangularI  = ((double)vangular) * ((double)256.0 / (double) 360.0);

    if (vforwardI > 2047) vforwardI = 2047;
    if (vforwardI < -2047) vforwardI = -2047;

    if (vnormalI > 2047) vnormalI = 2047;
    if (vnormalI < -2047) vnormalI = -2047;

    if (vangularI > 2047) vangularI = 2047;
    if (vangularI < -2047) vangularI = -2047;

    int vangularAbs = abs(vangularI);
    int vforwardAbs = abs(vforwardI);
    int vnormalAbs  = abs(vnormalI);

    outputBuffer[3] = (((vangularAbs >> 7) & 0x0F) << 3) | outputBuffer[3];

    outputBuffer[4] = vforwardAbs & 0x7F;
    outputBuffer[5] = vnormalAbs & 0x7F;
    outputBuffer[6] = (vforwardAbs >> 7) & 0x0F;
    if (vforwardI < 0) outputBuffer[6] = outputBuffer[6] | 0x10;
    if (vnormalI < 0) outputBuffer[6] = outputBuffer[6] | 0x20;

    outputBuffer[7] = vangularAbs & 0x7F;

    requestBit = true; // change to release mode command
    if (requestBit && knowledge->getGameState() != CKnowledge::Halt)
    {
        outputBuffer[8] |= 0x01;
    }

    if (chip)
        outputBuffer[8] = outputBuffer[8] | 0x02;
    if (vangularI < 0)
        outputBuffer[8] = outputBuffer[8] | 0x04;
    outputBuffer[8] = outputBuffer[8] | (((vnormalAbs >> 7) & 0x0F) << 3);
    //    outputBuffer[9] =  (int)(_ACC*5) & (0x7F);
    //    outputBuffer[10] = (int)(_DEC*5) & (0x7F);



    unsigned int velTanSend = (int)(fabs(veltan)*100);

    unsigned int velNormSend = (int)(fabs(velnorm)*100);
    outputBuffer[11] = packetNum & 0x07;
    outputBuffer[12] = ((velTanSend >> 6 ) & 0X07) | ((velNormSend & 0x0F) << 3);
    outputBuffer[13] = (velNormSend >> 4) & 0x1F;

    if(veltan < 0)
        outputBuffer[11] |= 0x40;
    else
        outputBuffer[11] &= 0xBF;

    if( velnorm < 0)
        outputBuffer[13] |= 0x40;
    else
        outputBuffer[13] &= 0xBF;
    if(packetNum < 8 )
    {
        packetNum ++;
    }
    else
    {
        packetNum = 0;
    }
}

char* CAgent::getOutputBuffer()
{
    generateRobotCommand();
    return outputBuffer;
}

void CAgent::setOnOffState(bool state){
    onOffState = state;
}

void CAgent::setCommandID  (int ID){
    commandID = ID;
}

Vector2D CAgent::pos()
{
    return wm->our[selfID]->pos;
}

Vector2D CAgent::vel()
{
    return wm->our[selfID]->vel;
}

Vector2D CAgent::dir()
{
    return wm->our[selfID]->dir;
}

bool CAgent::shootSensor()
{
    return 0 ; //wm->our[selfID]->shootSensor;
}

void CAgent::setShootSensor(bool b)
{
    //wm->our[selfID]->shootSensor = b;
}

double CAgent::angularVel()
{
    return wm->our[selfID]->angularVel;
}

double CAgent::dirDegree()
{
    return wm->our[selfID]->dir.th().degree();
}

CRobot* CAgent::self()
{
    return wm->our[selfID];
}

Vector2D CAgent::distToBall()
{
    return wm->ball->pos - wm->our[selfID]->pos;
}

void CAgent::setRobotAbsVel(double _vx, double _vy, double _w)
{
    double ang = -dir().th().radian();
    setRobotVel((cos(ang) * _vx) - (sin(ang) * _vy), (sin(ang) * _vx) + (cos(ang) * _vy), _w);
}

void CAgent::setRobotVel(double _vtan , double _vnorm , double _w )
{


    vforward = _vtan ;
    vnormal  = _vnorm ;
    vangular = _w *_RAD2DEG ;

    double _v1,_v2,_v3,_v4;
    jacobian( _vtan , _vnorm , _w , _v1 , _v2 , _v3 , _v4);
    v1 = _v1;
    v2 = _v2;
    v3 = _v3;
    v4 = _v4;
    //	if( commandID == 2 )
    //		knowledge->plotWidgetCustom[2] = v3/40.0;
}

void CAgent::addRobotVel(double _vtan, double _vnorm, double _w)
{

    double _v1,_v2,_v3,_v4;
    jacobian( _vtan , _vnorm , _w , _v1 , _v2 , _v3 , _v4);

    v1 += _v1;
    v2 += _v2;
    v3 += _v3;
    v4 += _v4;

    double vx , vy , w;
    jacobianInverse(v1 , v2 , v3 , v4 , vx , vy , w);

    vforward = vx;
    vnormal  = vy;
    vangular = w*_RAD2DEG;
}

void CAgent::waitHere()
{
    setRobotVel(0.0, 0.0, 0.0);
    //	accelerationLimiter();
    setKick(0);
    setRoller(0);
    setBeep(false);
    setForceKick(false);
    idle = true;
    skill = NULL;
    _ACC = 0;
    _DEC = 0;
}

void CAgent::setForceKick(bool force)
{
    forceKick = force;
}

void CAgent::setKick(double _kickSpeed)
{
    kickSpeed = _kickSpeed;
    chip = false;
}

void CAgent::setChip(double _kickSpeed)
{
    kickSpeed = _kickSpeed;
    chip = true;
}


void CAgent::setBeep(bool _beep)
{
    beep = _beep;
}

void CAgent::setRoller(int state)
{
    roller = state;
}

char CAgent::getRoller()
{
    return roller;
}

float CAgent::getMotorMaxRadPerSec()
{

    return 1000*2*M_PI/60.0f;
}

float CAgent::getvLimit()
{
    return (1.0f / (float)_BIT_RESOLUTION) * getMotorMaxRadPerSec() * self()->wheelRadius();
}

float CAgent::getwLimit()
{
    return ((1.0f / (float)_BIT_RESOLUTION) * getMotorMaxRadPerSec() * self()->wheelRadius()) / self()->robotRadius();
}

void CAgent::jacobian(double vx, double vy, double w, double &v1, double &v2, double &v3, double &v4)
{
    float robotRadius = 0.0795;
    float wheelRadius = 0.0275;

    // Calculate Motor Speeds
    double motorAlpha[4] = {60.0 * _DEG2RAD, 135.0 * _DEG2RAD, 225.0 * _DEG2RAD, 300 * _DEG2RAD};

    double dw1 =  (1.0 / wheelRadius) * (( (robotRadius * w) - (vx * sin(motorAlpha[0])) + (vy * cos(motorAlpha[0]))) );
    double dw2 =  (1.0 / wheelRadius) * (( (robotRadius * w) - (vx * sin(motorAlpha[1])) + (vy * cos(motorAlpha[1]))) );
    double dw3 =  (1.0 / wheelRadius) * (( (robotRadius * w) - (vx * sin(motorAlpha[2])) + (vy * cos(motorAlpha[2]))) );
    double dw4 =  (1.0 / wheelRadius) * (( (robotRadius * w) - (vx * sin(motorAlpha[3])) + (vy * cos(motorAlpha[3]))) );

    float motorMaxRadPerSec = getMotorMaxRadPerSec();

    dw1 = (dw1 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
    dw2 = (dw2 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
    dw3 = (dw3 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;
    dw4 = (dw4 / (motorMaxRadPerSec)) * _BIT_RESOLUTION;

    v1 = (char) (round(dw1));
    v2 = (char) (round(dw2));
    v3 = (char) (round(dw3));
    v4 = (char) (round(dw4));
}

void CAgent::jacobianInverse(double v1, double v2, double v3, double v4, double &vx, double &vy, double &w)
{
    float motorMaxRadPerSec = getMotorMaxRadPerSec();
    double dw1 = ((double)v1)*motorMaxRadPerSec*self()->wheelRadius()/(double)_BIT_RESOLUTION;
    double dw2 = ((double)v2)*motorMaxRadPerSec*self()->wheelRadius()/(double)_BIT_RESOLUTION;
    double dw3 = ((double)v3)*motorMaxRadPerSec*self()->wheelRadius()/(double)_BIT_RESOLUTION;
    double dw4 = ((double)v4)*motorMaxRadPerSec*self()->wheelRadius()/(double)_BIT_RESOLUTION;
    vx = (-0.3464 * dw1) + (-0.2828 * dw2) + ( 0.2828 * dw3) + ( 0.3464 * dw4);
    vy = ( 0.4142 * dw1) + (-0.4142 * dw2) + (-0.4142 * dw3) + ( 0.4142 * dw4);
    w = ( 0.2929 * dw1) + ( 0.2071 * dw2) + ( 0.2071 * dw3) + ( 0.2929 * dw4);
}

#define _99 0.96f
double F( double d, double bnd)
{
    d=fabs(d);
    double temp = tanh(d*atanh(_99)/bnd)/_99;
    return (temp);
}

CAgent::Abilities::Abilities()
{
    hasGyro = canChip = canKick = canSpin = highBattery = true;
}

double CAgent::kickValueSpeed(double value,bool spinner)//for onetouch
{
    if (wm->getIsSimulMode())
    {
        return value;
    }
    else {
        int sp = spinner?1:0;
        int v = round(value);
        return shotProfile[v][sp];
    }
}

double CAgent::kickSpeedValue(double speed,bool spinner)//for pass speed
{
    if (wm->getIsSimulMode())
    {
        return speed;
    }
    else {
        //        int sp = spinner?1:0;
        //        double mdist = 1e10;
        //        int    mi    = 0;
        //        for(int i=0;i<32;i++)
        //        {
        //            if(fabs(speed-shotProfile[i][sp]) < mdist)
        //            {
        //                mi = i;
        //                mdist = fabs(speed-shotProfile[i][sp]);
        //            }
        //        }
        //        return mi;
        double desiredSpeed = getKickValue(false, false, speed);
        //        qDebug() <<"desired= "<< desiredSpeed;
        /*
  double x = speed;
  double desiredSpeed;
  switch( self()->id ){
  case 0:
   desiredSpeed = 8.472*x*x  + 49.55*x - 203.4;
   break;
  case 1:
   desiredSpeed = 10.68*x*x + 28.84*x + 27.43;
   break;
  case 2:
   desiredSpeed = 11.47*x*x + 9.641*x + 42.55;
   break;
  case 3:
   desiredSpeed = 9.523*x*x + 33.00*x + 8.336;
   break;
  case 4:
   desiredSpeed = 8.778*x*x + 27.99*x + 20.14;
   break;
  case 5:
   desiredSpeed = 8.663*x*x + 35.57*x + 13.89;
   break;
  case 6:
   desiredSpeed = 8.732*x*x + 24.35*x + 39.74;
   break;
  case 7:
   desiredSpeed = 8.732*x*x + 24.35*x + 39.74;/////////////////
   break;
  case 8:
   desiredSpeed = 8.732*x*x + 24.35*x + 39.74;///////////////
   break;
  case 9:
   desiredSpeed = 8.732*x*x + 24.35*x + 39.74;////////////////
   break;
  default:
   desiredSpeed = 8.732*x*x + 24.35*x + 39.74; ///////////////
   break;
  }*/
        if( desiredSpeed < 0 )
            return 0;
        else if( desiredSpeed > 1023 )
            return 1023;
        return desiredSpeed;
    }
}

double CAgent::chipValueDistance(double value,bool spinner) //for chip recieve
{
    if (wm->getIsSimulMode())
    {
        return sqrt(value * value / Gravity);
    }
    else {
        int v = round(value);
        int sp = spinner?1:0;
        return chipProfile[v][sp];
    }
}

double CAgent::getKickValue(bool chip, bool spin, double v)
{
    double y, x, x2, x3, x4, x5, x6;
    int myId = selfID;



    if (chip)
    {
        x = v*100;
    }
    else {
        x = v;
    }
    x2 = x*x;
    x3 = x2*x;
    x4 = x2*x2;
    x5 = x2*x3;
    x6 = x3*x3;

    switch (selfID)
    {
    case 3:
        if (chip && spin)
            return 0.001*x2 + 1.8639*x + 58.865;
        if (chip)
            return 3E-05*x2 + 2.8236*x + 13.833;
        return 0.0386*x4 -1.262*x3 + 20.143*x2 - 6.1246*x + 37.614;
    case 4:
        if (chip && spin)
            return 0.0015*x2 + 2.9201*x - 45.104;
        if (chip)
            return y = 0.0009*x2 + 2.2283*x + 16.247;
        return -0.7326*x4 + 14.181*x3 - 83.857*x2 + 272.69*x - 205.66;
    case 5:
        if (chip && spin)
            return -0.0009*x2 + 3.422*x + 38.807;
        if (chip)
            return 0.0015*x2 + 2.8468*x - 74.866;
        return 0.0474*x6 - 1.4049*x5 + 15.668*x4 - 80.694*x3 + 198.6*x2 - 155.04*x + 75.944;
    case 2:
        if (chip && spin)
            return 0.0004*x2 + 2.0255*x + 4.1774;
        if (chip)
            return 0.0032*x2 + 1.1196*x + 50.543;
        return 0.0198*x6 - 0.5191*x5 + 4.9436*x4 - 20.368*x3 + 39.014*x2 + 27.566*x + 21.604;
    case 9:
        if (chip && spin)
            return -0.0032*x2 + 3.8923*x + 6.5453;
        if (chip)
            return 0.0027*x2 + 2.1671*x + 54.724;
        return -9E-14*x6 + 2E-10*x5 - 1E-07*x4 + 4E-05*x3 - 0.007*x2 + 1.9532*x - 1.6872;
    case 7:
        if (chip)
        {
            selfID = 5;
            y=getKickValue(true, 5, v);
            selfID = 7;
            return y;
        }
        return -0.0171*x6 + 0.5062*x5 - 5.7639*x4 + 32.104*x3 - 84.666*x2 + 159.48*x - 39.783;
    case 1:
        if (chip && spin)
            return 0.0036*x2 + 0.8745*x + 88.982;
        if (chip)
            return 0.0031*x2 + 2.2316*x - 0.4071;
        return 0.3568*x4 - 6.2606*x3 + 36.989*x2 + 29.483*x + 19.816;
    case 6:
        if (chip && spin)
            return -0.001*x2 + 3.8301*x - 9.4641;
        if (chip)
            return -0.001*x2 + 3.8301*x - 9.4641;
        return -0.0321*x6 + 0.9598*x5 - 11.038*x4 + 61.438*x3 - 163.39*x2 + 254.1*x - 71.466;
    default:
        myId = selfID;
        selfID = 1;
        y = getKickValue(chip, spin, v);
        selfID = myId;
        return y;
    }
}



double CAgent::chipDistanceValue(double distance,bool spinner) //auto chip
{
    if (wm->getIsSimulMode())
    {
        return sqrt(distance * Gravity);
    }
    else {

        //        int sp = spinner?1:0;
        //        double mdist = 1e10;
        //        int    mi    = 0;
        //        for(int i=0;i<32;i++)
        //        {
        //            if(fabs(distance-chipProfile[i][sp]) < mdist)
        //            {
        //                mi = i;
        //                mdist = fabs(distance-chipProfile[i][sp]);
        //            }
        //        }

        //		draw(QString("CHIPPING = %1").arg(mi), Vector2D(0,0), "red", 31);

        //        return mi;
        double desiredSpeed;// = getKickValue(true, spinner, distance);
        //		qDebug() << "dist : " << distance << " Speed : " << desiredSpeed;
        //
        double x = distance;
        switch( self()->id ){
        case 0:
            if( spinner )
                desiredSpeed = -0.0005*x*x  + 3.0458*x - 17.439;
            else
                desiredSpeed = -0.0005*x*x  + 3.0458*x - 17.439;
            break;
        case 1:
            if( spinner )
                desiredSpeed = 0.0015*x*x + 3.523*x + 53.18;
            else
                desiredSpeed = 0.0015*x*x + 3.523*x + 53.18;
            break;
        case 2:
            if( spinner )
                desiredSpeed = 0.000*x*x + 2.008*x + 23.98;
            else
                desiredSpeed = 0.002*x*x + 1.675*x + 34.61;
            break;
        case 3:
            if( spinner )
                desiredSpeed = 0.0005*x*x + 2.8648*x + 68.679;
            else
                desiredSpeed = 0.0005*x*x + 2.8648*x + 68.679;
            break;
        case 4:
            if( spinner )
                desiredSpeed = -0.005*x*x + 5.137*x + 0.188;
            else
                desiredSpeed = -0.000*x*x + 3.765*x + 3.212;
            break;
        case 5:
            if( spinner )
                desiredSpeed = 0.007*x*x + 2.639*x + 19.77;
            else
                desiredSpeed = 0.005*x*x + 2.982*x + 20.34;
            break;
        case 6:
            if( spinner )
                desiredSpeed = -0.001*x*x + 2.944*x + 3.219;
            else
                desiredSpeed = 0.002*x*x + 1.27*x + 60.59;
            break;
        case 7:
            if( spinner )
                desiredSpeed = -0.002*x*x + 4.068*x - 3.794;
            else
                desiredSpeed = 0.000*x*x + 2.835*x + 81.43;
            break;
        case 8:
            if( spinner )
                desiredSpeed = -0.002*x*x + 4.068*x - 3.794; /////////////
            else
                desiredSpeed = 0.000*x*x + 2.835*x + 81.43; //////////////
            break;
        case 9:
            if( spinner )
                desiredSpeed = -0.002*x*x + 4.068*x - 3.794; /////////////
            else
                desiredSpeed = 0.000*x*x + 2.835*x + 81.43; ///////////////
            break;
        default:
            desiredSpeed = -0.002*x*x + 4.068*x - 3.794; ///////////////
            break;
        }
        if( desiredSpeed < 0 )
            return 0;
        else if( desiredSpeed > 1023 )
            return 1023;
        return desiredSpeed;
    }
}

int CAgent::kickValueForDistance(double dist, double finalVel)
{
    double a = 0.66;//BallFriction()*Gravity;
    double temp,vel;
    temp = finalVel*finalVel + 2*a*dist;
    if ( temp < 0)
        temp = 0;
    vel = sqrt( temp);
    //	qDebug() << "vel=" << vel;
    return kickSpeedValue( vel, false);
}


Vector2D CAgent::oneTouchCheck(Vector2D positioningPos, Vector2D* oneTouchDirection)
{
    Vector2D oneTouchDir = Vector2D::unitVector(CSkillKickOneTouch::oneTouchAngle(pos(), Vector2D(0, 0), (pos() - wm->ball->pos).norm(),
                                                                                  pos() - wm->ball->pos, wm->field->oppGoal(), conf()->SkillsParams_KickOneTouch_Landa(), conf()->SkillsParams_KickOneTouch_Gamma()));
    Vector2D q;
    q.invalidate();
    bool oneTouchKick = false;
    if (wm->ball->vel.length() > 0.1 && (wm->ball->pos - pos()).length() < 1.0) {
        oneTouchKick = true;
    }
    if (self()->ballComingSpeed() > 0.1)
    {
        Line2D l(wm->ball->pos, wm->ball->pos + wm->ball->vel.norm());
        q = l.projection(positioningPos);
        debug("case",D_ERROR);
        if (q.valid() && (q-positioningPos).length() < 1.0 )
        {
            debug("case2",D_ERROR);
            if ((wm->ball->pos - pos()).length() < 1.0) oneTouchKick = true;
            q -= (self()->centerFromKicker() + CBall::radius) * oneTouchDir;
        }
    }
    if (oneTouchKick)
    {
        if (fabs(Vector2D::angleBetween(self()->dir, wm->field->oppGoal() - self()->pos).degree()) < 45)
        {
            debug("case3",D_ERROR);
            setKick(chipDistanceValue(8 , false));
        }
        debug("case4",D_ERROR);
    }
    *oneTouchDirection = oneTouchDir;
    return q;
}

void CAgent::getPathPlannerResult(int agentID , vector<Vector2D> _result , Vector2D _averageDir){
    if( agentID != id() )
        return;
    pathPlannerResult.assign(_result.begin() , _result.end());
    plannerAverageDir = _averageDir.norm();
}
//#include <QCoreApplication>
void CAgent::initPlanner( const int &_id , const Vector2D &_target , const QList<int>& _ourRelaxList , const QList<int> &_oppRelaxList , const bool &_avoidPenaltyArea , const bool &_avoidCenterCircle , const double &_ballObstacleRadius){
    //  QTime timer;
    //  timer.start();
    emit initPathPlanning(_id,  _target , _ourRelaxList , _oppRelaxList ,  _avoidPenaltyArea, _avoidCenterCircle, _ballObstacleRadius);
    //  qApp->processEvents();
    //  debug(QString("%1) InitPlanner Time1: %2").arg(knowledge->frameCount).arg(timer.elapsed()) , D_MASOOD);
}

//IMPORTANT
////CKS

bool CAgent::canOneTouch()
{
    draw(QString("%1 , %2").arg(self()->ballComingSpeed()).arg(fabs(Vector2D::angleBetween(wm->ball->vel.norm(),(pos() - wm->ball->pos).norm()).degree())),Vector2D(0,1));
    draw(Segment2D(Vector2D(0,0) ,(pos() - wm->ball->pos).norm()),"blue");
    draw(Segment2D(Vector2D(0,0) , wm->ball->vel.norm()),"red");
    if( self()->ballComingSpeed() > 0.9)
    {
        if( fabs(Vector2D::angleBetween(wm->ball->vel.norm(),(pos() - wm->ball->pos).norm()).degree()) < 21)
            return true;
    }
    return false;
}

void CAgent::setGyroZero()
{
    //	if ( calibrated < 999)
    //		return;
    calibrated = 0;
    //    if( id() == 4)
    agentAngelForGyro = self()->dir;
    calibrateGyro = true;
    //	debug(QString("Calibrated ! ang : %1").arg(agentAngelForGyro.dir().degree()),D_SEPEHR);
}
