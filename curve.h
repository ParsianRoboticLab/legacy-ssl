#ifndef CURVE_H
#define CURVE_H
#include "geom.h"
#include <QList>

class CCurve
{
protected:
    double tf;
public:
    static const int curveIntegralSteps;
    static const int newtonSolveSteps;
    double last_t;
    CCurve();
    void setTFinal(double _tf);
    double tFinal();
    virtual double X(double t)=0;
    virtual double Y(double t)=0;
    virtual double derivX(double t);
    virtual double derivY(double t);
    virtual double deriv2X(double t);
    virtual double deriv2Y(double t);
    virtual double invXY(double x,double y,double t0=0);
    virtual double curveIntegral(double t);
    virtual double curveLength(double t0,double t1);
    double newtonSolveX(double x,double t0);
    double newtonSolveY(double y,double t0);
};
class CCurveX2 : public CCurve
{
    double a,b,c;
public:
    CCurveX2(double _a,double _b,double _c,double _tf);
    virtual double X(double t);
    virtual double Y(double t);
    virtual double derivX(double t);
    virtual double derivY(double t);
    virtual double deriv2X(double t);
    virtual double deriv2Y(double t);
    virtual double invXY(double x,double y,double t0=0);
};

class CCurveX3 : public CCurve
{
    double a,b,c,d;
public:
    CCurveX3(double _a,double _b,double _c,double _d,double _tf);
    virtual double X(double t);
    virtual double Y(double t);
    virtual double derivX(double t);
    virtual double derivY(double t);
    virtual double deriv2X(double t);
    virtual double deriv2Y(double t);
    virtual double invXY(double x,double y,double t0=0);
};

class CCurveSin : public CCurve
{
    double a,w;
public:
    CCurveSin(double _a,double _w,double _tf);
    virtual double X(double t);
    virtual double Y(double t);
    virtual double derivX(double t);
    virtual double derivY(double t);
    virtual double deriv2X(double t);
    virtual double deriv2Y(double t);
};

class CCurveMarpich : public CCurve
{
    double a,w;
public:
    CCurveMarpich(double _a,double _w,double _tf);
    virtual double X(double t);
    virtual double Y(double t);
    virtual double derivX(double t);
    virtual double derivY(double t);
    virtual double deriv2X(double t);
    virtual double deriv2Y(double t);
    //virtual double invXY(double x,double y,double t0=0);
};

class CCurveCircle : public CCurve
{
    double a;
public:
    CCurveCircle(double _a,double _tf);
    virtual double X(double t);
    virtual double Y(double t);
    virtual double derivX(double t);
    virtual double derivY(double t);
    virtual double deriv2X(double t);
    virtual double deriv2Y(double t);
    virtual double curveLength(double t0,double t1);
};

class CCurveEllipse : public CCurve
{
    double a,b,wx,wy,th0x,th0y;
public:
    CCurveEllipse(double _a,double _b,double _tf,double _wx=1,double _wy=1,double _th0x=0,double _th0y=0);
    virtual double X(double t);
    virtual double Y(double t);
    virtual double derivX(double t);
    virtual double derivY(double t);
    virtual double deriv2X(double t);
    virtual double deriv2Y(double t);
};

class CCurveSpline : public CCurve
{
    int n;
    double *xx,*fx,*bx,*cx,*dx;
    double *yy,*fy,*by,*cy,*dy;
public:
    QList<Vector2D> knots;
    CCurveSpline();
    void init(bool cubic_clamped_spline=false, double deriv_x0=0, double deriv_y0=0, double deriv_xf=0, double deriv_yf=0);
    virtual double X(double t);
    virtual double Y(double t);
};

class CCurveHeart : public CCurve
{
    double xs, ys, w, a;
public:
    CCurveHeart(double _xs,double _ys, double _a,double _w);
    virtual double X(double t);
    virtual double Y(double t);
};


void tridiagonal ( int n, double *c, double *a, double *b, double *r );
void cubic_nak ( int n, double *x, double *f, double *b, double *c, double *d );
void cubic_clamped ( int n, double *x, double *f, double *b, double *c,double *d, double fpa, double fpb );
double spline_eval ( int n, double *x, double *f, double *b, double *c,double *d, double t );
double spline_eval_deriv ( int n, double *x, double *f, double *b, double *c,double *d, double t );
double spline_eval_deriv2 ( int n, double *x, double *f, double *b, double *c,double *d, double t );

#endif // CURVE_H
