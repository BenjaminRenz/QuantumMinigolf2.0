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
    struct CameraListItem* CameraLp=getCameras(&numOfAvailCams);
    printf("Num of avail cam: %u\n",numOfAvailCams);
    struct CameraStorageObject* Cam1ResP=getAvailableCameraResolutions(CameraLp[0]);
    struct CameraStorageObject* Cam2ResP=getAvailableCameraResolutions(CameraLp[1]);

        printf("Here42\n");
        registerCameraCallback(Cam1ResP,0,&callbackForCam1);
    registerCameraCallback(Cam2ResP,0,&callbackForCam2);
    printf("Here\n");
    Cam1ResP->_MediaControlP->lpVtbl->Run(Cam1ResP->_MediaControlP);

    Sleep(10);
Cam2ResP->_MediaControlP->lpVtbl->Run(Cam2ResP->_MediaControlP);
    while(1){

    }
}

