#include "curve.h"
#include <math.h>

#include "geom.h"
#include <QList>
const int CCurve::curveIntegralSteps = 10;  
const int CCurve::newtonSolveSteps   = 10;  

CCurve::CCurve()
{
    last_t = 0;
}

void CCurve::setTFinal(double _tf)
{
    tf = _tf;
}

double CCurve::tFinal()
{
    return tf;
}

double CCurve::curveIntegral(double /*t*/)
{
    return 0;
}

double CCurve::curveLength(double t0,double t1)
{
    if (t0==t1) return 0;
    if (t0>t1) return -curveLength(t1,t0);
    double x0,y0;
    double s=0;    
    for (int i=0;i<curveIntegralSteps;i++)
    {
        double t = ((t1-t0)*i/(double)curveIntegralSteps) + t0;
        if (i!=0) s += hypot(X(t)-x0,Y(t)-y0);
        x0 = X(t);
        y0 = Y(t);
    }
    return s;
}

double CCurve::newtonSolveX(double x,double t0)
{
    double t=t0;
    for (int i=0;i<newtonSolveSteps;i++)
    {
        double dx = derivX(t);
        if (dx==0) return t;
        t = t - (X(t)-x)/dx;
    }
    return t;
}

double CCurve::newtonSolveY(double y,double t0)
{
    double t=t0;
    for (int i=0;i<newtonSolveSteps;i++)
    {
        double dy = derivY(t);
        if (dy==0) return t;
        t = t - (Y(t)-y)/dy;
    }
    return t;
}

double CCurve::invXY(double x,double y,double t0)
{
    double t=t0;
    for (int i=0;i<newtonSolveSteps;i++)
    {
        double xt = X(t);
        double yt = Y(t);
        double dx = derivX(t);
        double dy = derivY(t);
        double d2x= deriv2X(t);
        double d2y= deriv2Y(t);
        double f  = (xt - x)*dx + (yt - y)*dy;
        double df = dx*dx + xt*d2x - x*d2x + dy*dy + yt*d2y - y*d2y;
        if (df==0) return t;
        t = t - f/df;
    }
    return t;
}

double CCurve::derivX(double t)
{
    return (X(t+0.001) - X(t)) * 1000.0;
}

double CCurve::derivY(double t)
{
    return (Y(t+0.001) - Y(t)) * 1000.0;
}

double CCurve::deriv2X(double t)
{
    return (derivX(t+0.001) - derivX(t)) * 1000.0;
}

double CCurve::deriv2Y(double t)
{
    return (derivY(t+0.001) - derivY(t)) * 1000.0;
}

//--------------------------------------------------------------------------
CCurveX2::CCurveX2(double _a,double _b,double _c,double _tf)
{
    a = _a;
    b = _b;
    c = _c;
    tf = _tf;
}

double CCurveX2::X(double t)
{
    return t;
}

double CCurveX2::Y(double t)
{
    return a*t*t+b*t+c;
}

double CCurveX2::derivX(double /*t*/)
{
    return 1;
}

double CCurveX2::derivY(double t)
{
    return 2*a*t+b;
}

double CCurveX2::deriv2X(double /*t*/)
{
    return 0;
}

double CCurveX2::deriv2Y(double /*t*/)
{
    return 2*a;
}

double CCurveX2::invXY(double x,double /*y*/,double /*t0*/)
{
    return x;
}


//--------------------------------------------------------------------------

CCurveX3::CCurveX3(double _a,double _b,double _c,double _d,double _tf)
{
    a = _a;b = _b;c = _c;d = _d;
    tf = _tf;
}

double CCurveX3::X(double t)
{
    return t;
}

double CCurveX3::Y(double t)
{
    return a*t*t*t+b*t*t+c*t+d;
}

double CCurveX3::derivX(double /*t*/)
{
    return 1;
}

double CCurveX3::derivY(double t)
{
    return 3*a*t*t+2*b*t+c;
}

double CCurveX3::deriv2X(double /*t*/)
{
    return 0;
}

double CCurveX3::deriv2Y(double t)
{
    return 6*a*t+2*b;
}

double CCurveX3::invXY(double x,double /*y*/,double /*t0*/)
{
    return x;
}

//--------------------------------------------------------------------------
CCurveCircle::CCurveCircle(double _a,double _tf)
{
    a = _a;
    tf = _tf;
}

double CCurveCircle::X(double t)
{
    return a*cos(t);
}

double CCurveCircle::Y(double t)
{
    return a*sin(t);
}

double CCurveCircle::derivX(double t)
{
    return -a*sin(t);
}

