#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "verzerrung.h"

/*float x, y;

int Punktx=30;
int Punkty=10;

//int Randpunkt[8]={0,0,100,50,50,100,100,100};
int Randpunkt[8]={50,-50,0,0,75,50,25,50};
//int Randpunkt[8]={0,0,50,-50,25,50,75,50};
*/


struct CalibData* perspec_calibrating(int CalibPoints[8]){ //CalibPoints contains CalibPoints[0],CalibPoints[1],CalibPoints[2]... (outer field border)
    struct CalibData* CalibDataOut=(struct CalibData*)malloc(sizeof(struct CalibData));
    CalibDataOut->A[0]=CalibPoints[0];
    CalibDataOut->A[1]=CalibPoints[1];
    CalibDataOut->FocusPointsMask=0;

    float u1x, u1y, r1x, r1y, d1, u2x, u2y, r2x, r2y, d2;
    //Calculation F1 and x-Coordinate
    float P1x=CalibPoints[6]+((CalibPoints[4]-CalibPoints[6])*(CalibPoints[2]-CalibPoints[6])+(CalibPoints[5]-CalibPoints[7])*(CalibPoints[3]-CalibPoints[7]))*(CalibPoints[2]-CalibPoints[6])/((CalibPoints[2]-CalibPoints[6])*(CalibPoints[2]-CalibPoints[6])+(CalibPoints[3]-CalibPoints[7])*(CalibPoints[3]-CalibPoints[7]));
    float P1y=CalibPoints[7]+((CalibPoints[4]-CalibPoints[6])*(CalibPoints[2]-CalibPoints[6])+(CalibPoints[5]-CalibPoints[7])*(CalibPoints[3]-CalibPoints[7]))*(CalibPoints[3]-CalibPoints[7])/((CalibPoints[2]-CalibPoints[6])*(CalibPoints[2]-CalibPoints[6])+(CalibPoints[3]-CalibPoints[7])*(CalibPoints[3]-CalibPoints[7]));
    //length between P1 and C
    float L1=sqrt((P1x-CalibPoints[4])*(P1x-CalibPoints[4])+(P1y-CalibPoints[5])*(P1y-CalibPoints[5]));
    float MirrFlag1=((CalibPoints[2]-CalibPoints[4])*(CalibPoints[7]-CalibPoints[1])-(CalibPoints[3]-CalibPoints[5])*(CalibPoints[6]-CalibPoints[0])); //Mirroring correction
    float alph1=atan2(CalibPoints[1]-CalibPoints[5],CalibPoints[0]-CalibPoints[4]); //angle of CA to x-Axyis atan2(cy-ay,cx-ax)
    float alph2=atan2(CalibPoints[3]-CalibPoints[7],CalibPoints[2]-CalibPoints[6]); //angle of DB to x-Axyis atan2(by-dy,bx-dx)
    float alpha=alph2-alph1; //angle between CA and DB which is angle in focus in mathematical defined way
    alpha=alpha-((alpha>M_PI)-(alpha<-M_PI))*2*M_PI;//Map angle to [-PI,PI]
    printf("alpha=%f\n",alpha/3.14159*180);
    printf("%f,%f,%f\n",P1x,P1y,MirrFlag1);
    if(alpha>0.05||alpha<-0.05){ //if focuspoints
        CalibDataOut->FocusPointsMask=1;
        CalibDataOut->AngleOrLength[0]=alpha;
        float Abs1=L1/tan(alpha);
        CalibDataOut->PointsOrVec[0]=P1x-((MirrFlag1>0)-(MirrFlag1<0))*(CalibPoints[2]-CalibPoints[6])/sqrt((CalibPoints[2]-CalibPoints[6])*(CalibPoints[2]-CalibPoints[6])+(CalibPoints[3]-CalibPoints[7])*(CalibPoints[3]-CalibPoints[7]))*Abs1;
        CalibDataOut->PointsOrVec[1]=P1y-((MirrFlag1>0)-(MirrFlag1<0))*(CalibPoints[3]-CalibPoints[7])/sqrt((CalibPoints[2]-CalibPoints[6])*(CalibPoints[2]-CalibPoints[6])+(CalibPoints[3]-CalibPoints[7])*(CalibPoints[3]-CalibPoints[7]))*Abs1;
    }else {
        u1x=(CalibPoints[4]-CalibPoints[0])/sqrt((CalibPoints[4]-CalibPoints[0])*(CalibPoints[4]-CalibPoints[0])+(CalibPoints[5]-CalibPoints[1])*(CalibPoints[5]-CalibPoints[1]));
        u1y=(CalibPoints[5]-CalibPoints[1])/sqrt((CalibPoints[4]-CalibPoints[0])*(CalibPoints[4]-CalibPoints[0])+(CalibPoints[5]-CalibPoints[1])*(CalibPoints[5]-CalibPoints[1]));
        r1x=(CalibPoints[2]*u1x+CalibPoints[3]*u1y-CalibPoints[0]*u1x-CalibPoints[1]*u1y)*u1x;
        r1y=(CalibPoints[2]*u1x+CalibPoints[3]*u1y-CalibPoints[0]*u1x-CalibPoints[1]*u1y)*u1y;
        CalibDataOut->AngleOrLength[0]=sqrt((CalibPoints[0]+r1x-CalibPoints[2])*(CalibPoints[0]+r1x-CalibPoints[2])+(CalibPoints[1]+r1y-CalibPoints[3])*(CalibPoints[1]+r1y-CalibPoints[3]));
        CalibDataOut->PointsOrVec[0]=((MirrFlag1>0)-(MirrFlag1<0))*u1y;
        CalibDataOut->PointsOrVec[1]=-((MirrFlag1>0)-(MirrFlag1<0))*u1x;
    }

