#define factor 0.5
__constant const float im_width = 242*factor;
__constant const float im_height = 122*factor;
__constant const float rad = 2.0 * 0.09 * 242 * factor / 6.05;
__constant const float goalx = 242 * factor;
__constant const float goaly = 122.0 * factor/ 2.0;
__constant const float ourgoalx = 0;
__constant const float ourgoaly = 122.0 * factor/ 2.0;
__constant const float goalx1 = 242 * factor;
__constant const float goalx2 = 242 * factor;
__constant const float goaly1 = (-0.35 / 2.025 + 1.0) * 122.0 * factor / 2.0;
__constant const float goaly2 = (+0.35 / 2.025 + 1.0) * 122.0 * factor / 2.0;
__constant const int Features_Count = 10;
__constant const int HiddenLayer_Size = 25; //note: if you change this you have to take a look at forwardPropagate function
__constant const float _zero = 0;
__constant const float _one = 1;
#ifndef M_PI
#define M_PI 3.141592
#endif


struct range {
  float a,b;
};

inline float getangle(float x1,float y1,float x2,float y2)
{
    return atan2(y2-y1,x2-x1);
}

inline float len(float x1,float y1,float x2,float y2)
{
    return hypot(x1-x2, y1-y2);
}

inline float len2(float x1,float y1,float x2,float y2)
{
    return (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2);
}


inline float normalang(float dir)
{
    const float _2PI = 2.0 * M_PI;
      if ( dir < -2.0*M_PI || 2.0*M_PI < dir )
      {
          dir = fmod( dir, _2PI );
      }
      if ( dir < -M_PI)
      {
          dir += 2.0*M_PI;
      }
      if ( dir > M_PI)
      {
          dir -= 2.0*M_PI;
      }
      return dir;
}

inline float normalangabs(float dir)
{
    const float _2PI = 2.0 * M_PI;
    if ( dir < -2.0*M_PI || 2.0*M_PI < dir )
      {
          dir = fmod( dir, _2PI );
      }
      if ( dir < -M_PI)
      {
          dir += 2.0*M_PI;
      }
      if ( dir > M_PI)
      {
          dir -= 2.0*M_PI;
      }
      if (dir < 0) return -dir;
      return dir;
}
#if 0


void quicksort(struct range* a, int lo , int hi )
{
        int i = lo , j = hi;
        float x = a[(lo + hi)/2].a;
        struct range tmp;
        do
        {
                while ( normalang(a[i].a-x)<0 ) i++;
                while ( normalang(a[j].a-x)>0 ) j--;
                if ( i <= j )
                {
                    tmp = a[i];
                    a[i] = a[j];
                    a[j] = tmp;
                    i++;
                    j--;
                }
        }while ( i < j );
        if ( lo < j ) quicksort(a , lo , j);
        if ( i < hi ) quicksort(a , i , hi);
}
#endif
inline float canrecvpass(float rx, float ry, float x,float y,__global __read_only  float *obsx, __global __read_only float *obsy, int numObs)
{
    int i;
    float a,b,c,l,d,s,ox,oy;
    float rcv = 1.0;
    a = ry-y;
    b = x-rx;
    c = -x*a-y*b;
    d = hypot(a,b);
    for (i=0;i<numObs;i++)
    {
        ox = obsx[i];
        oy = obsy[i];
        l = fabs(ox*a + oy*b + c) / d;
        if (l<rad)
        {
            s = (ox-x)*(ox-rx) + (oy-y)*(oy-ry);
            if (s < 0) {
                rcv *= l/rad;
            }
        }
    }
    return rcv;
}

