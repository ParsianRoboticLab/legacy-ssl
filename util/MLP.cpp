#include "MLP.h"
TTrainingInfo::TTrainingInfo (float __e,float __eg,float __lr,bool __term,QTime __past,int __iter) {err = __e;errg = __eg;LearningRate=__lr;Terminate=__term;past = __past;iter = __iter;};
float TTrainingInfo::Error() {return err;}
float TTrainingInfo::Error_Goal() {return errg;}
QTime TTrainingInfo::PastTime() {return past;}
int TTrainingInfo::PastIterations() {return iter;}
TTrainSet& TTrainSet::operator =(TTrainSet &s) {Assign(s);return *this;}
TMLP::TMLP() {allocated=false;}
TMLP::~TMLP() {if (allocated) delete nn;}

int TMLP::LayersCount() {return nn->layers;}
int TMLP::NeuronsCount(int layer) {return nn->neurons[layer];}
int TMLP::GetInputLayerSize() {return nn->neurons[0];}
int TMLP::GetOutputLayerSize() {return nn->neurons[nn->layers-1];}
int TMLP::ActivationFunc(int layer) {return nn->activationFunction[layer];}
int TMLP::GetPatternsCount() {return TrainSet.t.count();}

float& TMLP::Value(int layer,int neuron) {return nn->value[layer][neuron];}
float& TMLP::Bias(int layer,int neuron) {return nn->bias[layer][neuron];}
float& TMLP::Weight(int layer,int neuron_a,int neuron_b) {return nn->weight[layer][neuron_a][neuron_b];}
float  TMLP::OutputError(int neuron) {return oerr[neuron];}

TMLP::TMLP(int layers, int neurons[], float weight_range, int Activation_Function, float Learningrate) //Constructor
{
   allocated = false;
   Init(layers,neurons,weight_range,Activation_Function,Learningrate);
}

void TMLP::Init(int layers, int neurons[], float weight_range, int Activation_Function, float Learningrate)
{
  OnBeforeTrainEvent = NULL;
  OnTrainEvent       = NULL;
  OnAfterTrainEvent  = NULL;
  if (allocated) delete nn;  
  nn = new NNFpp(layers,neurons,weight_range,Activation_Function,Learningrate);
  oerr.resize(neurons[layers-1]);
  LearningRate = Learningrate;
  allocated = true;
}


void TMLP::SaveToFile(QString filename)
{
   FILE *f = fopen(filename.toStdString().c_str(),"w");
   if (f==NULL) return;
   nn->SaveNet(f);
   fclose(f);
}

void TMLP::LoadFromFile(QString filename)
{   
   FILE *f = fopen(filename.toStdString().c_str(),"r");
   if (f==NULL) return;
   //if (allocated) delete nn;
   nn = new NNFpp(f);
   fclose(f);
}

void TMLP::TrainEpoch()
{
   float zero=0;oerr.fill(zero);
   delete nn->Errors;
   nn->Errors = new Error;
   int patts = TrainSet.t.count();
   float *in, *des, *out;
   int in_N = nn->neurons[0];
   int out_N = nn->neurons[nn->layers-1];
   in = (float*)malloc(in_N*sizeof(float));
   des = (float*)malloc(out_N*sizeof(float));
   for (int p=0;p<patts;p++)
   {                     
       in = TrainSet.t[p].in.begin();
       des = TrainSet.t[p].out.begin();

       out = nn->Execute(in);
       nn->BackPropagateOnLine(des);
       nn->Errors->ErrorQuadratic(out,des,out_N);
       nn->Errors->Accuracy(out,des,out_N);

       for (int j=0;j<nn->neurons[nn->layers-1];j++)
           oerr[j] += fabs(nn->value[nn->layers-1][j]-des[j]);
   }
}