    //Calculation F2 and y-Coordinate
    float P2x=CalibPoints[4]+((CalibPoints[0]-CalibPoints[4])*(CalibPoints[6]-CalibPoints[4])+(CalibPoints[1]-CalibPoints[5])*(CalibPoints[7]-CalibPoints[5]))*(CalibPoints[6]-CalibPoints[4])/((CalibPoints[6]-CalibPoints[4])*(CalibPoints[6]-CalibPoints[4])+(CalibPoints[7]-CalibPoints[5])*(CalibPoints[7]-CalibPoints[5]));
    float P2y=CalibPoints[5]+((CalibPoints[0]-CalibPoints[4])*(CalibPoints[6]-CalibPoints[4])+(CalibPoints[1]-CalibPoints[5])*(CalibPoints[7]-CalibPoints[5]))*(CalibPoints[7]-CalibPoints[5])/((CalibPoints[6]-CalibPoints[4])*(CalibPoints[6]-CalibPoints[4])+(CalibPoints[7]-CalibPoints[5])*(CalibPoints[7]-CalibPoints[5]));
    //length between P2 and A
    float L2=sqrt((P2x-CalibPoints[0])*(P2x-CalibPoints[0])+(P2y-CalibPoints[1])*(P2y-CalibPoints[1]));
    float MirrFlag2=((CalibPoints[6]-CalibPoints[0])*(CalibPoints[5]-CalibPoints[3])-(CalibPoints[7]-CalibPoints[1])*(CalibPoints[4]-CalibPoints[2])); //Mirroring correction
    float bet1=atan2(CalibPoints[1]-CalibPoints[3],CalibPoints[0]-CalibPoints[2]); //angle of CA to x-Axyis atan2(cy-ay,cx-ax)
    float bet2=atan2(CalibPoints[5]-CalibPoints[7],CalibPoints[4]-CalibPoints[6]); //angle of DB to x-Axyis atan2(by-dy,bx-dx)
    float beta=bet2-bet1; //angle between BA and DC which is angle in focus in mathematical defined way
    alpha=beta-((beta>M_PI)-(beta<-M_PI))*2*M_PI;//Map angle to [-PI,PI]
    if(beta>0.05||beta<-0.05){
        CalibDataOut->FocusPointsMask+=2;
        CalibDataOut->AngleOrLength[1]=beta;
        float Abs2=L2/tan(beta);
        CalibDataOut->PointsOrVec[2]=P2x-((MirrFlag2>0)-(MirrFlag2<0))*(CalibPoints[6]-CalibPoints[4])/sqrt((CalibPoints[6]-CalibPoints[4])*(CalibPoints[6]-CalibPoints[4])+(CalibPoints[7]-CalibPoints[5])*(CalibPoints[7]-CalibPoints[5]))*Abs2;
        CalibDataOut->PointsOrVec[3]=P2y-((MirrFlag2>0)-(MirrFlag2<0))*(CalibPoints[7]-CalibPoints[5])/sqrt((CalibPoints[6]-CalibPoints[4])*(CalibPoints[6]-CalibPoints[4])+(CalibPoints[7]-CalibPoints[5])*(CalibPoints[7]-CalibPoints[5]))*Abs2;
    }else{
        u2x=(CalibPoints[2]-CalibPoints[0])/sqrt((CalibPoints[2]-CalibPoints[0])*(CalibPoints[2]-CalibPoints[0])+(CalibPoints[3]-CalibPoints[1])*(CalibPoints[3]-CalibPoints[1]));
        u2y=(CalibPoints[3]-CalibPoints[1])/sqrt((CalibPoints[2]-CalibPoints[0])*(CalibPoints[2]-CalibPoints[0])+(CalibPoints[3]-CalibPoints[1])*(CalibPoints[3]-CalibPoints[1]));
        r2x=(CalibPoints[4]*u2x+CalibPoints[5]*u2y-CalibPoints[0]*u2x-CalibPoints[1]*u2y)*u2x;
        r2y=(CalibPoints[4]*u2x+CalibPoints[5]*u2y-CalibPoints[0]*u2x-CalibPoints[1]*u2y)*u2y;
        CalibDataOut->AngleOrLength[1]=sqrt((CalibPoints[0]+r2x-CalibPoints[4])*(CalibPoints[0]+r2x-CalibPoints[4])+(CalibPoints[1]+r2y-CalibPoints[5])*(CalibPoints[1]+r2y-CalibPoints[5]));
        CalibDataOut->PointsOrVec[2]=-((MirrFlag2>0)-(MirrFlag2<0))*u2y;
        CalibDataOut->PointsOrVec[3]=((MirrFlag2>0)-(MirrFlag2<0))*u2x;
    }
    printf("%f,%f,%f,%f\n",CalibDataOut->PointsOrVec[0],CalibDataOut->PointsOrVec[1],CalibDataOut->PointsOrVec[2],CalibDataOut->PointsOrVec[3]);
    return CalibDataOut;
}