inline float openness(float gx1, float gy1, float gx2, float gy2, float x,float y,__global __read_only  float *obsx, __global __read_only float *obsy, int numObs, float radfactor)
{
    float d, a1, a2, a, l, a0, q1, q2, al;
    float la, lb, lc;
    float ox, oy;
    la = gy2-gy1;
    lb = gx1-gx2;
    lc = -gx1*la-gy1*lb;
    bool inobs = false;
    int par = 0;
    bool tmp;
    struct range tmpr;
    int count = 0;
    int i,j;
    float radf = rad * radfactor;
    float minl = 10000;
    d = 0;
    struct range r[20];
    bool flag[20];
    for (i = 0;i<20;i++)
        flag[i] = false;
    //al = normalang(getangle(gx1, gy1, gx2, gy2) - M_PI*0.5);
    al = getangle(x, y, (gx1+gx2)*0.5, (gy1+gy2)*0.5);
    q1 = getangle(x,y,gx1,gy1)-al;
    q2 = getangle(x,y,gx2,gy2)-al;
    q1 = normalang(q1);
    q2 = normalang(q2);
    if (normalang(q1 - q2) > 0)
    {
        a = q1;
        q1 = q2;
        q2 = a;
    }    
    for (i = 0; i < numObs; ++i) {
        ox = obsx[i];
        oy = obsy[i];
        l = len(x,y,ox,oy);
        if (l < minl) minl = l;
        if (l<radf) {inobs = true;break;}
        a1 = ox*la + oy*lb + lc;
        a2 = x*la + y*lb + lc;
        if (a1 > 0) a1 = 1;else a1 = -1;
        if (a2 > 0) a2 = 1;else a2 = -1;
        a1 = a1*a2;
        if (a1 > 0)
        {
            a = normalang(getangle(x,y,ox,oy)-al);
            a0 = asin(radf/l);
            a1 = a - a0;
            a2 = a + a0;
            if (a1 < -M_PI*0.95*0.5) a1 = -M_PI*0.95*0.5;
            if (a2 < -M_PI*0.95*0.5) a2 = -M_PI*0.95*0.5;
            if (a1 > +M_PI*0.95*0.5) a1 =  M_PI*0.95*0.5;
            if (a2 > +M_PI*0.95*0.5) a2 =  M_PI*0.95*0.5;
            if (normalang(a1-a2)>0)
            {
                a = a1;
                a1 = a2;
                a2 = a;
            }
            a1 = normalang(a1);
            a2 = normalang(a2);
            if (normalang(a1-q1)<=0) a1 = q1;
            if (normalang(a1-q2)>=0) a1 = q2;
            if (normalang(a2-q1)<=0) a2 = q1;
            if (normalang(a2-q2)>=0) a2 = q2;
            if (normalangabs(a1-a2)>=0.001)
            {
                r[count].a = a1;
                r[count].b = a2;
                count ++;
            }
        }
    }
    if (!inobs)
    {
        for (i=0;i<count;i++)
            for (j=0;j<count-1;j++)
            {
                if (normalang(r[j].a-r[j+1].a) > 0)
                {
                    tmpr = r[j];
                    r[j] = r[j+1];
                    r[j+1] = tmpr;
                }
            }
        //quicksort(r, 0, count-1);
        for (i=0;i<count-1;i++)
        {
            if (normalang(r[i+1].a - r[i].b) < 0)
            {
                r[i+1].a = r[i].a;
                if (normalang(r[i+1].b - r[i].b) < 0)
                {
                    r[i+1].b = r[i].b;
                }
                flag[i] = true;
            }
        }
        for (i=0;i<count;i++)
        {
            if (flag[i] == false)
            {
                d += normalang(r[i].b - r[i].a);
                flag[i] = true;
            }
        }
		d = (normalangabs(q2-q1) - d)*6.0f;
		//d /= normalangabs(q2-q1);
		//d = 1-d;
        //d = d*0.5 + normalang(q2-q1) * 0.5;
    }
    else d = 0.0;
	d = d*(1.0f-exp(-minl*minl/300.0f));//  tanh(minl/40.0f);
	if (d > 1) d = 1.0f;
	if (d < 0.001f) d = 0.001f;
	return d;
}

