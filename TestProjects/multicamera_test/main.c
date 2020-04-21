#include <stdio.h>
#include "camera_dshow.h"
HRESULT callbackForCam1(void* inst, IMediaSample* smp){
    BYTE* pictureBuffer=NULL;
    smp->lpVtbl->GetPointer(smp,&pictureBuffer);
    printf("Cam1: v: %d\n",pictureBuffer[0]);
    return S_OK;
}

HRESULT callbackForCam2(void* inst, IMediaSample* smp){
    BYTE* pictureBuffer=NULL;
    smp->lpVtbl->GetPointer(smp,&pictureBuffer);
    printf("Cam2: v: %d\n",pictureBuffer[0]);
    return S_OK;
}


int main(void){
    unsigned int numOfAvailCams=0;
    struct CameraListItem* CameraLp1=getCameras(&numOfAvailCams);
    struct CameraListItem* CameraLp2=getCameras(&numOfAvailCams);
    printf("Num of avail cam: %u\n",numOfAvailCams);
    struct CameraStorageObject* Cam1ResP=getAvailableCameraResolutions(CameraLp1[0]);
    struct CameraStorageObject* Cam2ResP=getAvailableCameraResolutions(CameraLp2[1]);
    registerCameraCallback(Cam1ResP,0,&callbackForCam1);
    registerCameraCallback(Cam2ResP,0,&callbackForCam2);
    Cam1ResP->_MediaControlP->lpVtbl->Run(Cam1ResP->_MediaControlP);
    Cam2ResP->_MediaControlP->lpVtbl->Run(Cam2ResP->_MediaControlP);

    while(1){

    }
}