double CCurveCircle::derivY(double t)
{
    return a*cos(t);
}

double CCurveCircle::deriv2X(double t)
{
    return -a*cos(t);
}

double CCurveCircle::deriv2Y(double t)
{
    return -a*sin(t);
}

double CCurveCircle::curveLength(double t0,double t1)
{
    //return a*(t1-t0);
    return a*AngleDeg::normalize_angle((t1-t0)*180.0f/M_PI)*M_PI/180.0f;
}

//--------------------------------------------------------------------------

CCurveEllipse::CCurveEllipse(double _a,double _b,double _tf,double _wx,double _wy,double _th0x,double _th0y)
{
    a = _a;
    b = _b;
    wx= _wx;
    wy= _wy;
    tf = _tf;
    th0x = _th0x;
    th0y = _th0y;
}

double CCurveEllipse::X(double t)
{
    return a*cos(wx*t+th0x);
}

double CCurveEllipse::Y(double t)
{
    return b*sin(wy*t+th0y);
}

double CCurveEllipse::derivX(double t)
{
    return -a*wx*sin(wx*t+th0x);
}


double CCurveEllipse::derivY(double t)
{
    return b*wy*cos(wy*t+th0y);
}

double CCurveEllipse::deriv2X(double t)
{
    return -a*wx*wx*cos(wx*t+th0x);
}

double CCurveEllipse::deriv2Y(double t)
{
    return -b*wy*wy*sin(wy*t+th0y);
}

//--------------------------------------------------------------------------

CCurveSin::CCurveSin(double _a,double _w,double _tf)
{
    a = _a;
    w = _w;
    tf = _tf;
}

double CCurveSin::X(double t)
{
    return t;
}

double CCurveSin::Y(double t)
{
    return a*sin(w*t);
}

double CCurveSin::derivX(double /*t*/)
{
    return 1;
}

double CCurveSin::derivY(double t)
{
    return a*w*cos(w*t);
}

double CCurveSin::deriv2X(double /*t*/)
{
    return 0;
}

double CCurveSin::deriv2Y(double t)
{
    return -a*w*w*sin(w*t);
}

//--------------------------------------------------------------------------

CCurveMarpich::CCurveMarpich(double _a,double _w,double _tf)
{
    a = _a;
    w = _w;
    tf = _tf;
}

double CCurveMarpich::X(double t)
{
    return a*t*cos(w*t);
}

double CCurveMarpich::Y(double t)
{
    return a*t*sin(w*t);
}

double CCurveMarpich::derivX(double t)
{
    return a*cos(w*t) - a*t*w*sin(w*t);
}

double CCurveMarpich::derivY(double t)
{
    return a*sin(w*t) + a*t*w*cos(w*t);
}

double CCurveMarpich::deriv2X(double t)
{
    return -a*w*sin(w*t) - (a*w*sin(w*t) + a*w*w*t*cos(w*t));
}

double CCurveMarpich::deriv2Y(double t)
{
    return a*w*cos(w*t) + (a*w*cos(w*t) - a*w*w*t*sin(w*t));
}

//--------------------------------------------------------------------------

CCurveSpline::CCurveSpline()
{
    xx = NULL;n=0;
}

void CCurveSpline::init(bool cubic_clamped_spline, double deriv_x0, double deriv_y0, double deriv_xf, double deriv_yf)
{
    if (xx!=NULL)
    {
        delete []xx;delete []yy;delete []fx;delete []fy;delete []bx;delete []by;delete []cx;delete []cy;delete []dx;delete []dy;
    }
    n = knots.size();
    tf = n - 1;
    xx = new double [n];
    yy = new double [n];
    fx = new double [n];
    fy = new double [n];
    bx = new double [n];
    by = new double [n];
    cx = new double [n];
    cy = new double [n];
    dx = new double [n];
    dy = new double [n];
    for (int i=0;i<n;i++)
    {
        xx[i] = yy[i] = i;
        fx[i] = knots[i].x;
        fy[i] = knots[i].y;        
    }
    if (!cubic_clamped_spline)
    {
        cubic_nak(n,xx,fx,bx,cx,dx);
        cubic_nak(n,yy,fy,by,cy,dy);
    }
    else {
        cubic_clamped(n,xx,fx,bx,cx,dx,deriv_x0,deriv_xf);
        cubic_clamped(n,yy,fy,by,cy,dy,deriv_y0,deriv_yf);
    }
 }

double CCurveSpline::X(double t)
{
    if (n==0) return 0.0;
    return spline_eval(n,xx,fx,bx,cx,dx,t);
}

