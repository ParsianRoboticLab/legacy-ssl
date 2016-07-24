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
__kernel void domdiag(__write_only image2d_t dstImage,
                        __global __read_only float *robotx,
                        __global __read_only float *roboty,
                        __global __read_only float *robotvelx,
                        __global __read_only float *robotvely,
                         float amaxx,
                         float amaxy,
                         int n)
{
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float x = get_global_id(0);
    float y = get_global_id(1);
    float4 pixel = (float4)(0, 0, 0, 0);
    int i=0;
    float rx, ry, rvx, rvy, tx, ty;
    int k = 0;
    float tmin=0;
    float d = 0;
    for (i=0;i<n;i++)
    {       
        rx  = robotx[i];
        ry  = roboty[i];
//        d += 0.0005*len(0,0,im_width,im_height)/len(x, y, rx, ry);
        if (rx < -10000) continue;
        rvx = robotvelx[i];
        rvy = robotvely[i];
        tx  = fabs(sign(x)*sqrt(fabs(x-rx)*amaxx*2.0+rvx*rvx) - rvx) / amaxx;
        ty  = fabs(sign(x)*sqrt(fabs(y-ry)*amaxy*2.0+rvy*rvy) - rvy) / amaxy;
        tx = (tx > ty) ? tx : ty;
        if (i==0 || tx < tmin)
        {
            tmin = tx;
            k = i;
        }
    }    
    d = (float)k / (float)n;
    pixel = (float4) (d,d,d,1);
    write_imagef(dstImage, pos, clamp(pixel, 0.0f, 1.0f));
}
