#ifndef CAMERA_DSHOW_INCLUDED
#define CAMERA_DSHOW_INCLUDED


#include <dshow.h>
#include <guiddef.h>
#include <winnt.h>  //for InterlockedIncrement/Decrement
#include <qedit.h>  //For sample grabber
#include <stdio.h>
#include <strmif.h>
#include <uchar.h>
#include <stddef.h>

struct inputForBrightspotfinder
{
    int* cam_current_xpos; //used from main to get values out of other thread
    int* cam_current_ypos;
    int xres;
    int yres;
};

struct CameraListItem
{
    IMoniker* MonikerPointer;
    char32_t friendlyName[30];
    char32_t devicePath[200];
};

struct CameraStorageObject
{
    IMoniker* MonikerP;
    char32_t friendlyName[30];
    char32_t devicePath[200];
    IGraphBuilder* _GraphP;
    IMediaControl* _MediaControlP;
    IAMCameraControl* _CameraControlP;
    unsigned int numberOfSupportedResolutions; //(maximum value of resolutionNum)+1, see one line below
    unsigned int** resolutionsXYarrayP;  //treat as if it would be a 2d array e.g.: resolutionsXYPointer[resolutionNum][0] for width ... [width=0,height=1]
    AM_MEDIA_TYPE** _amMediaArrayP;
    IAMStreamConfig* _StreamCfgP;
    IBaseFilter* _camFilterP;
};

/* This function shall be called with a NULL pointer to initialize and return all available cameras as structs. The user then should pick one camera and
deallocate other cameras witch have not been selected*/
void closeCamera(struct CameraStorageObject* Camera);
HRESULT callbackForGraphview(void* inst, IMediaSample* smp);
HRESULT (*callbackForGraphviewFPointer)(void* inst, IMediaSample* smp); //create a function pointer which we will to inject our custom function into the RenderPinObject

/** \brief call this before any other function, gets available cameras to the system
 *
 * \param pointer to an int in which the number of connected cameras is stored
 * \return pointer to a list of <numberOfCameras> CameraListItems
 *
 */
struct CameraListItem* getCameras(unsigned int* numberOfCameras);


struct CameraStorageObject* getAvailableCameraResolutions(struct CameraListItem CameraIn);
int registerCameraCallback(struct CameraStorageObject* CameraIn,int selectedResolution,long (*CBFunp)(double SampleTime,unsigned char *pBuffer,long BufferLen));  //selected resolution is position in array
#endif
