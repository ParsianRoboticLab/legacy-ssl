#include "motionestimator.h"
#include "base.h"
#include <cstdlib>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QStringList>

using namespace std;

#define NO_SVM

CSVMEstimator::CSVMEstimator()
{
#ifndef NO_SVM
    model = NULL;
#endif
}

//TODO define desctructor
void CSVMEstimator::setParams(QString input_line)
{
//    int i,j;
    // default values
#ifndef NO_SVM
    param.svm_type = C_SVC;
    param.kernel_type = RBF;
    param.degree = 3;
    param.gamma = 0;
    param.coef0 = 0;
    param.nu = 0.5;
    param.cache_size = 100;
    param.C = 1;
    param.eps = 1e-3;
    param.p = 0.1;
    param.shrinking = 1;
    param.probability = 0;
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;

    // parse options
    const char *p = input_line.toAscii().constData();

    while (1) {
            while (*p && *p != '-')
                    p++;

            if (*p == '\0')
                    break;

            p++;
            switch (*p++) {
                    case 's':
                            param.svm_type = atoi(p);
                            break;
                    case 't':
                            param.kernel_type = atoi(p);
                            break;
                    case 'd':
                            param.degree = atoi(p);
                            break;
                    case 'g':
                            param.gamma = atof(p);
                            break;
                    case 'r':
                            param.coef0 = atof(p);
                            break;
                    case 'n':
                            param.nu = atof(p);
                            break;
                    case 'm':
                            param.cache_size = atof(p);
                            break;
                    case 'c':
                            param.C = atof(p);
                            break;
                    case 'e':
                            param.eps = atof(p);
                            break;
                    case 'p':
                            param.p = atof(p);
                            break;
                    case 'h':
                            param.shrinking = atoi(p);
                            break;
                    case 'b':
                            param.probability = atoi(p);
                            break;
                    case 'w':
                            ++param.nr_weight;
                            param.weight_label = (int *)realloc(param.weight_label,sizeof(int)*param.nr_weight);
                            param.weight = (double *)realloc(param.weight,sizeof(double)*param.nr_weight);
                            param.weight_label[param.nr_weight-1] = atoi(p);
                            while(*p && !isspace(*p)) ++p;
                            param.weight[param.nr_weight-1] = atof(p);
                            break;
            }
    }
#endif
}

void CSVMEstimator::train(double**input, double *output, int inputsize, int samples)
{
#ifndef NO_SVM
    if (param.gamma == 0) param.gamma = 0.5;
    svm_problem prob;
    prob.l = samples;
    prob.y = new double[prob.l];
    prob.x = new svm_node *[prob.l];
    svm_node *x_space = new svm_node[prob.l*(inputsize+1)];
    for (int i=0;i<samples;i++)
    {
        prob.y[i] = output[i];
        for (int j=0;j<inputsize;j++)
        {
            x_space[i*(inputsize+1) + j].value = input[i][j];
            x_space[i*(inputsize+1) + j].index = j + 1;
        }
        x_space[i*(inputsize+1) + inputsize].index = -1;
        prob.x[i] = &x_space[i*(inputsize+1)];
    }
    // build model & classify
    model = svm_train(&prob, &param);
    qDebug() << "trained!!!";
#endif
}

int CSVMEstimator::loadSamplesFromCSV(QString filename, double*** input, double **output, int& inputsize)
        //each line is #,x1,x2,...,xn,y
{
#ifndef NO_SVM
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Could not open the file";
        return -1;
    }    
    QTextStream in(&file);
    QString line;
    inputsize = -1;
    int samples = 0;
    do {
        line = in.readLine();
        if (!line.isNull())
        {
            QStringList values = line.split(",");
            if (inputsize != -1)
            {
                if (inputsize != values.count() - 2)
                {
                    qDebug() << "Bad format in csv file";
                    return -1;
                }
            }
            inputsize = values.count() - 2;
            samples ++;
        }
    } while (!line.isNull());
    in.seek(0);
    int k=0;
    *input = new double *[samples];
    *output = new double [samples];
    do {
        line = in.readLine();
        if (!line.isNull())
        {
            QStringList values = line.split(",");
            (*input)[k] = new double [inputsize];
            for (int i=0;i<inputsize;i++)
            {
                (*input)[k][i] = values[i+1].toDouble();
            }
            (*output)[k] = values[inputsize+1].toDouble();
            k ++;
        }
    } while (!line.isNull());
    return samples;
