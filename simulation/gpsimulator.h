#ifndef GPSIMULATOR_H
#define GPSIMULATOR_H
#include "qclcontext.h"


class CGPSimulator
{
public:
    QCLContext context;
    QCLProgram program;
    QCLVector<float> obsx;
    QCLVector<float> obsy;
    QCLVector<float> weights;
    QCLVector<int> cc;
    QCLKernel evalpoint;
    QCLKernel hgaussian;
    QCLImage2D dstImageBuffer;
    CGPSimulator();
};

#endif // GPSIMULATOR_H
