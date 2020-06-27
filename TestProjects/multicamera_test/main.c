#include <stdio.h>
#include "camera_dshow.h"

HRESULT callbackForCam1(struct SampleGrabberCB_iface* this,double SampleTime, IMediaSample* smp){
    BYTE* pictureBuffer=NULL;
    smp->lpVtbl->GetPointer(smp,&pictureBuffer);
    printf(" C1, Sample at T=%f, px0=%d\n",SampleTime,pictureBuffer[0]);
    return S_OK;
}

HRESULT callbackForCam2(struct SampleGrabberCB_iface* this,double SampleTime, IMediaSample* smp){
    BYTE* pictureBuffer=NULL;
    smp->lpVtbl->GetPointer(smp,&pictureBuffer);
    printf("C2, Sample at T=%f, px0=%d\n",SampleTime,pictureBuffer[0]);
    return S_OK;
}

//finds brightest spot
void findBrightspotReplace(uint16_t* return_brightspot_xp, uint16_t* return_brightspot_yp, uint16_t* return_brightspot_bp, uint16_t xmin, uint16_t xmax, uint16_t ymin, uint16_t ymax, uint8_t* pxData, uint16_t resolutionX){
    *return_brightspot_xp=0;
    *return_brightspot_yp=0;
    *return_brightspot_bp=0;
    for(uint16_t current_x=xmin;current_x<=xmax;current_x++){
        for(uint16_t current_y=ymin;current_y<=ymax;current_y++){
            uint32_t pixelOffset=3*(current_x+current_y*resolutionX);
            uint16_t brightness=PixelData[pixelOffset]+PixelData[pixelOffset+1]+PixelData[pixelOffset+2];
            if(*return_brightspot_bp<brightness){
                //update best_brightspot
                *return_brightspot_bp=brightness;
                *return_brightspot_xp=current_x;
                *return_brightspot_yp=current_y;
            }
        }
    }
    return;
}

//finds maximum bright row and maximum bright column
void findBrightspotSumRC(uint16_t* return_brightspot_xp, uint16_t* return_brightspot_yp, uint16_t* return_brightspot_bp, uint16_t xmin, uint16_t xmax, uint16_t ymin, uint16_t ymax, uint8_t* PixelDatap, uint16_t resolutionX){
    //find best column
    uint32_t best_brightness_sum=0;
    for(uint16_t current_x=xmin;current_x<=xmax;current_x++){
        uint32_t brightness_col_sum=0;
        for(uint16_t current_y=ymin;current_y<=ymax;current_y++){
            uint32_t pixelOffset=3*(current_x+current_y*resolutionX);
            brightness_col_sum+=PixelDatap[pixelOffset]+PixelDatap[pixelOffset+1]+PixelDatap[pixelOffset+2];
        }
        if(best_brightness_sum<brightness_col_sum){
            best_brightness_sum=brightness_col_sum;
            *return_brightspot_xp=current_x;
        }
    }
    for(uint16_t current_y=ymin;current_y<=ymax;current_y++){
        uint32_t brightness_row_sum=0;
        for(uint16_t current_x=xmin;current_x<=xmax;current_x++){
            uint32_t pixelOffset=3*(current_x+current_y*resolutionX);
            brightness_row_sum+=PixelDatap[pixelOffset]+PixelDatap[pixelOffset+1]+PixelDatap[pixelOffset+2];
        }
        if(best_brightness_sum<brightness_row_sum){
            best_brightness_sum=brightness_row_sum;
            *return_brightspot_yp=current_y;
        }
    }
    uint32_t pixelOffset=3*((*return_brightspot_xp)+(*return_brightspot_yp)*resolutionX);
    return_brightspot_bp=PixelDatap[pixelOffset]+PixelDatap[pixelOffset+1]+PixelDatap[pixelOffset+2];
    return;
}

void cam_predictor_close_search_thread(void* init){
    uint16_t resolution_x;
    uint16_t resolution_y;
    uint16_t searchsize;
    static uint16_t last_x_pos;
    static uint16_t last_y_pos;
    static uint16_t last_x_velocity;
    static uint16_t last_y_velocity;

    uint16_t new_x_pos=last_x_pos+last_x_velocity;
    uint16_t new_y_pos=last_y_pos+last_y_velocity;

    uint8_t* PixelDatap;
    uint16_t brightness=0;
    //make sure not to clip image
    uint16_t search_xmin=max(0,new_x_pos-searchsize);
    uint16_t search_xmax=min(resolution_x-1,new_x_pos+searchsize);
    uint16_t search_ymin=max(0,new_y_pos-searchsize);
    uint16_t search_ymax=min(resolution_y-1,new_y_pos+searchsize);
    findBrightspotReplace(&new_x_pos,&new_y_pos,&brightness,search_xmin,search_xmax,search_ymin,search_ymax,PixelDatap,resolution_x);
    last_x_velocity=new_x_pos-last_x_pos;
    last_y_velocity=new_y_pos-last_y_pos;
    last_x_pos=new_x_pos;
    last_y_pos=new_y_pos;
    //Todo lock mutex when writing new position
}

int main(void){
    unsigned int numOfAvailCams=0;
    struct CameraListItem* CameraLp=getCameras(&numOfAvailCams);
    printf("Num of avail cam: %u\n",numOfAvailCams);
    struct CameraStorageObject* Cam1ResP=getAvailableCameraResolutions(CameraLp[0]);
    struct CameraStorageObject* Cam2ResP=getAvailableCameraResolutions(CameraLp[1]);
    registerCameraCallback(Cam1ResP,0,&callbackForCam1);
    registerCameraCallback(Cam2ResP,0,&callbackForCam2);
    printf("Regitered Callbacks, moving on.\n");
    Cam1ResP->_MediaControlP->lpVtbl->Run(Cam1ResP->_MediaControlP);
    Cam2ResP->_MediaControlP->lpVtbl->Run(Cam2ResP->_MediaControlP);

    //TODO function callback needs fixing see;
    //https://docs.microsoft.com/de-de/office/client-developer/outlook/mapi/implementing-iunknown-in-c
    //https://www.c-plusplus.net/forum/topic/284559/isamplegrabber-setcallback-eigene-funktion-%C3%BCbergeben/6
    //https://stackoverflow.com/questions/16562479/isamplegrabber-callback-method-code-works-but-might-need-some-love
    //https://docs.microsoft.com/en-us/windows/win32/directshow/isamplegrabber-setcallback
    //http://doc.51windows.net/Directx9_SDK/htm/isamplegrabbercbinterface.htm

	//SaveGraphFile(Cam1ResP->_GraphP,L".\\graph.grf");

    while(1){

    }
}

