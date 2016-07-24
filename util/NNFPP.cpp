/***************************************************************************
 *                                                                         *
 *                  NNFpp - Neural Net Framework plus plus                 *
 *                                                                         *
 *                                                                         *
 *                                                                         *
 *   Copyright (C) 2006 by Pierluigi Failla                                *
 *   nnfpp.project@gmail.com                                               *
 *                                                                         *
 *   http://nnfpp.sourceforge.net                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License                                                               *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
//---------------------------------------------------------------------------

#pragma hdrstop

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
//#include <conio.h>
#include <time.h>
#include <string.h>

#include "NNFPP.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

//NNFpp---------------------------------------------------------------------------

NNFpp::NNFpp(int layrs, int neurs[], int range, int A_Function, float Epsilon)
{
	InitType = 0;
 	layers = layrs;
	neurons = (int*)malloc(layrs*sizeof(int));
	bias = (float**)malloc(layrs*sizeof(float*));
	value = (float**)malloc(layrs*sizeof(float*));
	delta = (float**)malloc(layrs*sizeof(float*));
	weight = (float***)malloc(layrs*sizeof(float**));
	activationFunction = (int*)malloc(layrs*sizeof(int));
 	for (int i=0;i<layrs;i++)
	{
		neurons[i] = neurs[i];
	}
	for (int i=0;i<layrs;i++)
	{
		bias[i] = (float*)malloc(neurons[i]*sizeof(float));
		value[i] = (float*)malloc(neurons[i]*sizeof(float));
		delta[i] = (float*)malloc(neurons[i]*sizeof(float));
		weight[i] = (float**)malloc(neurons[i]*sizeof(float*));
		if (i)
		{
			for (int j=0;j<neurons[i];j++)
			{
				weight[i][j] = (float*)malloc(neurons[i-1]*sizeof(float));
			}
		}
	}
 	srand(time(NULL));
 	for (int i=1;i<layrs;i++)
	{
  		for (int j=0;j<neurs[i];j++)
		{
			bias[i][j] =  2.0 * range * ((float) rand() / RAND_MAX - 0.5);
			for (int k=0;k<neurs[i-1];k++)
			{
				weight[i][j][k] =  2.0 * range * ((float) rand() / RAND_MAX - 0.5);
			}
  		}
	}

	for(int i=0; i<layrs; i++)
	{
		activationFunction[i] = A_Function;
	}
	epsilon = Epsilon;
	Errors = new Error;
}

NNFpp::NNFpp(int layrs, int neurs[], int range, int A_Function, float FunctionParameter, float Epsilon)
{
	InitType = 1;
 	layers = layrs;
	neurons = (int*)malloc(layrs*sizeof(int));
	bias = (float**)malloc(layrs*sizeof(float*));
	value = (float**)malloc(layrs*sizeof(float*));
	delta = (float**)malloc(layrs*sizeof(float*));
	weight = (float***)malloc(layrs*sizeof(float**));
	activationFunction = (int*)malloc(layrs*sizeof(int));
	alfa = (float *)malloc(layrs*sizeof(float));
 	for (int i=0;i<layrs;i++)
	{
  		neurons[i] = neurs[i];
	}
	for (int i=0;i<layrs;i++)
	{
		bias[i] = (float*)malloc(neurons[i]*sizeof(float));
		value[i] = (float*)malloc(neurons[i]*sizeof(float));
		delta[i] = (float*)malloc(neurons[i]*sizeof(float));
		weight[i] = (float**)malloc(neurons[i]*sizeof(float*));
		if (i)
		{
			for (int j=0;j<neurons[i];j++)
			{
				weight[i][j] = (float*)malloc(neurons[i-1]*sizeof(float));
			}
		}
	}
 	srand(time(NULL));
 	for (int i=1;i<layrs;i++)
	{
  		for (int j=0;j<neurs[i];j++)
		{
			bias[i][j] =  2.0 * range * ((float) rand() / RAND_MAX - 0.5);
			for (int k=0;k<neurs[i-1];k++)
			{
				weight[i][j][k] =  2.0 * range * ((float) rand() / RAND_MAX - 0.5);
			}
  		}
	}

	for(int i=0; i<layrs; i++)
	{
		activationFunction[i] = A_Function;
	}

	for(int i=0; i<layrs; i++)
	{
		alfa[i] = FunctionParameter;
	}
	epsilon = Epsilon;
	Errors = new Error;
}

NNFpp::NNFpp(int layrs, int neurs[], int range, int A_Function[], float FunctionParameter[], float Epsilon)
{
	InitType = 2;
 	layers = layrs;
	neurons = (int*)malloc(layrs*sizeof(int));
	bias = (float**)malloc(layrs*sizeof(float*));
	value = (float**)malloc(layrs*sizeof(float*));
	delta = (float**)malloc(layrs*sizeof(float*));
	weight = (float***)malloc(layrs*sizeof(float**));
	activationFunction = (int*)malloc(layrs*sizeof(int));
	alfa = (float *)malloc(layrs*sizeof(float));
 	for (int i=0;i<layrs;i++)
	{
  		neurons[i] = neurs[i];
	}
	for (int i=0;i<layrs;i++)
	{
		bias[i] = (float*)malloc(neurons[i]*sizeof(float));
		value[i] = (float*)malloc(neurons[i]*sizeof(float));
		delta[i] = (float*)malloc(neurons[i]*sizeof(float));
		weight[i] = (float**)malloc(neurons[i]*sizeof(float*));
		if (i)
		{
			for (int j=0;j<neurons[i];j++)
			{
				weight[i][j] = (float*)malloc(neurons[i-1]*sizeof(float));
			}
		}
	}
 	srand(time(NULL));
 	for (int i=1;i<layrs;i++)
	{
  		for (int j=0;j<neurs[i];j++)
		{
			bias[i][j] =  2.0 * range * ((float) rand() / RAND_MAX - 0.5);
			for (int k=0;k<neurs[i-1];k++)
			{
				weight[i][j][k] =  2.0 * range * ((float) rand() / RAND_MAX - 0.5);
			}
  		}
	}

	for(int i=0; i<layrs-1; i++)
	{
		activationFunction[i] = A_Function[i];
	}

	for(int i=0; i<layrs-1; i++)
	{
		alfa[i] = FunctionParameter[i];
	}
	epsilon = Epsilon;
	Errors = new Error;
}

NNFpp::NNFpp(FILE *NNFfile)
{
	InitType = 3;
	FileForReset = NNFfile;
	char str[100];
	fscanf(NNFfile,"%s",str);

	if(strcmp(str,"MLP")==0){

	for(int i=2; i<=12; i++)
	{
		fscanf(NNFfile,"%s",str);
	}
 	fscanf(NNFfile,"%d",&layers);
 	fscanf(NNFfile,"%s",str);
 	fscanf(NNFfile,"%s",str);
 	fscanf(NNFfile,"%s",str);

	neurons = (int*)malloc(layers*sizeof(int));
	bias = (float**)malloc(layers*sizeof(float*));
	value = (float**)malloc(layers*sizeof(float*));
	delta = (float**)malloc(layers*sizeof(float*));
	weight = (float***)malloc(layers*sizeof(float**));
	activationFunction = (int*)malloc(layers*sizeof(int));
	alfa = (float *)malloc(layers*sizeof(float));

 	for (int i=0;i<layers;i++)
	{
		fscanf(NNFfile,"%d",&neurons[i]);
	}

	for (int i=0;i<layers;i++)
	{
		bias[i] = (float*)malloc(neurons[i]*sizeof(float));
		value[i] = (float*)malloc(neurons[i]*sizeof(float));
		delta[i] = (float*)malloc(neurons[i]*sizeof(float));
		weight[i] = (float**)malloc(neurons[i]*sizeof(float*));
		if (i)
		{
			for (int j=0;j<neurons[i];j++)
			{
				weight[i][j] = (float*)malloc(neurons[i-1]*sizeof(float));
			}
		}
	}

 	fscanf(NNFfile,"%s",str);
 	fscanf(NNFfile,"%f",&epsilon);
	fscanf(NNFfile,"%s",str);
	fscanf(NNFfile,"%s",str);
	for(int i=0; i<layers; i++)
	{
		fscanf(NNFfile,"%s",str);
		if (!strcmp(str,"step"))
		{
			activationFunction[i] = NNF_STEP;
			fscanf(NNFfile,"%s",str);
			fscanf(NNFfile,"%f",&alfa[i]);
		}
		else
		{
			if (!strcmp(str,"sign"))
			{
				activationFunction[i] = NNF_SIGN;
			}
			else
			{
				if (!strcmp(str,"sigma_hyperbolic"))
				{
					activationFunction[i] = NNF_SIGMA_HYPERBOLIC;
				}
				else
				{
					if(!strcmp(str,"simil_sigmoid"))
					{
						activationFunction[i] = NNF_SIMIL_SIGMOID;
						fscanf(NNFfile,"%s",str);
						fscanf(NNFfile,"%f",&alfa[i]);
					}
					else
					{
						if(!strcmp(str,"sinc"))
						{
							activationFunction[i] = NNF_SINC;
						}
						else
						{
							activationFunction[i] = NNF_SIGMA_LOGISTIC;
						}
					}
				}
			}
		}
	}
 	fscanf(NNFfile,"%s",str);
 	for (int i=1;i<layers;i++)
	{
		 for (int j=0;j<neurons[i];j++)
		 {
 		 	fscanf(NNFfile,"%f",&bias[i][j]);
   			for (int k=0;k<neurons[i-1];k++)
			{
				fscanf(NNFfile,"%f",&weight[i][j][k]);
			}
  		 }
	}

	} //if MLP

	fseek(NNFfile,0,0);
	Errors = new Error;
}

NNFpp::~NNFpp(void)
{
	int i,j;
	for (i=0;i<layers;i++)
	{
		free(bias[i]);
		free(value[i]);
		free(delta[i]);
		if (i)
		{
			for (j=0;j<neurons[i];j++)
			{
				free(weight[i][j]);
			}
		}
		free(weight[i]);
	}
	free(neurons);
	free(activationFunction);
	free(alfa);
	delete Errors;
}

void NNFpp::LoadNet(FILE *loadFile)
{
 	char str[100];
	fscanf(loadFile,"%s",str);

	if(strcmp(str,"MLP")==0){

	for(int i=2; i<=12; i++)
	{
		fscanf(loadFile,"%s",str);
	}
 	fscanf(loadFile,"%d",&layers);
 	fscanf(loadFile,"%s",str);
 	fscanf(loadFile,"%s",str);
 	fscanf(loadFile,"%s",str);
 	for (int i=0;i<layers;i++)
	{
		fscanf(loadFile,"%d",&neurons[i]);
	}
 	fscanf(loadFile,"%s",str);
 	fscanf(loadFile,"%f",&epsilon);
	fscanf(loadFile,"%s",str);
	fscanf(loadFile,"%s",str);
	fscanf(loadFile,"%s",str);
	for(int i=0; i<layers; i++)
	{
		if (!strcmp(str,"step"))
		{
		        activationFunction[i] = NNF_STEP;
		        fscanf(loadFile,"%s",str);
		        fscanf(loadFile,"%f",&alfa);
		}
		else
		{
			if (!strcmp(str,"sign"))
			{
				activationFunction[i] = NNF_SIGN;
			}
			else
			{
				if (!strcmp(str,"sigma_hyperbolic"))
				{
					activationFunction[i] = NNF_SIGMA_HYPERBOLIC;
				}
				else
				{
					if(!strcmp(str,"simil_sigmoid"))
					{
						activationFunction[i] = NNF_SIMIL_SIGMOID;
						fscanf(loadFile,"%s",str);
						fscanf(loadFile,"%f",&alfa);
					}
					else
					{
						if(!strcmp(str,"sinc"))
						{
							activationFunction[i] = NNF_SINC;
						}
						else
						{
							activationFunction[i] = NNF_SIGMA_LOGISTIC;
						}
					}
				}
			}
		}
	}
 	fscanf(loadFile,"%s",str);
 	for (int i=1;i<layers;i++)
	{
		 for (int j=0;j<neurons[i];j++)
		 {
 		 	fscanf(loadFile,"%f",&bias[i][j]);
   			for (int k=0;k<neurons[i-1];k++)
			{
				fscanf(loadFile,"%f",&weight[i][j][k]);
			}
  		 }
	}

	}//if MLP

	fseek(loadFile,0,0);
}

void NNFpp::SaveNet(FILE *saveFile)
{
 	fprintf(saveFile,"%s\n\n","MLP NNF   -   Neural Net Framework");
 	fprintf(saveFile,"%s\n\n","__Neural Net Save File__");
 	fprintf(saveFile,"%s\n","CONSTANTS");
	fprintf(saveFile,"%s\n","Layers:");
 	fprintf(saveFile,"%d\n",layers);
	fprintf(saveFile,"%s\n","Neurons Per Layer:");
 	for (int i=0;i<layers;i++)
	{
  		fprintf(saveFile,"%d ",neurons[i]);
	}
 	fprintf(saveFile,"\n");
	fprintf(saveFile,"%s\n","Epsilon:");
 	fprintf(saveFile,"%f\n",epsilon);
	fprintf(saveFile,"%s\n","Activation Function:");
        for(int i=0; i<layers; i++)
        {
	        switch (activationFunction[i])
                {
		        case NNF_STEP:
			        fprintf(saveFile,"%s\n","step");
			        fprintf(saveFile,"Parameter: %f\n",alfa[i]);
			        break;
		        case NNF_SIGN:
			        fprintf(saveFile,"%s\n","sign");
			        break;
		        case NNF_SIGMA_LOGISTIC:
			        fprintf(saveFile,"%s\n","sigma_logistic");
			        break;
		        case NNF_SIGMA_HYPERBOLIC:
			        fprintf(saveFile,"%s\n","sigma_hyperbolic");
			        break;
				case NNF_SIMIL_SIGMOID:
			        fprintf(saveFile,"%s\n","simil_sigmoid");
					fprintf(saveFile,"Parameter: %f\n",alfa[i]);
					break;
				case NNF_SINC:
			        fprintf(saveFile,"%s\n","sinc");
					break;
	        }
        }
	fprintf(saveFile,"%s\n","WEIGHTS:");
 	for (int i=1;i<layers;i++)
	{
  		for (int j=0;j<neurons[i];j++)
		{
   			fprintf(saveFile,"%f\n",bias[i][j]);
   			for (int k=0;k<neurons[i-1];k++)
			{
				fprintf(saveFile,"%f ",weight[i][j][k]);
			}
   			fprintf(saveFile,"\n");
  		}
 	}
}

void NNFpp::ResetNet(int range)
{
        if(InitType == 0)
        {
	        NNFpp(layers,neurons,range,activationFunction[0],epsilon);
        }
        if(InitType == 1)
        {
	        NNFpp(layers,neurons,range,activationFunction[0],alfa[0],epsilon);
        }
        if(InitType == 2)
        {
	        NNFpp(layers,neurons,range,activationFunction,alfa,epsilon);
        }
        if(InitType == 3)
        {
	        NNFpp(this->FileForReset);
        }
}

float* NNFpp::Execute(float input[])
{
	float a;
 	for (int i=0;i<neurons[0];i++)
	{
  		value[0][i] = input[i];
	}
 	for (int i=1;i<layers;i++)
	{
  		for (int j=0;j<neurons[i];j++)
		{
   			a = 0.0;
   			for (int k=0;k<neurons[i-1];k++)
                        {
   		 		a += weight[i][j][k] * value[i-1][k];
			}
   			a += bias[i][j];
			switch (activationFunction[i-1])
			{
				case NNF_STEP:
					value[i][j] = NNF_FUNC_STEP(a,alfa[i-1]);
					break;
				case NNF_SIGN:
					value[i][j] = NNF_FUNC_SIGN(a);
					break;
				case NNF_SIGMA_LOGISTIC:
					value[i][j] = NNF_FUNC_SIGMA_LOGISTIC(a);
					break;
				case NNF_SIGMA_HYPERBOLIC:
					value[i][j] = NNF_FUNC_SIGMA_HYPERBOLIC(a);
					break;
				case NNF_SIMIL_SIGMOID:
					value[i][j] = NNF_FUNC_SIMIL_SIGMOID(a,alfa[i-1]);
					break;
				case NNF_SINC:
					value[i][j] = NNF_FUNC_SINC(a);
					break;
			}
	  	}
	}
	return value[layers-1];
}

void NNFpp::BackPropagateOnLine(float desired[])
{
 	float err;
 	for (int i=0;i<neurons[layers-1];i++)
	{
		err = desired[i] - value[layers-1][i];
		err = LearningBP(i,err,layers-1);
 	} 
 	for (int i=layers-2;i>0;i--)
	{
  		for (int j=0;j<neurons[i];j++)            
		{
   			err = 0.0;
   			for (int k=0;k<neurons[i+1];k++)
			{
				err += delta[i+1][k] * weight[i+1][k][j];
			}
			err = LearningBP(j,err,i);
  		}
	}
 	for (int i=layers-1;i>0;i--)
	{
  		for (int j=0;j<neurons[i];j++)
		{
   			for (int k=0;k<neurons[i-1];k++)
			{
				this->weight[i][j][k] += epsilon * delta[i][j] * value[i-1][k];
			}
   			this->bias[i][j] += epsilon * delta[i][j];
  		}
	}
}

float NNFpp::LearningBP(int i, float err, int l)
{
        switch (activationFunction[l])
        {
                case NNF_STEP:
                        delta[l][i] = err;
                        break;
                case NNF_SIGN:
                        delta[l][i] = err;
                        break;
                case NNF_SIGMA_LOGISTIC:
                        delta[l][i] = err * value[l][i] * (1 - value[l][i]);
                        break;
                case NNF_SIGMA_HYPERBOLIC:
                        delta[l][i] = err * (1 - pow(value[l][i],2));
                        break;
                case NNF_SIMIL_SIGMOID:
                        delta[l][i] = err * (value[l][i]-0.5)*(abs(value[l][i]-0.5)+0.5);
                        break;
                case NNF_SINC:
                        if(value[l][i]>0)
                        {
                                delta[l][i] = err * value[l][i] * (1 - value[l][i]);
                        }
                        else
                        {
                                delta[l][i] = -err * value[l][i] * (1 - value[l][i]);
                        }
                        break;
        }
        return err;
}

bool NNFpp::TrainingSetFromFile(FILE *TrainingSet)
{
	float *in, *des, *out;
	int in_N=0, out_N=0, Examples=0;
 	char str[100];
	for(int i=1; i<=12; i++)
	{
		fscanf(TrainingSet,"%s",str);
	}
	fscanf(TrainingSet,"%d",&in_N);
	fscanf(TrainingSet,"%s",str);
	fscanf(TrainingSet,"%d",&out_N);
	if((in_N == neurons[0]) && (out_N == neurons[layers-1]))
	{
		in = (float*)malloc(in_N*sizeof(float));
		des = (float*)malloc(out_N*sizeof(float));
		fscanf(TrainingSet,"%s",str);
		fscanf(TrainingSet,"%d",&Examples);
		for(int i=0; i<Examples; i++)
		{
			for(int k=0; k<in_N; k++)
			{
				fscanf(TrainingSet,"%f",&in[k]);
			}
			for(int k=0; k<out_N; k++)
			{
				fscanf(TrainingSet,"%f",&des[k]);
			}
			out = Execute(in);
			BackPropagateOnLine(des);
			Errors->ErrorQuadratic(out,des,out_N);
			Errors->Accuracy(out,des,out_N);
		}
		fseek(TrainingSet,0,0);
		return true;
	}
	else
	{
		return false;
	}
}

FILE *ActualTrainingFile;

void NNFpp::TrainingSetFromFile(FILE *TrainingSet, int seed)
{
        float *out;
        if(ActualTrainingFile!=TrainingSet)
        {
                ActualTrainingFile = TrainingSet;
                TD = new TrainingData(TrainingSet);
        }
        TD->Shuffle(seed);
        for(int i=0; i<TD->ExamplesNum; i++)
        {
                out = this->Execute((float*)TD->Data[TD->Sequence[i]].In);
	        BackPropagateOnLine((float*)TD->Data[TD->Sequence[i]].Des);
	        Errors->ErrorQuadratic(out,(float*)TD->Data[TD->Sequence[i]].Des,TD->NeuDes);
	        Errors->Accuracy(out,(float*)TD->Data[TD->Sequence[i]].Des,TD->NeuDes);
        }
}

void NNFpp::TestOnFile(FILE *test, FILE *output)
{
        Errors->ResetValue();
        float *in, *des, *out;
	int in_N=0, out_N=0, Examples=0;
 	char str[100];
	for(int i=1; i<=12; i++)
	{
		fscanf(test,"%s",str);
	}
	fscanf(test,"%d",&in_N);
	fscanf(test,"%s",str);
	fscanf(test,"%d",&out_N);
        in = (float*)malloc(in_N*sizeof(float));
        des = (float*)malloc(out_N*sizeof(float));
        fscanf(test,"%s",str);
        fscanf(test,"%d",&Examples);
        fprintf(output,"%s\t%s\t%s\n","Example","Error","Accuracy");
        for(int i=0; i<Examples; i++)
        {
                for(int k=0; k<in_N; k++)
                {
                        fscanf(test,"%f",&in[k]);
                }
                for(int k=0; k<out_N; k++)
                {
                        fscanf(test,"%f",&des[k]);
                }
                out = Execute(in);
                Errors->ErrorQuadratic(out,des,out_N);
                Errors->Accuracy(out,des,out_N);
                fprintf(output,"%d\t%f\t%d\n",i,Errors->SingleExampleError,Errors->SingleExampleAccuracy);
        }
        fprintf(output,"\nExamples: %d\nCumulative Quadratic Error: %f \nAccuracy on Test Set: %f %% \n",Examples,Errors->CumulativeError,((Errors->CumulativeAccuracy/(float)Examples)*100));
        fseek(test,0,0);
}

//TrainingData---------------------------------------------------------------------------

TrainingData::TrainingData(FILE *f)
{
        char str[100];
	for(int i=1; i<=12; i++)
	{
		fscanf(f,"%s",str);
	}
	fscanf(f,"%d",&NeuIn);
	fscanf(f,"%s",str);
	fscanf(f,"%d",&NeuDes);
        fscanf(f,"%s",str);
        fscanf(f,"%d",&ExamplesNum);

        Sequence = new int [ExamplesNum];

        Data = new Examples [ExamplesNum];
        for(int i=0; i<ExamplesNum; i++)
        {
                Sequence[i] = i;
                Data[i].In = new float [NeuIn];
                Data[i].Des = new float [NeuDes];
        }

        for(int i=0; i<ExamplesNum; i++)
        {
                for(int k=0; k<NeuIn; k++)
                {
                        fscanf(f,"%f",&(Data[i].In[k]));
                }
                for(int k=0; k<NeuDes; k++)
                {
                        fscanf(f,"%f",&(Data[i].Des[k]));
                }
        }
}

void TrainingData::Shuffle(int seed)
{
        srand(time(NULL));
        int a=0, b=0, s=0, g=0;
        s = (seed * (rand()%ExamplesNum) + seed)%ExamplesNum;
        for(int i=0; i<ExamplesNum; i++)
        {
                g = (s + rand())%ExamplesNum;
                a = Sequence[s];
                b = Sequence[g];
                Sequence[g] = a;
                Sequence[s] = b;
                s = (s + i + rand())%ExamplesNum;
        }
}

bool TrainingData::DataIntegrity(FILE *f)
{
        float in=0, des=0;
        int NI=0, ND=0, EN=0, c=0;
        char str[100];
	for(int i=1; i<=12; i++)
	{
		fscanf(f,"%s",str);
	}
	fscanf(f,"%d",&NI);
	fscanf(f,"%s",str);
	fscanf(f,"%d",&ND);
        fscanf(f,"%s",str);
        fscanf(f,"%d",&EN);
        for(int i=0; i<EN; i++)
        {
                for(int k=0; k<NI; k++)
                {
                        fscanf(f,"%f",&in);
                        if(Data[i].In[k]!=in)
                        {
                                c++;
                        }
                }
                for(int k=0; k<ND; k++)
                {
                        fscanf(f,"%f",&des);
                        if(Data[i].Des[k]!=des)
                        {
                                c++;
                        }
                }
        }
        if(c>0)
        {
                return false;
        }
        else
        {
                return true;
        }
}
