#include "image_analyse.h"

HRESULT callbackForGraphview(void* inst, IMediaSample* smp) //when first called will set smp==0 and recieve a inputForBrightspotfinder Sruct to sore camera resolution and pointer for bright spot coordinates
{
    static int xres; //since we don't have controll when this callback will occure it will be initialised before giving this functionpointer to directshow to get some values passed
    static int yres;
    static int* current_xpos;
    static int* current_ypos;
    /*static int xsteps;
    static int ysteps;*/
    static int xstepwidth;
    static int ystepwidth;
    if(smp==NULL)  //Recieve initialisation not normal operation
    {
        xres=((struct inputForBrightspotfinder*)inst)->xres;
        yres=((struct inputForBrightspotfinder*)inst)->yres;

        current_xpos=((struct inputForBrightspotfinder*)inst)->cam_current_xpos;
        current_ypos=((struct inputForBrightspotfinder*)inst)->cam_current_ypos;
        //printf("Got initialize add %d,%d, val %d,%d\n",(int)current_xpos,(int)current_ypos,*current_xpos,*current_ypos);
        /*xsteps=camera_big_grid_points_x;
        ysteps=(int)(camera_big_grid_points_x/(((float)xres)/yres));
        */
        //get aspect ratio in float and calculate the int value for the numer of grid points in y direction
        xstepwidth=xres/camera_big_grid_points_x;
        ystepwidth=yres/((int)(camera_big_grid_points_x/(((float)xres)/yres)));
        //printf("Debug: xres: %d, yres %d\n",xres,yres);
        //printf("Debug: xsteps: %d, ysetps %d\n",xsteps,ysteps);
        //printf("Debug: xsteps: %d, ysetps %d\n",xstepwidth,ystepwidth);
        return S_OK;
    }
    else
    {
        //printf("Debug: Frame analyse started \\/ \\/ \\/ \\/ \n");
        int bitperPixel=smp->lpVtbl->GetActualDataLength(smp)/(xres*yres);
        BYTE* pictureBuffer=NULL;
        smp->lpVtbl->GetPointer(smp,&pictureBuffer);
        //printf("DEBUG: %d",pictureBuffer[3]);
        //Rough search for five brightest spots with a big grid
        int* BrightSpots=(int*)malloc(2*sizeof(int)*camera_testspots);
        memset(BrightSpots,0,2*sizeof(int)*camera_testspots);
        for(int y_pos=0;y_pos<yres;y_pos+=ystepwidth){
            for(int x_pos=0;x_pos<xres;x_pos+=xstepwidth){
                int arrayposWithoutRGB=(x_pos*yres+y_pos);
                int birghtness=pictureBuffer[arrayposWithoutRGB*bitperPixel+1];
                if(birghtness>BrightSpots[2*(camera_testspots-1)+1]){ //check if brighter then the least bright spot
                    int i=0; //non local offset for bright spot array
                    for(;i<(camera_testspots-1);i++){ //compare to the other four brighter spots (will from this point on definitively override an existing brightspot)
                        if(birghtness>BrightSpots[2*((camera_testspots-1)-i)+1]){
                            BrightSpots[2*((camera_testspots-1)-i)]=BrightSpots[2*((camera_testspots-2)-i)];//shift less brighter spots down in the array
                            BrightSpots[2*((camera_testspots-1)-i)+1]=BrightSpots[2*((camera_testspots-2)-i)+1];//including its intensity val
                        }else{
                            break;
                        }
                    }
                    BrightSpots[2*((camera_testspots-1)-i)]=arrayposWithoutRGB;
                    BrightSpots[2*((camera_testspots-1)-i)+1]=birghtness;
                }
            }

        }

        //printf("Image thread: %d\t%d\t%d\n",BrightSpots[0]%xres,BrightSpots[0]/xres,BrightSpots[1]);

        /*int* BrightSpots=(int*)malloc(2*sizeof(int)*camera_testspots); //first val is coordinate as x_coord+y_coord*xres, second is brightness val, 0 index is the brightest, 1 index the second brightest and so on
        memset(BrightSpots,0,2*sizeof(int)*camera_testspots);
        for(int y_pos=0,y_counter=0;y_pos<yres;y_pos=((yres*++y_counter)/ysteps)){ //Loop over picture array with larger grid and find 5 brightest spots
            for(int x_pos=0,x_counter=0;x_pos<xres;x_pos=((xres*++x_counter)/xsteps)){
                int arrayposWithoutRGB=(x_pos*yres+y_pos);
                int birghtness=pictureBuffer[arrayposWithoutRGB*bitperPixel]+pictureBuffer[arrayposWithoutRGB*bitperPixel+1]+pictureBuffer[arrayposWithoutRGB*bitperPixel+2];
                if(birghtness>BrightSpots[2*(camera_testspots-1)+1]){ //check if brighter then the least bright spot
                    int i=0; //non local offset for bright spot array
                    for(;i<(camera_testspots-1);i++){ //compare to the other four brighter spots (will from this point on definitively override an existing brightspot)
                        if(birghtness>BrightSpots[2*((camera_testspots-1)-i)+1]){
                            BrightSpots[2*((camera_testspots-1)-i)]=BrightSpots[2*((camera_testspots-2)-i)];//shift less brighter spots down in the array
                            BrightSpots[2*((camera_testspots-1)-i)+1]=BrightSpots[2*((camera_testspots-2)-i)+1];//including its intensity val
                        }else{
                            break;
                        }
                    }
                    BrightSpots[2*((camera_testspots-1)-i)]=arrayposWithoutRGB;
                    BrightSpots[2*((camera_testspots-1)-i)+1]=birghtness;
                }
            }
        }*/
        /*printf("Debug: ");
        for(int i=0;i<camera_testspots;i++){
            printf("p:%d,b:%d,,,",BrightSpots[2*i],BrightSpots[2*i+1]);
        }
        printf("\n");*/

        //TODO dead pixel blocker?

        //Average brightSpot
        int averagebrightspot_x=0;
        int averagebrightspot_y=0;

        for(int brightspot=0;brightspot<camera_testspots;brightspot++){
            averagebrightspot_x+=BrightSpots[2*brightspot]%xres; //NOTE if this value does not fit then the picture is most likely not arranged as rgb or rgba, try a different camera or modify the code
            averagebrightspot_y+=BrightSpots[2*brightspot]/xres;
        }
        averagebrightspot_x/=camera_testspots;
        averagebrightspot_y/=camera_testspots; //Calculate the mean
        *current_xpos=averagebrightspot_x;
        *current_ypos=averagebrightspot_y;
        //Do a scanline search around this point



        /*
        int brightestVal=0;
        int search_current_max_pixel=0;
        for(unsigned int pixel=0; pixel<xres*yres; pixel+=19) *3 because of rgb
        {
            int pixel_brightness=pictureBuffer[pixel*3]+pictureBuffer[1+pixel*3]+pictureBuffer[2+pixel*3];
            if(brightestVal<pixel_brightness)
            {
                //printf("brighter one found%d",pixel/3);
                search_current_max_pixel=pixel;
                brightestVal=pixel_brightness;
            }
        }
        *current_xpos=((float)(search_current_max_pixel%xres))/xres;
        *current_ypos=((float)(search_current_max_pixel/xres))/yres;
        printf("x%f,y%f\n",*current_xpos,*current_ypos);
        //printf("%d\n",pictureBuffer[0]);
        */
        free(BrightSpots);
        //printf("Debug: Frame analyse ended /\\ /\\ /\\ /\\ \n");
        return S_OK;
    }
}

