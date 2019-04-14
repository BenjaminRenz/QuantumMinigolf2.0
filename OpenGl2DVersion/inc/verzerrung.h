#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif
struct CalibData{
    int FocusPointsMask;
    float PointsOrVec[4]; //Stores focus points or Normalized Vectors
    int A[2];
    float AngleOrLength[2];
};
struct CalibData* perspec_calibrating(int CalibPoints[8]);

float* calculatePosCurs(struct CalibData* CalibDataIn,float Punktx, float Punkty);
