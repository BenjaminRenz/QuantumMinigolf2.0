#include <stdio.h>
#include "camera_dshow.h"

long callbackForCam1(double SampleTime,unsigned char *pBuffer,long BufferLen){
    printf("Cam1: v: %x,%x,%lf\n",pBuffer[0],BufferLen,SampleTime);
    return S_OK;
}

HRESULT callbackForCam2(struct SampleGrabberCB_iface* this,double SampleTime, IMediaSample* smp){
    BYTE* pictureBuffer=NULL;
    smp->lpVtbl->GetPointer(smp,&pictureBuffer);
    printf("Cam2: v: %d\n",pictureBuffer[0]);
    return S_OK;
}


int main(void){
    unsigned int numOfAvailCams=0;
    struct CameraListItem* CameraLp1=getCameras(&numOfAvailCams);
    //struct CameraListItem* CameraLp2=getCameras(&numOfAvailCams);
    printf("Num of avail cam: %u\n",numOfAvailCams);
    struct CameraStorageObject* Cam1ResP=getAvailableCameraResolutions(CameraLp1[0]);
    //struct CameraStorageObject* Cam2ResP=getAvailableCameraResolutions(CameraLp2[1]);
    registerCameraCallback(Cam1ResP,0,&callbackForCam2);
    //registerCameraCallback(Cam2ResP,0,&callbackForCam2);
    printf("Here\n");
    Cam1ResP->_MediaControlP->lpVtbl->Run(Cam1ResP->_MediaControlP);

    //TODO function callback needs fixing see;
    //https://docs.microsoft.com/de-de/office/client-developer/outlook/mapi/implementing-iunknown-in-c
    //https://www.c-plusplus.net/forum/topic/284559/isamplegrabber-setcallback-eigene-funktion-%C3%BCbergeben/6
    //https://stackoverflow.com/questions/16562479/isamplegrabber-callback-method-code-works-but-might-need-some-love
    //https://docs.microsoft.com/en-us/windows/win32/directshow/isamplegrabber-setcallback
    //http://doc.51windows.net/Directx9_SDK/htm/isamplegrabbercbinterface.htm

    Sleep(10);

	SaveGraphFile(Cam1ResP->_GraphP,L".\\graph.grf");
	Cam1ResP->_MediaControlP->lpVtbl->Run(Cam1ResP->_MediaControlP);

    while(1){

    }
}