int* getBrightspot(int brightspot_mode){
    static volatile int xpos_local=-1;
    static volatile int ypos_local=-1;
    static IMediaControl* Control_local;
    static int* returnData;
    switch(brightspot_mode){
    case brightspot_init:
        Control_local=getPositionPointer(&xpos_local,&ypos_local);
        Control_local->lpVtbl->Run(Control_local);
        returnData=(int*)malloc(2*sizeof(int));
        return NULL;
    case brightspot_get:
        if(xpos_local==-1||ypos_local==-1){
            return NULL;
        }else{
            returnData[0]=xpos_local;
            returnData[1]=ypos_local;
        }
        xpos_local=-1;
        ypos_local=-1;
        return returnData;
    case brightspot_deinit:
        Control_local->lpVtbl->Stop(Control_local);
        break;
        return NULL;
    }
    return -1;
}

IMediaControl* getPositionPointer(volatile int* Posx,volatile int* Posy){
    printf("first test\n");
    unsigned int numberOfAllocatedCams=0;
    struct CameraListItem* AllAvailableCameras=getCameras(&numberOfAllocatedCams); //Get the address of our function which we wish to inject into the object for callback
    if(numberOfAllocatedCams==0){
        return 1;
    }
    struct CameraStorageObject* allRes=getAvailableCameraResolutions(&AllAvailableCameras[cameraNum]);
    printf("----Camera Info----\n");
    printf("FriendlyName: %S\n",AllAvailableCameras[cameraNum].friendlyName);
    printf("Path: %S\n",AllAvailableCameras[cameraNum].devicePath);
    printf("NumSupRes: %d",(allRes->numberOfSupportedResolutions));
    if(S_OK!=registerCameraCallback(allRes, cameraResNum, &callbackForGraphview)){
        printf("ERROR: Camera in use abort!\n");
        return 0;
    }
    //set pointers for positions

    //WARNING! these values should not be read from when callback could happen
    //TODO!! lock these values if they are processed #multithread
    //Set values to -1 or something like that and check for that in callback to block values
    struct inputForBrightspotfinder* BrightSpotInput=(struct inputForBrightspotfinder*)malloc(sizeof(struct inputForBrightspotfinder));
    printf("Test6\n");
    //TODO check if format is allowed to be used
    BrightSpotInput->xres=allRes->resolutionsXYPointer[cameraResNum][0]; //
    BrightSpotInput->yres=allRes->resolutionsXYPointer[cameraResNum][1]; //(allRes->numberOfSupportedResolutions)-1
    printf("Test7\n");
    printf("INPUTVAL REMOVE add %d,%d val%d,%d\n",Posx,Posy,*Posx,*Posy);
    BrightSpotInput->cam_current_xpos=Posx;
    BrightSpotInput->cam_current_ypos=Posy;
    printf("Test8\n");
    callbackForGraphview(BrightSpotInput,0); //We want to pass data before first call of this function so that it knows the resolution
    printf("Test9\n");



    printf("Debug: try to set manual exposure\n");
    long prop_Min=0;
    long prop_Max=0;
    long prop_SteppingDelta=0;
    long prop_Default=0;
    long prop_CapsFlags=0;
    if(S_OK==allRes->_CameraControl->lpVtbl->GetRange(allRes->_CameraControl,CameraControl_Exposure,&prop_Min,&prop_Max,&prop_SteppingDelta,&prop_Default,&prop_CapsFlags)){
        printf("Debug: successfully queried camera exposure range\n");
        if((prop_CapsFlags&CameraControl_Flags_Manual)!=0){
            printf("Debug: Property can be changed from %ld to %ld, default %ld, with %ld stepwidth\n",prop_Min,prop_Max,prop_Default,prop_SteppingDelta);
            /*int hr=allRes->_CameraControl->lpVtbl->Set(allRes->_CameraControl,CameraControl_Exposure,prop_Min,CameraControl_Flags_Manual);
            if(hr==S_OK){
                printf("Debug: Changed exposure time successfully");
            }else{
                printf("Error: Could not change exposure time, error %d",hr);
            }*/
        }else{
            printf("Debug: Property can be changed by manually\n");
        }
    }else{
        printf("Error: could not query camera exposure range\n");
    }

    return allRes->_MediaControl;
    /*while(1)
    {
        allRes->_MediaControl->lpVtbl->Run(allRes->_MediaControl);
    }
    */
        //TODO / WARNING make sure that no other program is accessing the camera at the same time as this will result in an error!!!

}