float* calculatePosCurs(struct CalibData* CalibDataIn,float Punktx, float Punkty){
    float* resultPoint=(float*)malloc(sizeof(float)*2);
    float NormF1A, NormF2A, NormF1Punkt, NormF2Punkt;
    switch(CalibDataIn->FocusPointsMask){
    case 0:
        printf("Debug: Case no fp\n");
        resultPoint[0]=((Punktx-CalibDataIn->A[0])*CalibDataIn->PointsOrVec[0]+(Punkty-CalibDataIn->A[1])*CalibDataIn->PointsOrVec[1])/CalibDataIn->AngleOrLength[0];
        resultPoint[1]=((Punktx-CalibDataIn->A[0])*CalibDataIn->PointsOrVec[2]+(Punkty-CalibDataIn->A[1])*CalibDataIn->PointsOrVec[3])/CalibDataIn->AngleOrLength[1];
        break;
    case 1:
        resultPoint[1]=((Punktx-CalibDataIn->A[0])*CalibDataIn->PointsOrVec[2]+(Punkty-CalibDataIn->A[1])*CalibDataIn->PointsOrVec[3])/CalibDataIn->AngleOrLength[1];
        {
            //X calc
            float gamma1=atan2(Punkty-CalibDataIn->PointsOrVec[1],Punktx-CalibDataIn->PointsOrVec[0]); //angle of F1P to x-Axyis atan2(py-f1y,px-f1x)
            float gamma2=atan2(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[1],CalibDataIn->A[0]-CalibDataIn->PointsOrVec[0]); //angle of F1 to x-Axyis atan2(ay-f1y,ax-f1x)
            float gamma=gamma1-gamma2; //angle between CA and DB which is angle in focus in mathematical defined way
            gamma=gamma-((gamma>M_PI)-(gamma<-M_PI))*2*M_PI;//Map angle to [-PI,PI]
            resultPoint[0]=gamma/CalibDataIn->AngleOrLength[0];
        }
        /*
        printf("Debug: Case first fp\n");
        printf("%f,%f\n",CalibDataIn->AngleOrLength[0],CalibDataIn->AngleOrLength[1]);
        NormF1A=    sqrt((CalibDataIn->A[0]-CalibDataIn->PointsOrVec[0])*(CalibDataIn->A[0]-CalibDataIn->PointsOrVec[0])+(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[1])*(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[1]));
        NormF1Punkt=sqrt((Punktx-CalibDataIn->PointsOrVec[0])*(Punktx-CalibDataIn->PointsOrVec[0])+(Punkty-CalibDataIn->PointsOrVec[1])*(Punkty-CalibDataIn->PointsOrVec[1]));
        resultPoint[0]=asin(((CalibDataIn->A[0]-CalibDataIn->PointsOrVec[0])*(Punkty-CalibDataIn->PointsOrVec[1])-(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[1])*(Punktx-CalibDataIn->PointsOrVec[0]))/NormF1A/NormF1Punkt)/CalibDataIn->AngleOrLength[0];
        resultPoint[1]=((Punktx-CalibDataIn->A[0])*CalibDataIn->PointsOrVec[2]+(Punkty-CalibDataIn->A[1])*CalibDataIn->PointsOrVec[3])/CalibDataIn->AngleOrLength[1];
        */
        printf("case1\n");
        break;
    case 2:
        {
            resultPoint[0]=((Punktx-CalibDataIn->A[0])*CalibDataIn->PointsOrVec[0]+(Punkty-CalibDataIn->A[1])*CalibDataIn->PointsOrVec[1])/CalibDataIn->AngleOrLength[0];
            //Y calc
            float delta1=atan2(Punkty-CalibDataIn->PointsOrVec[3],Punktx-CalibDataIn->PointsOrVec[2]); //angle of F1P to x-Axyis atan2(py-f1y,px-f1x)
            float delta2=atan2(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[3],CalibDataIn->A[0]-CalibDataIn->PointsOrVec[2]); //angle of F1 to x-Axyis atan2(ay-f1y,ax-f1x)
            float delta=delta1-delta2; //angle between CA and DB which is angle in focus in mathematical defined way
            delta=delta-((delta>M_PI)-(delta<-M_PI))*2*M_PI;//Map angle to [-PI,PI]
            resultPoint[1]=delta/CalibDataIn->AngleOrLength[1];
            printf("case2\n");
        }
        break;
    case 3:
        printf("Debug: Case two fp\n");
        printf("%f,%f\n",CalibDataIn->AngleOrLength[0],CalibDataIn->AngleOrLength[1]);


        /*NormF1A=    sqrt((CalibDataIn->A[0]-CalibDataIn->PointsOrVec[0])*(CalibDataIn->A[0]-CalibDataIn->PointsOrVec[0])+(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[1])*(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[1]));
        NormF1Punkt=sqrt((Punktx-CalibDataIn->PointsOrVec[0])*(Punktx-CalibDataIn->PointsOrVec[0])+(Punkty-CalibDataIn->PointsOrVec[1])*(Punkty-CalibDataIn->PointsOrVec[1]));
        NormF2A=    sqrt((CalibDataIn->A[0]-CalibDataIn->PointsOrVec[2])*(CalibDataIn->A[0]-CalibDataIn->PointsOrVec[2])+(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[3])*(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[3]));
        NormF2Punkt=sqrt((Punktx-CalibDataIn->PointsOrVec[2])*(Punktx-CalibDataIn->PointsOrVec[2])+(Punkty-CalibDataIn->PointsOrVec[3])*(Punkty-CalibDataIn->PointsOrVec[3]));
        resultPoint[0]=asin(((CalibDataIn->A[0]-CalibDataIn->PointsOrVec[0])*(Punkty-CalibDataIn->PointsOrVec[1])-(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[1])*(Punktx-CalibDataIn->PointsOrVec[0]))/NormF1A/NormF1Punkt)/CalibDataIn->AngleOrLength[0];
        resultPoint[1]=asin(((CalibDataIn->A[0]-CalibDataIn->PointsOrVec[2])*(Punkty-CalibDataIn->PointsOrVec[3])-(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[3])*(Punktx-CalibDataIn->PointsOrVec[2]))/NormF2A/NormF2Punkt)/CalibDataIn->AngleOrLength[1];
        */
        //X calc
        float gamma1=atan2(Punkty-CalibDataIn->PointsOrVec[1],Punktx-CalibDataIn->PointsOrVec[0]); //angle of F1P to x-Axyis atan2(py-f1y,px-f1x)
        float gamma2=atan2(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[1],CalibDataIn->A[0]-CalibDataIn->PointsOrVec[0]); //angle of F1 to x-Axyis atan2(ay-f1y,ax-f1x)
        float gamma=gamma1-gamma2; //angle between CA and DB which is angle in focus in mathematical defined way
        gamma=gamma-((gamma>M_PI)-(gamma<-M_PI))*2*M_PI;//Map angle to [-PI,PI]
        resultPoint[0]=gamma/CalibDataIn->AngleOrLength[0];
        //Y calc
        float delta1=atan2(Punkty-CalibDataIn->PointsOrVec[3],Punktx-CalibDataIn->PointsOrVec[2]); //angle of F1P to x-Axyis atan2(py-f1y,px-f1x)
        float delta2=atan2(CalibDataIn->A[1]-CalibDataIn->PointsOrVec[3],CalibDataIn->A[0]-CalibDataIn->PointsOrVec[2]); //angle of F1 to x-Axyis atan2(ay-f1y,ax-f1x)
        float delta=delta1-delta2; //angle between CA and DB which is angle in focus in mathematical defined way
        delta=delta-((delta>M_PI)-(delta<-M_PI))*2*M_PI;//Map angle to [-PI,PI]
        resultPoint[1]=delta/CalibDataIn->AngleOrLength[1];
        printf("gamma, delta, %f, %f",gamma,delta);
        break;
    default:
        break;
    }
    return resultPoint;
}
