#ifndef MOTIONESTIMATOR_H
#define MOTIONESTIMATOR_H
#ifndef Q_WS_MAC
#include "libsvm/svm.h"
#endif
#include <geom.h>
class QString;
class QTextStream;
class QFile;
struct TrainStruct
{
    Vector2D vel1,vel2;
	double t, theta, dist;
};

class CSVMEstimator
{
public:
#ifndef Q_WS_MAC
    svm_parameter param;
    struct svm_model* model;
#endif
    CSVMEstimator();
    void setParams(QString input_line);
    void train(double** input, double* output, int inputsize, int samples);
    double estimate(double *input, int inputsize);
    void save(QString filename);
    void load(QString filename);
    int loadSamplesFromCSV(QString filename, double*** input, double **output, int& inputsize);
};

class CMotionEstimator
{
public:
    double **input;
    double *output;
    int inputsize;
    int samples;
    CMotionEstimator();
    CSVMEstimator* svm;
    static TrainStruct trainStructOf(Vector2D pos1,Vector2D vel1, Vector2D pos2, Vector2D vel2);
    static double* normalizedStruct(TrainStruct ts);
    double estimate(Vector2D pos1, Vector2D vel1, Vector2D pos2, Vector2D vel2);
    void train();
    void loadSamplesFromCSV(QString filename);
};

#endif // MOTIONESTIMATOR_H