void extractFeatures(float x, float y,
                     float playmakerposx, float playmakerposy,
                     float ballx, float bally,
                     float positioner_posx, float positioner_posy,
                     __global __read_only float *obsx,
                     __global __read_only float *obsy,
                     int numObs, int oppindex, float* features,
                     const int feature_index)
{
    float maxDist = hypot(im_width,im_height);
    float probForShooting = 0;
    float probForRecieving = 0;
    float senderDistance = 0;
    float oppsDist = 0;
    float markerAngle = 0;
    float oneTouchAngle = 0;
    float passLineFactor = 0;
    float cornerness = 0;
    float distToGoal = 0;
    float distToOurGoal = 0;
    float ballToGoalBlock = 0;
    float passReceive = 0.0;
    float receiverVisibleAngle = 0;
    float asideness = 0;
    int nearestOpp = -1;
    float nearestDist = 0;
    float markerPosX, markerPosY;
	float f1 = openness(goalx1, goaly1, goalx2, goaly2, x, y, obsx, obsy, numObs, 0.3);
    float f2 = openness(goalx1, goaly1, goalx2, goaly2, x, y, obsx, obsy, numObs, 0.6);
    float f3 = openness(goalx1, goaly1, goalx2, goaly2, x, y, obsx, obsy, numObs, 1);
	probForShooting = (f1*0.1 + f2*0.3 + f3*0.6);
	//probForShooting = openness(goalx1, goaly1, goalx2, goaly2, x, y, obsx, obsy, numObs, 1);
    float px = x - playmakerposx;
    float py = y - playmakerposy;
    float pp = len(0, 0, px, py);
    float px1, py1, px2, py2; //sides of pass receiver
    float dist;
    int i;
    if (playmakerposx > -50.0) // valid
    {
/*        px1 = playmakerposx+(-py * 4.0 + 2.0*px) * rad / pp;
        py1 = playmakerposy+( px * 4.0 + 2.0*py) * rad / pp;
        px2 = playmakerposx+( py * 4.0 + 2.0*px) * rad / pp;
        py2 = playmakerposy+(-px * 4.0 + 2.0*py) * rad / pp;*/
        //passReceive = openness(px1, py1, px2, py2, x, y, obsx, obsy, numObs);
        passReceive = canrecvpass(x,y,playmakerposx,playmakerposy,obsx,obsy,numObs);
        senderDistance = pp / maxDist;
        oneTouchAngle  = normalangabs(getangle(x,y,playmakerposx,playmakerposy) - getangle(x,y,0.5*(goalx1+goalx2),goaly)) / M_PI;
        senderDistance -= 0.35;
        senderDistance *= -senderDistance/0.25;
        senderDistance  = senderDistance;
        oneTouchAngle  -= 0.3;
        oneTouchAngle  *= -oneTouchAngle/0.09;
        oneTouchAngle   = oneTouchAngle;
    }
    else{        
        senderDistance = 1.0;
        oneTouchAngle = 1.0;
        passReceive = 1.0;
    }    
    ballToGoalBlock = normalangabs(getangle(x,y,ballx,bally)-getangle(goalx,goaly,ballx,bally)) / M_PI;    
    receiverVisibleAngle = normalangabs(getangle(x,y,goalx1,goaly1)-getangle(x,y,goalx2,goaly2)) / M_PI;
    if (receiverVisibleAngle==0.0)
        ballToGoalBlock = 1;
    else
        ballToGoalBlock = (ballToGoalBlock/receiverVisibleAngle);
    dist = len(goalx,goaly,x,y)/maxDist;
    cornerness = (fabs(x-goalx)/maxDist) / dist;
    if (cornerness>1) cornerness = 1;
    if (y<im_height-y)
        asideness = y/im_height;
    else
        asideness = (im_height-y)/im_height;
    asideness = tanh(asideness*8);
    asideness *= tanh(x*1.5/im_width);
    cornerness *= asideness;
	cornerness *= 1-exp(-(y-im_height/2)*(y-im_height/2)*100.0f/(im_height*im_height));
    cornerness = cornerness*cornerness;	
	distToGoal = 1.0-exp(-dist*dist/0.1);
/*	if (distToGoal > 0.7) distToGoal = 1.0;
    else {
		if (distToGoal < 0.7)
        {
			distToGoal = distToGoal / 0.7;
        }
	}*/
    //distToGoal = tanh(distToGoal/0.3);


    distToOurGoal = len(ourgoalx,ourgoaly,x,y)/maxDist;
    if (distToOurGoal > 0.1) distToOurGoal = 1.0;
    else {
        if (distToOurGoal < 0.1)
        {
            distToOurGoal = distToOurGoal / 0.1;
        }
    }   
/*
    /////////////////////////

    float minDistToBallLine=1e5;
    float prod=0;
    float lenlen=0;
    float lineDistWithObs;
    float M_a,M_b,M_c;
    float ox,oy;


    for (i = 0; i < numObs; ++i) {

        ox = obsx[i];
        oy = obsy[i];

        //prod=(x-ballx,y-bally)*(ox-ballx,oy-bally)
        prod = (x-ballx)*(ox-ballx)+(y-bally)*(oy-bally);
        lenlen = (x-ballx)*(x-ballx)+(y-bally)*(y-bally);
        if((0<=prod) && (prod<=lenlen))
        {
            M_a = -( bally - y );
            M_b = ballx - x;
            M_c = -M_a * x - M_b * y;
            lineDistWithObs = fabs( ( M_a * ox + M_b * oy + M_c ) / sqrt( M_a * M_a + M_b * M_b ) );
        }
        if(lineDistWithObs<minDistToBallLine)
        {
            minDistToBallLine = lineDistWithObs;
        }
    }
    minDistToBallLine = minDistToBallLine / maxDist;

    if(minDistToBallLine>1.0)
    {
        minDistToBallLine = 1.0;
    }
    /////////////////////////
*/
    features[0] = log(probForShooting>0.001?probForShooting:0.001);
    features[1] = senderDistance;//
    features[2] = oneTouchAngle;//
    features[3] = log(cornerness>0.001?cornerness:0.001);
    features[4] = log(distToGoal>0.001?distToGoal:0.001);
    features[5] = log(tanh(ballToGoalBlock)>0.001?tanh(ballToGoalBlock):0.001);//varboro
	features[6] = log(passReceive>0.001?passReceive:0.001);
//    dist = len(positioner_posx, positioner_posy, x, y)/maxDist;
    features[7] = log(distToOurGoal>0.001?distToOurGoal:0.001);//-dist*dist/(.5*.5);//1.0 / (1.0 + dist * dist);
//    features[8] = log(minDistToBallLine>0.001?minDistToBallLine:0.001);
	//features[0] += features[4];
}