double CCurveSpline::Y(double t)
{
    if (n==0) return 0.0;
    return spline_eval(n,yy,fy,by,cy,dy,t);
}

CCurveHeart::CCurveHeart(double _xs, double _ys, double _a, double _w)
{
    xs = _xs;
    ys = _ys;
    a = _a;
    w = _w;
}


double CCurveHeart::X(double t)
{
    double c = cos(w*t);
    return (1.0+a*c)*c*xs;
}

double CCurveHeart::Y(double t)
{
    double c = cos(w*t);
    return (1.0+a*c)*sin(w*t)*ys;
}



void tridiagonal ( int n, double *c, double *a, double *b, double *r )

{
     int i;

     for ( i = 0; i < n-1; i++ ) {
         b[i] /= a[i];
         a[i+1] -= c[i]*b[i];
     }

     r[0] /= a[0];
     for ( i = 1; i < n; i++ )
         r[i] = ( r[i] - c[i-1] * r[i-1] ) / a[i];

     for ( i = n-2; i >= 0; i-- )
         r[i] -= r[i+1] * b[i];
}

void cubic_nak ( int n, double *x, double *f, double *b, double *c, double *d )

/*
     PURPOSE:
          determine the coefficients for the 'not-a-knot'
          cubic spline for a given set of data


     CALLING SEQUENCE:
          cubic_nak ( n, x, f, b, c, d );


     INPUTS:
          n		number of interpolating points
          x		array containing interpolating points
          f		array containing function values to
                        be interpolated;  f[i] is the function
                        value corresponding to x[i]
          b		array of size at least n; contents will
                        be overwritten
          c		array of size at least n; contents will
                        be overwritten
          d		array of size at least n; contents will
                        be overwritten


     OUTPUTS:
          b		coefficients of linear terms in cubic
                        spline
          c		coefficients of quadratic terms in
                        cubic spline
          d		coefficients of cubic terms in cubic
                        spline

     REMARK:
          remember that the constant terms in the cubic spline
          are given by the function values being interpolated;
          i.e., the contents of the f array are the constant
          terms

          to evaluate the cubic spline, use the routine
          'spline_eval'
*/

{
     double *h,
            *dl,
            *dd,
            *du;
     int i;

     h  = new double [n];
     dl = new double [n];
     dd = new double [n];
     du = new double [n];

     for ( i = 0; i < n-1; i++ )
         h[i] = x[i+1] - x[i];
     for ( i = 0; i < n-3; i++ )
         dl[i] = du[i] = h[i+1];

     for ( i = 0; i < n-2; i++ ) {
         dd[i] = 2.0 * ( h[i] + h[i+1] );
         c[i]  = ( 3.0 / h[i+1] ) * ( f[i+2] - f[i+1] ) -
                 ( 3.0 / h[i] ) * ( f[i+1] - f[i] );
     }
     dd[0] += ( h[0] + h[0]*h[0] / h[1] );
     dd[n-3] += ( h[n-2] + h[n-2]*h[n-2] / h[n-3] );
     du[0] -= ( h[0]*h[0] / h[1] );
     dl[n-4] -= ( h[n-2]*h[n-2] / h[n-3] );

     tridiagonal ( n-2, dl, dd, du, c );

     for ( i = n-3; i >= 0; i-- )
         c[i+1] = c[i];
     c[0] = ( 1.0 + h[0] / h[1] ) * c[1] - h[0] / h[1] * c[2];
     c[n-1] = ( 1.0 + h[n-2] / h[n-3] ) * c[n-2] - h[n-2] / h[n-3] * c[n-3];
     for ( i = 0; i < n-1; i++ ) {
         d[i] = ( c[i+1] - c[i] ) / ( 3.0 * h[i] );
         b[i] = ( f[i+1] - f[i] ) / h[i] - h[i] * ( c[i+1] + 2.0*c[i] ) / 3.0;
     }

     delete [] h;
     delete [] du;
     delete [] dd;
     delete [] dl;
}


void cubic_clamped ( int n, double *x, double *f, double *b, double *c,
                     double *d, double fpa, double fpb )