void TMLP::Train(float Error_Goal)
{
  QTime timepast;
  timepast.start();
  if (OnBeforeTrainEvent != NULL) (*OnBeforeTrainEvent)(this);
  nn->epsilon = LearningRate;
  int iter=0;
  while (1)
  {
    TrainEpoch();
    TTrainingInfo ti(nn->Errors->CumulativeError,Error_Goal,nn->epsilon,false,timepast,iter);
    if  (OnTrainEvent!=NULL) OnTrainEvent(this,ti);
    nn->epsilon = ti.LearningRate;
    if (ti.Terminate) break;
    if (ti.Error()<Error_Goal) break;
    fprintf(stderr,"%.5f",ti.Error());/**/
    iter ++;
  }
  if (OnAfterTrainEvent != NULL) (*OnAfterTrainEvent)(this);
}

int TMLP::Process(TFloatArray in,TFloatArray& out,float threshold)
{
   out.resize(nn->neurons[nn->layers-1]);
   nn->Execute(in.begin());
   float best = -1e5;
   int best_ind = 0;
   for (int i=0;i<nn->neurons[nn->layers-1];i++)
   {
     out[i] = nn->value[nn->layers-1][i];
     if (out[i]>best) {best = out[i];best_ind=i;}
   }
   if (best<threshold)
   best_ind = -1;
   return best_ind;
}

void TTrainSet::Assign(TTrainSet &s)
{
    t.clear();
     Add(s);
}

void TTrainSet::Add(TTrainSet &s)
{
     int Count = s.t.count();
     int Last = t.count();
     t.resize(Count+Last);
     for (int i=0;i<Count;i++)
     {
        t[i+Last].in = s.t[i].in;
        t[i+Last].out = s.t[i].out;
     }
}

void TTrainSet::Add(TFloatArray &in,TFloatArray &out)
{
  TTrainElem tt;
  tt.in = in;
  tt.out = out;
  t.push_back(tt);
}

void TTrainSet::Add(TFloatArray &in,int outputlen,int desired,float def)
{
  TFloatArray out;
  out.resize(outputlen);
  out.fill(def);
  out[desired] = 1;
  Add(in,out);
}

void TTrainSet::LoadFromFile(QString File)
{
   t.clear();
   AddFromFile(File);
}

void TTrainSet::AddFromFile(QString File)
{
   FILE *f = fopen(File.toStdString().c_str(),"r");
   if (f==NULL) return;
   int exams=0;
   char str[100];
   fscanf(f,"%s",str);
   fscanf(f,"%s",str);
   fscanf(f,"%s",str);
   fscanf(f,"%s",str);
   fscanf(f,"%d",&exams);
   if (exams==0) {fclose(f);return;}
   int li=0;
   int lo=0;
   fscanf(f,"%s",str);
   fscanf(f,"%d",&li);
   fscanf(f,"%s",str);
   fscanf(f,"%d",&lo);
   int last = t.count();
   t.resize(exams+last);
   for (int i=0;i<exams;i++)
   {
      t[i+last].in.resize(li);
      t[i+last].out.resize(lo);      
      for (int j=0;j<li;j++)
         fscanf(f,"%f",&(t[i+last].in[j]));
      for (int j=0;j<lo;j++)
         fscanf(f,"%f",&(t[i+last].out[j]));
   }
   fclose(f);
}

void TTrainSet::SaveToFile(QString File)
{
   FILE *f = fopen(File.toStdString().c_str(),"w");
   if (f==NULL) return;
   int exams = t.count();
   fprintf(f,"Train Set File:\n");
   fprintf(f,"Examples:\n");
   fprintf(f,"%d\n",exams);
   if (exams==0) {fclose(f);delete f;return;}
   int li = t[0].in.count();
   int lo = t[0].out.count();
   fprintf(f,"Input:\n");
   fprintf(f,"%d\n",li);
   fprintf(f,"Output:\n");
   fprintf(f,"%d\n",lo);
   for (int i=0;i<exams;i++)
   {
      for (int j=0;j<li;j++)
         fprintf(f,"%f ",t[i].in[j]);
      fprintf(f,"\n");      
      for (int j=0;j<lo;j++)
         fprintf(f,"%f ",t[i].out[j]);
      fprintf(f,"\n");
   }
   fclose(f);   
}