/*void extractFeatures(float x, float y, float playmakerposx, float playmakerposy, float ballx, bally, __global __read_only  float *obsx, __global __read_only float *obsy, int numObs, int oppindex, float* features,const int feature_index)
{
    float maxDist = hypot(im_width,im_height);
    float probForShooting = 0;
    float probForRecieving = 0;
    float senderDistance = 0;
    float senderVisibleAngle = 0;
    float receiverVisibleAngle = 0;
    float oppsDist = 0;
    float markerAngle = 0;
    float oneTouchAngle = 0;
    float senderGoalReceiverAngle = 0;
    float passLineFactor = 0;
    float cornerness = 0;
    float distToGoal = 0;
    float ballToGoalBlock = 0;
    int nearestOpp = -1;
    float nearestDist = 0;
    float markerPosX, markerPosY;
    if (feature_index==0 || feature_index==-1) probForShooting = openness(goalx1, goaly1, goalx2, goaly2, x, y, obsx, obsy, numObs);
    float px = x - playmakerposx;
    float py = y - playmakerposy;
    float pp = len(0, 0, px, py);
    float px1, py1, px2, py2; //sides of pass receiver
    float dist;
    int i;
    if (playmakerposx > -50.0) // valid
    {
        px1 = playmakerposx-py * rad * 4.0 / pp;
        py1 = playmakerposy+px * rad * 4.0 / pp;
        px2 = playmakerposx+py * rad * 4.0 / pp;
        py2 = playmakerposy-px * rad * 4.0 / pp;        
        if (feature_index==2 || feature_index==-1) senderVisibleAngle = normalangabs(getangle(x,y,px1,py1)-getangle(x,y,px2,py2)) / M_PI;
        if (feature_index==1 || feature_index==-1) senderDistance = pp / maxDist;
        if (feature_index==6 || feature_index==-1) oneTouchAngle  = normalangabs(getangle(x,y,playmakerposx,playmakerposy) - getangle(x,y,0.5*(goalx1+goalx2),goaly));
        if (feature_index==7 || feature_index==-1) senderGoalReceiverAngle = normalangabs(getangle(goalx,goaly,playmakerposx,playmakerposy) - getangle(goalx,goaly,x,y) ) / M_PI;
    }
    else{
        senderVisibleAngle = 0.8;
        senderDistance = 0.5;
        oneTouchAngle = 0.5;
        senderGoalReceiverAngle = 0.5;
    }
    if (feature_index==10 || feature_index==3 || feature_index==-1) ballToGoalBlock = normalangabs(getangle(x,y,ballx,bally)-getangle(goalx,goaly,ballx,bally)) / M_PI;
    if (feature_index==3 || feature_index==10 || feature_index==-1) receiverVisibleAngle = normalangabs(getangle(x,y,goalx1,goaly1)-getangle(x,y,goalx2,goaly2)) / M_PI;
    if (receiverVisibleAngle==0.0)
        ballToGoalBlock = 1;
    else
        ballToGoalBlock = (ballToGoalBlock/receiverVisibleAngle);
    oppsDist = 1.0;
    if (feature_index == 4 || feature_index == 5)
    {
        for (i=oppindex;i<numObs;i++)
        {
            dist = len(obsx[i], obsy[i],x, y);
            if (nearestOpp == -1)
            {
                nearestDist = dist;
                nearestOpp = i;
                markerPosX = obsx[i];
                markerPosY = obsy[i];
            }
            else {
                if (dist < nearestDist)
                {
                    dist = nearestDist;
                    nearestOpp = i;
                    markerPosX = obsx[i];
                    markerPosY = obsy[i];
                }
            }
        }
        if (nearestOpp == -1)
        {
            oppsDist = 0.5;
            markerAngle = 0.0;
        }
        else {
            markerAngle = normalangabs(getangle(x,y,markerPosX,markerPosY)-getangle(goalx,goaly,markerPosX,markerPosY)) / M_PI;
            oppsDist = nearestDist / maxDist;
        }
    }
    dist = len(goalx,goaly,x,y);
    if (feature_index == 8 || feature_index == -1)
    {
        cornerness = fabs(x-goalx) / dist;
    }
    if (cornerness>1) cornerness = 1;
    if (feature_index == 9 || feature_index == -1)
    {
        distToGoal = dist / maxDist;
    }
    features[0]  = probForShooting;
    features[1]  = senderDistance;
    features[2]  = senderVisibleAngle;
    features[3]  = receiverVisibleAngle;
    features[4]  = oppsDist;
    features[5]  = 1.0-markerAngle;
    features[6]  = oneTouchAngle;
    features[7]  = senderGoalReceiverAngle;
    features[8]  = cornerness;
    features[9]  = distToGoal;
    features[10] = ballToGoalBlock;
}
*/
float forwardPropagate(float* input, __global __read_only float* weight12, __global __read_only float* weight23, __global __read_only float* bias2, float bias3)
{
    int i,j,k;
    float a;
    float value2[25];
    k = 0;
    for (j=0;j<HiddenLayer_Size;j++)
	{
        a = 0;
        for (i=0;i<Features_Count;i++)
            a += weight12[k+i]*input[i];
        k += Features_Count;
        a += bias2[j];
        a = -a;
        a = exp(a);
        value2[j] = 1.0 / (1.0 + a);
    }
    a = 0;
    for (i=0;i<HiddenLayer_Size;i++)
        a += weight23[i]*value2[i];
    a += bias3;
    a = -a;
    a = exp(a);
    a = 1.0 / (1.0 + a );
    return a;
}

