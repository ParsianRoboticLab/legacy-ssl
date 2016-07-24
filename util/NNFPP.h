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

#ifndef NNFppH
#define NNFppH

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define  NNF_FUNC_SIGMA_LOGISTIC(x) ( 1 / (1 + exp(-(x))) )
#define  NNF_FUNC_SIGMA_HYPERBOLIC(x) ( tanh((x)) )
#define  NNF_FUNC_STEP(x,alfa)  ( (x) >= (alfa) ? 1 : 0 )
#define  NNF_FUNC_SIGN(x)  ( (x) >= 0 ? 1 : -1 )
#define  NNF_FUNC_SIMIL_SIGMOID(x,alfa) ((0.5*x)/(alfa+abs(x))+0.5)
#define  NNF_FUNC_SINC(x) (float)sin(x)/x

enum {
	NNF_SIGMA_LOGISTIC,
	NNF_SIGMA_HYPERBOLIC,
	NNF_STEP,
	NNF_SIGN,
	NNF_SIMIL_SIGMOID, //for alpha = 0 f(x)= sign(x); f'(x) = (f(x)-0.5)*(|f(x)-0.5|+0.5)
	NNF_SINC
};
//---------------------------------------------------------------------------
class Error
{
        public: float SingleExampleError;
        public: float CumulativeError;
        public: int SingleExampleAccuracy;
        public: int CumulativeAccuracy;
        public: float Treshold;

        public: Error()
        {
                SingleExampleError = 0;
                CumulativeError = 0;
                SingleExampleAccuracy = 0;
                CumulativeAccuracy = 0;
                Treshold = 0.5;
        };

        public: void ErrorQuadratic(float Out[], float Des[], int Dim)
        {
                SingleExampleError = 0;
                for(int i=0; i<Dim; i++)
                {
                        SingleExampleError += (float)((Des[i]-Out[i])*(Des[i]-Out[i]))/2;
                }
                CumulativeError += SingleExampleError;
        };

        public: void Accuracy(float Out[], float Des[], int Dim)
        {
                int a=0;
                bool good=true;
                SingleExampleAccuracy = 0;
                for(int i=0; i<Dim; i++)
                {
                        if(Out[i]>Treshold)
                        {
                                a=1;
                        }
                        else
                        {
                                a=0;
                        }
                        if(a!=Des[i])
                        {
                                good = false;
                        }
                }
                if(good)
                {
                        SingleExampleAccuracy = 1;
                }
                CumulativeAccuracy += SingleExampleAccuracy;
        };

        public: void ResetValue(void)
        {
                SingleExampleError = 0;
                CumulativeError = 0;
                SingleExampleAccuracy = 0;
                CumulativeAccuracy = 0;
                Treshold = 0.5;
        };
};
//---------------------------------------------------------------------------
class Examples
{
        public: float *In;
        public: float *Des;
};
//---------------------------------------------------------------------------
class TrainingData
{
        public: int NeuIn;
        public: int NeuDes;
        public: int ExamplesNum;

        public: int *Sequence;

        public: Examples *Data;

        public: TrainingData(FILE *f);
        public: void Shuffle(int seed);
        public: bool DataIntegrity(FILE *f);
};
//---------------------------------------------------------------------------
class NNFpp
{
        public: int layers; //layers of the net
        public: int *neurons; //neurons in each layer
        public: float epsilon; //learning constant
        public: int *activationFunction; //chosen activation function
        public: float *alfa; //only for function with parameter
        public: float ***weight; //weight of the synapse between two neurons
        public: float **bias; //bias weight of each neuron
        public: float **value; //values for each neuron
        public: float **delta; //delta for each neuron
                private: int InitType; //used to reset the net
                private: FILE *FileForReset; //remember the file to load to reset

        public: Error *Errors; //Errors
        public: TrainingData *TD; // a container for the training set

        public: NNFpp(int layrs, int neurs[], int range, int A_Function, float Epsilon); //Constructor
        public: NNFpp(int layrs, int neurs[], int range, int A_Function, float FunctionParameter, float Epsilon); //Constructor for single function with parameter
        public: NNFpp(int layrs, int neurs[], int range, int A_Function[], float FunctionParameter[], float Epsilon); //Constructor for multiple function with parameters
        public: NNFpp(FILE *NNFfile); //Constructor from file
        public: ~NNFpp(void); //Destructor

        public: void LoadNet(FILE *loadFile); //Load Neural Network from file
        public: void SaveNet(FILE *saveFile); //Save Neural Network to file

        public: void ResetNet(int range); //Reset Neural Network
        public: float* Execute(float input[]); //Execute Neural Network
        public: void BackPropagateOnLine(float desired[]); //Backpropagation
                private: float LearningBP(int i, float error, int l); //used by backpropagation

        public: bool TrainingSetFromFile(FILE *TrainingSet); //Make training from file data
        public: void TrainingSetFromFile(FILE *TrainingSet, int seed); //Make shuffle training from file data
        public: void TestOnFile(FILE *test, FILE *output); // Make learning test
};
//---------------------------------------------------------------------------
#endif