#endif
}

void CSVMEstimator::save(QString filename)
{
#ifndef NO_SVM
    svm_save_model(filename.toAscii().constData(), model);
#endif
}

void CSVMEstimator::load(QString filename)
{
#ifndef NO_SVM
    if (model != NULL) svm_destroy_model(model);
    model = svm_load_model(filename.toAscii().constData());
#endif
}

double CSVMEstimator::estimate(double *input, int inputsize)
{
#ifndef NO_SVM
    if (model==NULL) return -1;
    svm_node *x = new svm_node[inputsize + 1];
    for (int i=0;i<inputsize;i++)
    {
        x[i].index = i + 1;
        x[i].value = input[i];
    }
    x[inputsize].index = -1;
    return svm_predict(model, x);
#endif
}

CMotionEstimator::CMotionEstimator()
{
    svm = new CSVMEstimator();
    input = NULL;
    output = NULL;
    inputsize = -1;
    samples = -1;
}

TrainStruct CMotionEstimator::trainStructOf(Vector2D pos1,Vector2D vel1, Vector2D pos2, Vector2D vel2)
{
//    TrainStruct st;
//    double angle = pos1.dir().degree();
//    st.dist = (pos2 - pos1); //final-initial
//    //st.dist = AngleDeg::normalize_angle(pos2.dir().degree() - pos1.dir().degree());
//    st.vel1 = vel1;
//    st.vel2 = vel2;
//    st.dist.rotate(-angle);
//    st.vel1.rotate(-angle);
//    st.vel2.rotate(-angle);
//    return st;
	TrainStruct st;
//    double angle = pos1.dir().degree();
	st.dist = (pos2 - pos1).length(); //final-initial
	st.vel1 = vel1;
	st.vel2 = vel2;
//    st.vel1.rotate(-angle);
//    st.vel2.rotate(-angle);
	st.theta = Vector2D::angleBetween(vel1,(pos2 - pos1)).degree();
	if( vel2.length() < 0.2)
		st.theta = pos1.dir().degree();
	return st;
}

double* CMotionEstimator::normalizedStruct(TrainStruct ts)
{
    float diameter = hypot(_FIELD_WIDTH,_FIELD_HEIGHT);
    float vmax = 4;
    double *l = new double[8];
//    l[0] = ts.dist.x / diameter;
//    l[1] = ts.dist.y / diameter;
//    l[2] = (ts.dist.dir().degree()+180.0) / 360.0;
//    l[3] = ts.vel1.x / vmax;
//    l[4] = ts.vel1.y / vmax;
//    l[5] = ts.vel2.x / vmax;
//    l[6] = ts.vel2.y / vmax;
//    l[7] = ts.t;
    return l;
}

void CMotionEstimator::train()
{
    if (inputsize!=-1)
    {
        //svm->setParams("-s 3 -t 2 -p 0.1 -c 8 -g 6.0628");
        svm->setParams("-s 3 -t 2 -p 0.1 -c 1.68 -g 6.72"); //20/3/89 learned with robots, optimized with matlab
        svm->train(input, output, inputsize, samples);
    }
}

double CMotionEstimator::estimate(Vector2D pos1, Vector2D vel1, Vector2D pos2, Vector2D vel2)
{
    TrainStruct ts = trainStructOf(pos1, vel1, pos2, vel2);
    double *input = normalizedStruct(ts);
    double time = svm->estimate(input, 7);
    delete input;
    return time;
}

void CMotionEstimator::loadSamplesFromCSV(QString filename)
{
    samples = svm->loadSamplesFromCSV(filename, &input, &output, inputsize);
}