__kernel void evalpoint(__write_only image2d_t dstImage,
                        __global __read_only float *obsx,
                        __global __read_only float *obsy,
                        const float2 playmakerpos, const float2 ball, const float2 positioner_pos,__global __read_only int* cc,
                        __global __read_only float* weights)
                        //__global __read_only float* weight12, __global __read_only float* weight23, __global __read_only float* bias2, float bias3)
{
    int oppindex = cc[0];
    int numObs = cc[1];
    int feature_index = cc[2];
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float x = get_global_id(0);
    float y = get_global_id(1);
	float4 pixel = (float4)(0, 0, 0, 0);
	float d = 0.3;
	int i=0;
    float features[11]; //features_count + 1
    extractFeatures(x,y,playmakerpos.x,playmakerpos.y,ball.x,ball.y,positioner_pos.x,positioner_pos.y,obsx,obsy,numObs,oppindex,features,feature_index);
    if (feature_index == -1)
    {                
//        d = forwardPropagate(&(features[1]), weight12, weight23, bias2, bias3) * features[0];
		/*for (i=0;i<8;i++)
        {
            if (features[i] > 0) features[i] = 0;
			d += features[i];// * weights[i];
        }
		d = exp(d);*/
		d = exp(features[0]*0.5f + features[3]*1.0f + (features[4])*0.2f);
		d = tanh((d-0.1)*5.0f);
        //if (d>1) d = 1;

        //d = log((1-d)/(1+d));
		//d = tanh(d*3);
    }
	else d = features[feature_index];
    pixel = (float4) (d,d,d,1);
    write_imagef(dstImage, pos, clamp(pixel, 0.0f, 1.0f));
}

