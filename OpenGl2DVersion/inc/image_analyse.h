#ifndef IMAGE_ANALYSE_C_INCLUDED
#define IMAGE_ANALYSE_C_INCLUDED

#include "camera_dshow.h"
#define camera_big_grid_points_x 300 //The y gridpoints will be calculated based on the aspect ratio
#define camera_testspots 1

HRESULT callbackForGraphview(void* inst, IMediaSample* smp); //when first called will set smp==0 and recieve a inputForBrightspotfinder Sruct to sore camera resolution and pointer for bright spot coordinates

enum {brightspot_get,brightspot_init,brightspot_deinit};
int* getBrightspot(int brightspot_mode);

#define cameraNum 1
#define cameraResNum 0

IMediaControl* getPositionPointer(volatile int* Posx,volatile int* Posy);
#endif //IMAGE_ANALYSE_C_INCLUDED