/*
     PURPOSE:
          determine the coefficients for the clamped
          cubic spline for a given set of data


     CALLING SEQUENCE:
          cubic_clamped ( n, x, f, b, c, d, fpa, fpb );


     INPUTS:
          n		number of interpolating points
          x		array containing interpolating points
          f		array containing function values to
                        be interpolated;  f[i] is the function
                        value corresponding to x[i]
          b		array of size at least n; contents will
                        be overwritten
          c		array of size at least n; contents will
                        be overwritten
          d		array of size at least n; contents will
                        be overwritten
          fpa		derivative of function at x=a
          fpb		derivative of function at x=b


     OUTPUTS:
          b		coefficients of linear terms in cubic
                        spline
          c		coefficients of quadratic terms in
                        cubic spline
          d		coefficients of cubic terms in cubic
                        spline

     REMARK:
          remember that the constant terms in the cubic spline
          are given by the function values being interpolated;
          i.e., the contents of the f array are the constant
          terms

          to evaluate the cubic spline, use the routine
          'spline_eval'
*/

{
     double *h,
            *dl,
            *dd,
            *du;
     int i;

     h  = new double [n];
     dl = new double [n];
     dd = new double [n];
     du = new double [n];

     for ( i = 0; i < n-1; i++ ) {
         h[i] = x[i+1] - x[i];
         dl[i] = du[i] = h[i];
     }

     dd[0] = 2.0 * h[0];
     dd[n-1] = 2.0 * h[n-2];
     c[0] = ( 3.0 / h[0] ) * ( f[1] - f[0] ) - 3.0 * fpa;
     c[n-1] = 3.0 * fpb - ( 3.0 / h[n-2] ) * ( f[n-1] - f[n-2] );
     for ( i = 0; i < n-2; i++ ) {
         dd[i+1] = 2.0 * ( h[i] + h[i+1] );
         c[i+1] = ( 3.0 / h[i+1] ) * ( f[i+2] - f[i+1] ) -
                  ( 3.0 / h[i] ) * ( f[i+1] - f[i] );
     }

     tridiagonal ( n, dl, dd, du, c );

     for ( i = 0; i < n-1; i++ ) {
         d[i] = ( c[i+1] - c[i] ) / ( 3.0 * h[i] );
         b[i] = ( f[i+1] - f[i] ) / h[i] - h[i] * ( c[i+1] + 2.0*c[i] ) / 3.0;
     }

     delete [] h;
     delete [] du;
     delete [] dd;
     delete [] dl;
}

double spline_eval ( int n, double *x, double *f, double *b, double *c,double *d, double t )

/*
     PURPOSE:
          evaluate a cubic spline at a single value of
          the independent variable given the coefficients of
          the cubic spline interpolant (obtained from
          'cubic_nak' or 'cubic_clamped')


     CALLING SEQUENCE:
          y = spline_eval ( n, x, f, b, c, d, t );
          spline_eval ( n, x, f, b, c, d, t );


     INPUTS:
          n		number of interpolating points
          x		array containing interpolating points
          f		array containing the constant terms from
                        the cubic spline (obtained from 'cubic_nak'
                        or 'cubic_clamped')
          b		array containing the coefficients of the
                        linear terms from the cubic spline
                        (obtained from 'cubic_nak' or 'cubic_clamped')
          c		array containing the coefficients of the
                        quadratic terms from the cubic spline
                        (obtained from 'cubic_nak' or 'cubic_clamped')
          d		array containing the coefficients of the
                        cubic terms from the cubic spline
                        (obtained from 'cubic_nak' or 'cubic_clamped')
          t		value of independent variable at which
                        the interpolating polynomial is to be
                        evaluated


     OUTPUTS:
          y		value of cubic spline at the specified
                        value of the independent variable
*/

{
     int i,
         found;

     i = 1;
     found = 0;
     while ( !found && ( i < n-1 ) ) {
           if ( t < x[i] )
              found = 1;
           else
              i++;
     }
     t = f[i-1] + ( t - x[i-1] ) * ( b[i-1] + ( t - x[i-1] ) * ( c[i-1] +
                  ( t - x[i-1] ) * d[i-1] ) );
     return ( t );
}

double spline_eval_deriv ( int n, double *x, double */*f*/, double *b, double *c,double *d, double t )
{
     int i,found;
     i = 1;
     found = 0;
     while ( !found && ( i < n-1 ) ) {
           if ( t < x[i] )
              found = 1;
           else
              i++;
     }
     t = b[i-1]+2*(t-x[i-1])*c[i-1] + 3*(t-x[i-1])*(t-x[i-1])*d[i-1];
     return t;
}

double spline_eval_deriv2 ( int n, double *x, double */*f*/, double */*b*/, double *c,double *d, double t )
{
     int i,found;
     i = 1;
     found = 0;
     while ( !found && ( i < n-1 ) ) {
           if ( t < x[i] )
              found = 1;
           else
              i++;
     }
     t = 2*c[i-1] + 6*(t-x[i-1])*d[i-1];
     return t;
}