const sampler_t samp = CLK_ADDRESS_CLAMP;
//| CLK_FILTER_LINEAR;

__kernel void conv(__write_only image2d_t dstImage, __read_only image2d_t srcImage, __global __read_only float* filter, int size)
{
    /*const float filter[81] =
    {4,   3,   2,   1,   0,   -1,  -2,  -3,  -4,
    5,   4,   3,   2,   0,   -2,  -3,  -4,  -5,
    6,   5,   4,   3,   0,   -3,  -4,  -5,  -6,
    7,   6,   5,   4,   0,   -4,  -5,  -6,  -7,
    8,   7,   6,   5,   0,   -5,  -6,  -7,  -8,
    7,   6,   5,   4,   0,   -4,  -5,  -6,  -7,
    6,   5,   4,   3,   0,   -3,  -4,  -5,  -6,
    5,   4,   3,   2,   0,   -2,  -3,  -4,  -5,
    4,   3,   2,   1,   0,   -1,  -2,  -3,  -4};*/
    /*const float filter[25] = {
    2,   1,   0,   -1,  -2,
    3,   2,   0,   -2,  -3,
    4,   3,   0,   -3,  -4,
    3,   2,   0,   -2,  -3,
    2,   1,   0,   -1,  -2};*/

    int i,j,k;
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float2 srcpos = (float2)(get_global_id(0), get_global_id(1));
    float4 pixel = (float4)(0, 0, 0, 0);
    float4 p;
    float d=0;
   // pixel = read_imagef(srcImage, samp, srcpos);
    k=0;
    for (i=0;i<size;i++)
    {
        for (j=0;j<size;j++)
        {
            p = read_imagef(srcImage, samp, srcpos + (float2)(i-size/2, j-size/2));
            d += p.x*filter[k];
            k++;
        }
    }
    //pixel = (float4) (fabs(pixel.x), fabs(pixel.y), fabs(pixel.z), 1);
    if (d < 0)
        pixel = (float4) (0, -d, 0, 1);
    else
        pixel = (float4) (d, 0, 0, 1);
    write_imagef(dstImage, pos, clamp(pixel, 0.0f, 1.0f));
}


__kernel void convthresh(__write_only image2d_t dstImage, __read_only image2d_t srcImage, __global __read_only float* filter, int size, float threshold)
{
    int i,j,k;
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float2 srcpos = (float2)(get_global_id(0), get_global_id(1));
    float4 pixel = (float4)(0, 0, 0, 0);
    float4 p;
    float d=0;
    k=0;
    for (i=0;i<size;i++)
    {
        for (j=0;j<size;j++)
        {
            p = read_imagef(srcImage, samp, srcpos + (float2)(i-size/2, j-size/2));
            d += p.x*filter[k];
            k++;
        }
    }
    if (d > threshold)
        pixel = (float4) (1, 1, 1, 1);
    else
        pixel = (float4) (0, 0, 0, 1);
    write_imagef(dstImage, pos, clamp(pixel, 0.0f, 1.0f));
}


