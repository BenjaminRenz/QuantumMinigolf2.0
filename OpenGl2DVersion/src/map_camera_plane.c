#include <stdio.h>
#include <math.h>
#include "map_camera_plane.h"

// Test Inputs
//float Randpunkt[8]={10.0f,5.0f,100.0f,50.0f,100.0f,100.0f,50.0f,100.0f};
//float Randpunkt[8]={50,-50,0,0,75,50,25,50};
//float Randpunkt[8]={0,0,50,-50,25,50,75,50};



void camera_perspec_calibrating(mat3x3 CalibMatrixOut,float* CalibPointsIn){
    //Get Vectors of border points
    /*vec3 PointA={(float)CalibPoints[0],(float)CalibPoints[1],1.0f};
    vec3 PointB={(float)CalibPoints[2],(float)CalibPoints[3],1.0f};
    vec3 PointC={(float)CalibPoints[4],(float)CalibPoints[5],1.0f};
    vec3 PointD={(float)CalibPoints[6],(float)CalibPoints[7],1.0f};
    */

    vec3 PointA={CalibPointsIn[0],CalibPointsIn[1],1.0f};
    vec3 PointB={CalibPointsIn[2],CalibPointsIn[3],1.0f};
    vec3 PointC={CalibPointsIn[4],CalibPointsIn[5],1.0f};
    vec3 PointD={CalibPointsIn[6],CalibPointsIn[7],1.0f};
    printf("Debug: Point A info %f, %f\n",PointA[0],PointA[1]);
    printf("Debug: Point B info %f, %f\n",PointB[0],PointB[1]);
    printf("Debug: Point C info %f, %f\n",PointC[0],PointC[1]);
    printf("Debug: Point D info %f, %f\n",PointD[0],PointD[1]);
    //Step 1: translate four-sided figure, so that PointA lands on the origin
    mat3x3 translationMatrix;
    mat3x3_translate(translationMatrix,-PointA[0],-PointA[1]);
    //Calculate the new positions of B
    vec3 PointB_translated;
    mat3x3_mul_vec3(PointB_translated,translationMatrix,PointB);

    //Step 2: rotate so that Side AB falls on the x-Axis
    mat3x3 translationRotationMatrix;
    float angle=atan2(PointB_translated[1],PointB_translated[0]); //Get angle of point B
    mat3x3_rotate_delayed_Z(translationRotationMatrix,translationMatrix,-angle); //Delayed function first applies translationMatrix and after that rotates
    //Calculate the new positions of B,D
    vec3 PointB_translated_rotated;
    vec3 PointD_translated_rotated;
    mat3x3_mul_vec3(PointB_translated_rotated,translationRotationMatrix,PointB);
    mat3x3_mul_vec3(PointD_translated_rotated,translationRotationMatrix,PointD);

    //Step 3: Shear Side AD so that it lands on the y-Axis DOES NOT WORK YET
    mat3x3 translationRotationShearScaleMatrix;
    mat3x3 shearScaleMatrix=
    {{ 1.f/PointB_translated_rotated[0]                                                             ,0.f                                ,0.f
    },{-PointD_translated_rotated[0]/(PointD_translated_rotated[1]*PointB_translated_rotated[0])    ,1.f/PointD_translated_rotated[1]   ,0.f
    },{0.f                                                                                          ,0.0f                               ,1.f
    }};
    mat3x3_mul(translationRotationShearScaleMatrix,shearScaleMatrix,translationRotationMatrix);
    //Calculate the new position of C
    vec3 PointC_translated_rotated_sheared_scaled;
    mat3x3_mul_vec3(PointC_translated_rotated_sheared_scaled,translationRotationShearScaleMatrix,PointC);

    //Step 4: Move point C to (1,1) and store final result
    float Cx=PointC_translated_rotated_sheared_scaled[0];
    float Cy=PointC_translated_rotated_sheared_scaled[1];
    mat3x3 mapPointCMatrix=
    { {(Cx+Cy-1)/Cx,0.f,(Cy-1)/Cx
    },{0.f,(Cx+Cy-1)/Cy,(Cx-1)/Cy
    },{0.f,0.f,1.f
    }};
    mat3x3_mul(CalibMatrixOut,mapPointCMatrix,translationRotationShearScaleMatrix);

    //Final Test
    /*vec3 testPoint1;
    vec3 testPoint2;
    vec3 testPoint3;
    vec3 testPoint4;
    mat3x3_mul_vec3(testPoint1,CalibMatrixOut,PointA);
    mat3x3_mul_vec3(testPoint2,CalibMatrixOut,PointB);
    mat3x3_mul_vec3(testPoint3,CalibMatrixOut,PointC);
    mat3x3_mul_vec3(testPoint4,CalibMatrixOut,PointD);
    printf("Debug: Point A final info %f, %f, %f\n",testPoint1[0]/testPoint1[2],testPoint1[1]/testPoint1[2]);
    printf("Debug: Point B final info %f, %f, %f\n",testPoint2[0]/testPoint2[2],testPoint2[1]/testPoint2[2]);
    printf("Debug: Point C final info %f, %f, %f\n",testPoint3[0]/testPoint3[2],testPoint3[1]/testPoint3[2]);
    printf("Debug: Point D final info %f, %f, %f\n",testPoint4[0]/testPoint4[2],testPoint4[1]/testPoint4[2]);
    */

}
void camera_perspec_map_point(vec2 MappedVecOut,mat3x3 CalibrationMatrixIn,vec2 CameraBrightSpotIn){
    vec3 BrightSpot={CameraBrightSpotIn[0],CameraBrightSpotIn[1],1.f};
    vec3 resultVec3;
    mat3x3_mul_vec3(resultVec3,CalibrationMatrixIn,BrightSpot);
    MappedVecOut[0]=resultVec3[0]/resultVec3[2];
    MappedVecOut[1]=resultVec3[1]/resultVec3[2];
}
