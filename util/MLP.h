#ifndef __ANN_H
#define __ANN_H

#include "util/NNFPP.h"
#include <QVector>
#include <QString>
#include <QTime>
typedef QVector<float> TFloatArray;


struct TTrainElem {    
   TFloatArray in,out;
};

class TTrainingInfo {
  private:
   float err;
   float errg;
   QTime past;
   int iter;
  public:
   TTrainingInfo(float __e,float __eg,float __lr,bool __term,QTime __past,int __iter);
   float Error();
   float Error_Goal();
   QTime PastTime();
   int PastIterations();
   float LearningRate;
   bool Terminate;
};
class TTrainSet {
  public:
    QVector<TTrainElem> t;
    void Assign(TTrainSet &s);
    void Add(TTrainSet &s);
    void Add(TFloatArray &in,TFloatArray &out);
    void Add(TFloatArray &in,int outputlen,int desired,float def);
    void LoadFromFile(QString File);
    void AddFromFile(QString File);
    void SaveToFile(QString File);
    TTrainSet& operator =(TTrainSet &s);
};
class TMLP;
typedef void TOnBeforeTrainEvent(TMLP* Sender);
typedef void TOnAfterTrainEvent(TMLP* Sender);
typedef void TOnTrainEvent(TMLP* Sender,TTrainingInfo &traininginfo);
class TMLP{
  private:
   NNFpp* nn;
   TFloatArray oerr;
   void TrainEpoch();
   bool allocated;
  public:
   TTrainSet TrainSet;

   TOnBeforeTrainEvent *OnBeforeTrainEvent;
   TOnTrainEvent       *OnTrainEvent;
   TOnAfterTrainEvent  *OnAfterTrainEvent;

   float LearningRate;

   TMLP();
   ~TMLP();
   TMLP(int layers, int neurons[], float weight_range, int Activation_Function, float Learningrate);   


   void Init(int layers, int neurons[], float weight_range, int Activation_Function, float Learningrate);   
   void Train(float Error_Goal);
   int Process(TFloatArray in,TFloatArray &out,float threshold);

   void SaveToFile(QString filename);
   void LoadFromFile(QString filename);

   int LayersCount();
   int NeuronsCount(int layer);
   int GetInputLayerSize();
   int GetOutputLayerSize();
   int ActivationFunc(int layer);
   int GetPatternsCount();

   float& Value(int layer,int neuron);
   float& Bias(int layer,int neuron);
   float& Weight(int layer,int neuron_a,int neuron_b);
   float  OutputError(int neuron);
};


#endif