__kernel void gradascent(__write_only image2d_t dstImage1, __write_only image2d_t dstImage2, __read_only image2d_t GX, __read_only image2d_t GY, __read_only image2d_t original, int num)
{
    int i,p,q;
    float mu,dx,dy;    
    float2 pos;
    int2 point, point0;
    float4 cx = (float4)(0, 0, 0, 0);
    float4 cy = (float4)(0, 0, 0, 0);
	float4 white2;
	float4 black2;
    p = get_global_id(0);
    q = get_global_id(1);
	black2.x= (float)p/200000.0f;
	black2.y= black2.x;
	black2.z= black2.x;
	black2.w= (float)200.0f/200.0f;
	white2.x= black2.w;
	white2.y= black2.x;
	white2.z= black2.x;
	white2.w= black2.w;
	mu = 2;
    point0.x = p;
    point0.y = q;
    pos.x = p;
    pos.y = q;	
    //black = read_imagef(original, samp, pos);	
	//write_imagef(dstImage2, point0, black2);

	for (i=0;i<50;i++)
    {
        cx = read_imagef(GX, samp, pos);
        cy = read_imagef(GY, samp, pos);
        dx = cx.x - cx.y;
        dy = cy.x - cy.y;
        pos.x -= dx*mu;
        pos.y -= dy*mu;
	}
	if (num==0)
	{
		write_imagef(dstImage1, point0, clamp(black2, 0.0f, 1.0f));
	}
	else {
		write_imagef(dstImage2, point0, clamp(black2, 0.0f, 1.0f));
	}


	if (pos.x<0 || pos.x>im_width-1 || pos.y<0 || pos.y>im_height-1) return;
    point.x = pos.x;
    point.y = pos.y;
	black2 = read_imagef(original, samp, pos);
	if (black2.x > 0.3)
    {
        if (num == 0)
        {
//			white = read_imagef(dst2, samp, pos);
//            white.x += 0.02;
			write_imagef(dstImage2, point, clamp(white2, 0.0f, 1.0f));
        }
        else {
//			white = read_imagef(dst1, samp, pos);
//            white.x += 0.02;
			write_imagef(dstImage1, point, clamp(white2, 0.0f, 1.0f));
        }
	}
}
/*

#define SIZE 12

struct StackItem
{
    int StartIndex;
    int EndIndex;
};
int stackPointer = 0;
int myArray[SIZE] = {44,33,11,55,77,90,40,60,99,22,88,66};
void Push(struct StackItem item)
{
    myStack[stackPointer] = item;
    stackPointer++;
}

struct StackItem Pop()
{
    stackPointer--;
    return myStack[stackPointer];
}

int StackHasItem()
{
    if(stackPointer>0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


void Swap(int * a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

int Scan(int *startIndex, int *endIndex)
{
    int partition = 0;
    int i = 0;

    if(*startIndex > *endIndex)
    {
        for(i=*startIndex ; i>=*endIndex ; i--)
        {
            if(myArray[i]<myArray[*endIndex])
            {
                Swap(&myArray[i], &myArray[*endIndex]);
                *startIndex = *endIndex;
                *endIndex = i;
                partition = i;
                break;
            }
            if(i==*endIndex)
            {
                *startIndex = *endIndex;
                *endIndex = i;
                partition = i;
            }
        }
    }
    else if(*startIndex < *endIndex)
    {
        for(i=*startIndex ; i<=*endIndex ; i++)
        {
            if(myArray[i]>myArray[*endIndex])
            {
                Swap(&myArray[i], &myArray[*endIndex]);
                *startIndex = *endIndex;
                *endIndex = i;
                partition = i;
                break;
            }
            if(i==*endIndex)
            {
                *startIndex = *endIndex;
                *endIndex = i;
                partition = i;
            }
        }
    }

    return partition;
}

int GetFinalPosition(struct StackItem item1)
{
    struct StackItem item = {0};
    int StartIndex = item1.StartIndex ;
    int EndIndex = item1.EndIndex;
    int PivotIndex = -99;

    while(StartIndex != EndIndex)
    {
        PivotIndex = Scan(&EndIndex, &StartIndex);
    }
    return PivotIndex;
}

void QuickSort()
{
    int median = 0;
    struct StackItem item;
    struct StackItem item1={0};
    struct StackItem item2={0};

    item.StartIndex = 0;
    item.EndIndex = SIZE-1;

    Push(item);

    while(StackHasItem())
    {
        item = Pop();

        median = GetFinalPosition(item);

        if(median>=0 && median<=(SIZE-1))
        {
            if(item.StartIndex<=(median-1))
            {
                item1.StartIndex = item.StartIndex;
                item1.EndIndex = median-1;
                Push(item1);
            }
            if(median+1<=(item.EndIndex))
            {
                item2.StartIndex = median+1;
                item2.EndIndex = item.EndIndex;
                Push(item2);
            }
        }

    }
}



*/
