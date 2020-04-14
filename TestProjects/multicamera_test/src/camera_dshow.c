#include "camera_dshow.h"

void closeCamera(struct CameraStorageObject* Camera){
    Camera->_MediaControlP->lpVtbl->Stop(Camera->_MediaControlP);

    // Enumerate the filters in the graph.
    IEnumFilters* FilterEnum = NULL;
    if (S_OK==Camera->_CameraGraphP->lpVtbl->EnumFilters(Camera->_CameraGraphP,&FilterEnum))
    {
        IBaseFilter* Filter = NULL;
        while (S_OK == FilterEnum->lpVtbl->Next(FilterEnum,1, &Filter, NULL))
        {
            // Remove the filter.
            Camera->_CameraGraphP->lpVtbl->RemoveFilter(Camera->_CameraGraphP,Filter);
            // Reset the enumerator.
            FilterEnum->lpVtbl->Reset(FilterEnum);
            Filter->lpVtbl->Release(Filter);
        }
        FilterEnum->lpVtbl->Release(FilterEnum);
    }
    CoUninitialize();
}


struct CameraListItem* getCameras(unsigned int* numberOfCameras)
{
    HRESULT hr=0;
    hr=CoInitializeEx(NULL,COINIT_MULTITHREADED);
    if(hr!=S_OK)
    {
        printf("Error: Com library could not be initialized!\n");
        return 0;
    }
    printf("Info: Successfully initialized COM library\n");
    ICreateDevEnum* myDeviceEnum=NULL;
    if(S_OK!=CoCreateInstance(&CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC,&IID_ICreateDevEnum,(void **)&myDeviceEnum)){
        return 0;
    }
    else{
        printf("Info: Successfully created DeviceEnumerator\n");
    }
    IEnumMoniker* myCameralist=NULL;
    if(S_OK!=myDeviceEnum->lpVtbl->CreateClassEnumerator(myDeviceEnum,&CLSID_VideoInputDeviceCategory, &myCameralist, 0)){
        numberOfCameras=0;
        printf("No camera found!\n");
        return 0;
    }

    //Release the DeviceEnumerator after we have retrieved all available Video Devices

    //TODO this is not working, we probably have to do this later, after we also have freed the Class Enumerator
    /*if(S_OK!=myDeviceEnum->lpVtbl->Release(myDeviceEnum)){
        printf("Error: Could not release VideoDevice!\n");
        return 0;
    }*/
    printf("Info: Successfully created VideoInputEnumerator\n");
    IMoniker* myCamera=NULL;
    struct CameraListItem* CameraListItemp=NULL;        //create a pointer for the CameraIdentifier-structs we wish to alloacate
    while(S_OK==myCameralist->lpVtbl->Next(myCameralist,1,&myCamera,NULL))
    {
        CameraListItemp=(struct CameraListItem*) realloc(CameraListItemp,((*numberOfCameras)+1)*sizeof(struct CameraListItem));
        CameraListItemp[*numberOfCameras].MonikerPointer=myCamera;//Store the moniker object for later use
        //IBindCtx* myBindContext=NULL;
        //hr=CreateBindCtx(0,&myBindContext);
        IPropertyBag* myPropertyBag=NULL;
        VARIANT VariantField; //Do not set to =0 or we will get access violation
        VariantInit(&VariantField);

        //Get specific data such as name and device path for camera
        hr=myCamera->lpVtbl->BindToStorage(myCamera,0,NULL,&IID_IPropertyBag,(void**)&myPropertyBag); //TODO test if it also works with a nullpointer
        myPropertyBag->lpVtbl->Read(myPropertyBag,L"FriendlyName",&VariantField,0);
        memcpy(CameraListItemp[*numberOfCameras].friendlyName, VariantField.bstrVal,28*sizeof(char32_t )); //Fill our structs with info and leve last character as null string terminator
        CameraListItemp[*numberOfCameras].friendlyName[29]=0; //set string termination character
        myPropertyBag->lpVtbl->Read(myPropertyBag,L"DevicePath",&VariantField,0);
        memcpy(CameraListItemp[*numberOfCameras].devicePath,VariantField.bstrVal,198*sizeof(char32_t ));
        CameraListItemp[*numberOfCameras].devicePath[199]=0; //make sure we have terminated the string
        //Increment
        (*numberOfCameras)++;
        //Cleanup
        VariantClear(&VariantField);
        myPropertyBag->lpVtbl->Release(myPropertyBag);
        //myBindContext->lpVtbl->Release(myBindContext); //After we got the device path and name we release the Context
    }
    printf("Info: Detected %d Camera(s)\n",*numberOfCameras);
    return CameraListItemp;
}

struct CameraStorageObject* getAvailableCameraResolutions(struct CameraListItem CameraIn)
{
    struct CameraStorageObject* CameraOut=(struct CameraStorageObject*) malloc(sizeof(struct CameraStorageObject));

    //Create Graph and Filter
    //Instantiate Graph to get access to FilterObjects
    CameraOut->_CameraGraphP=NULL;
    CoCreateInstance(&CLSID_FilterGraph,NULL,CLSCTX_INPROC,&IID_IGraphBuilder,(void **)&CameraOut->_CameraGraphP);
    //Create Control for Graph
    CameraOut->_MediaControlP=NULL;
    CameraOut->_CameraGraphP->lpVtbl->QueryInterface(CameraOut->_CameraGraphP,&IID_IMediaControl,(void**)&CameraOut->_MediaControlP);
    //Create Filter
    IBaseFilter* CameraFilter=NULL;
    IBindCtx* myBindContext=NULL;
    CreateBindCtx(0,&myBindContext);            //TODO free? unbind?
    CameraIn.MonikerPointer->lpVtbl->BindToObject(CameraIn.MonikerPointer,NULL,NULL,&IID_IBaseFilter,(void **)&CameraFilter); //Do not swap this and the line below, it will not work!
    CameraOut->_CameraGraphP->lpVtbl->AddFilter(CameraOut->_CameraGraphP, CameraFilter, L"Capture Source");
    IEnumPins* CameraOutputPins=0;
    CameraFilter->lpVtbl->EnumPins(CameraFilter,&CameraOutputPins);

    //
    printf("Experimental feature\n");
    IAMCameraControl *pCameraControl=0;
    CameraFilter->lpVtbl->QueryInterface(CameraFilter,&IID_IAMCameraControl,(void **)&pCameraControl);

    CameraOut->_CameraControlP=pCameraControl;


    //get output pin of CameraInputFilter
    IPin* pOutputPin=(IPin*) malloc(sizeof(IPin));
    CameraOutputPins->lpVtbl->Next(CameraOutputPins,1,&pOutputPin,0);
    CameraOut->_outputpinP=pOutputPin;
    //Get Resolution
    //get pin configuration-interface
    IAMStreamConfig* StreamCfg=(IAMStreamConfig*)malloc(sizeof(IAMStreamConfig));
    pOutputPin->lpVtbl->QueryInterface(pOutputPin,&IID_IAMStreamConfig, (void**) &StreamCfg);
    CameraOut->_StreamCfgP=StreamCfg;
    //Get size of config-structure for pin
    int numberOfPossibleRes=0;
    int sizeOfCFGStructureInByte=0;
    StreamCfg->lpVtbl->GetNumberOfCapabilities(StreamCfg,&numberOfPossibleRes,&sizeOfCFGStructureInByte);
    printf("Info: found %d possible formats\n",numberOfPossibleRes);
    byte* pUnusedSSC=(byte*) malloc(sizeof(byte)*sizeOfCFGStructureInByte);
    //get VideoSteamConfigSturcure
    CameraOut->_amMediaArrayP=(AM_MEDIA_TYPE**) malloc(sizeof(AM_MEDIA_TYPE)*numberOfPossibleRes);
    unsigned int** resolutionPointerArray=(unsigned int**) malloc((2*sizeof(int)+sizeof(int*))*numberOfPossibleRes); //we create an array for x,y resolution (sizeof(long)*2) which contains pointers to the first location of every pair to be accessed as regular 2d array (size_t size of general pointer)
    for(int numOfRes=0; numOfRes<numberOfPossibleRes; numOfRes++)
    {
        AM_MEDIA_TYPE* pAmMedia=NULL;
        StreamCfg->lpVtbl->GetStreamCaps(StreamCfg,numOfRes,&pAmMedia,pUnusedSSC);
        CameraOut->_amMediaArrayP[numOfRes]=pAmMedia;
        printf("\nInfo: Returning Info for item %d of %d\n",numOfRes+1,numberOfPossibleRes);
        if(pAmMedia->formattype.Data1==FORMAT_VideoInfo.Data1 && (pAmMedia->cbFormat >= sizeof(VIDEOINFOHEADER)) && pAmMedia->pbFormat!=NULL)  //Check if right format, If the space at pointer location is valid memory and if the pointer is actually filled with something
        {
            VIDEOINFOHEADER* pVideoInfoHead=(VIDEOINFOHEADER*) pAmMedia->pbFormat; //Get video info header
            resolutionPointerArray[numOfRes]=((unsigned int*)(resolutionPointerArray+numberOfPossibleRes))+(numOfRes*2); //point to the first element of the resolution tuples stored after the pointer table
            resolutionPointerArray[numOfRes][0]=pVideoInfoHead->bmiHeader.biWidth;
            resolutionPointerArray[numOfRes][1]=pVideoInfoHead->bmiHeader.biHeight;
            printf("Info width:%d\n",resolutionPointerArray[numOfRes][0]);
            printf("Info height:%d\n",resolutionPointerArray[numOfRes][1]);
        }
        else
        {
            printf("Warn: unusable format, resolution won't be extracted!\n");
            resolutionPointerArray[numOfRes]=((unsigned int*)(resolutionPointerArray+numberOfPossibleRes))+(numOfRes*2);
            resolutionPointerArray[numOfRes][0]=0;
            resolutionPointerArray[numOfRes][1]=0;
        }
        //if(numberOfPossibleRes)
        //Code checks for format
        /*if(AmMedia->majortype.Data1!=MEDIATYPE_Video.Data1){
            //Free this MEDIATYPE_FORMAT
            if(AmMedia->pbFormat!=0){ //Block with detailed format description
                CoTaskMemFree((void*)AmMedia->pbFormat);
            }
            if(AmMedia->pUnk!=NULL){
                AmMedia->pUnk->lpVtbl->Release(AmMedia->pUnk);
            }
            CoTaskMemFree(AmMedia);
            printf("Info: Free unused Format\n");
        }else{
            printf("1:%d\n",AmMedia->subtype.Data1);
            printf("2:%d\n",AmMedia->subtype.Data2);
            printf("3:%d\n",AmMedia->subtype.Data3);
            printf("4:%d\n\n",AmMedia->subtype.Data4);
            printf("MediaSubtype:%d\n",MEDIASUBTYPE_RGB24.Data1);
        }
        //DeleteMediaType
        printf("test%d\n\n",numberOfPossibleRes);
        */

    }
    CameraOut->numberOfSupportedResolutions=numberOfPossibleRes;
    CameraOut->resolutionsXYarrayP=resolutionPointerArray;
    free(pUnusedSSC);
    printf("returned");
    return CameraOut;
}

int registerCameraCallback(struct CameraStorageObject* CameraIn,int selectedResolution,HRESULT (*callbackForGraphviewFuncPointer) (void*, IMediaSample*) ) //selected resolution is position in array
{
    DWORD no;
    INT_PTR* p=NULL; //will be pointer to the input function of the render filter
    CameraIn->_StreamCfgP->lpVtbl->SetFormat(CameraIn->_StreamCfgP,(CameraIn->_amMediaArrayP[selectedResolution]));
    //Free unused formats
    for(int formatIter=0; formatIter<CameraIn->numberOfSupportedResolutions; formatIter++)
    {
        if(formatIter!=selectedResolution){
            if((CameraIn->_amMediaArrayP[formatIter])->pbFormat!=0)  //Block with detailed format description
            {
                CoTaskMemFree((void*)(CameraIn->_amMediaArrayP[formatIter])->pbFormat);
            }
            if((CameraIn->_amMediaArrayP[formatIter])->pUnk!=NULL)
            {
                (CameraIn->_amMediaArrayP[formatIter])->pUnk->lpVtbl->Release(CameraIn->_amMediaArrayP[formatIter]->pUnk);
            }
            CoTaskMemFree((CameraIn->_amMediaArrayP[formatIter]));
            printf("Info: Free unused Format\n");
        }
    }
    CameraIn->_CameraGraphP->lpVtbl->Render(CameraIn->_CameraGraphP,CameraIn->_outputpinP); //Render this output
    //get renderPin and hijack the method which recieves the inputdata from the last filter in the graph
    IEnumFilters* myFilter=NULL;
    CameraIn->_CameraGraphP->lpVtbl->EnumFilters(CameraIn->_CameraGraphP,&myFilter);//OK
    IBaseFilter* rnd=NULL;
    myFilter->lpVtbl->Next(myFilter,1,&rnd,0);//Does not work, no filter recieved
    IEnumPins* myRenderPins=0;
    rnd->lpVtbl->EnumPins(rnd,&myRenderPins);
    IPin* myRenderPin=0;
    myRenderPins->lpVtbl->Next(myRenderPins,1,&myRenderPin, 0);
    IMemInputPin* myMemoryInputPin=NULL;
    myRenderPin->lpVtbl->QueryInterface(myRenderPin,&IID_IMemInputPin,(void**)&myMemoryInputPin);
    p=6+*(INT_PTR**)myMemoryInputPin; //Get the function pointer for Recieve() of myRenderPin which we will use later to "inject" out own function pointer to redirect the output of the previous filter
    //printf("Debug: callback address location: %llux\n",(uint64_t)p);
    //printf("Debug: address before change: %llux\n",(uint64_t)*p);
    VirtualProtect(p,4,PAGE_EXECUTE_READWRITE,&no);//To allow the write from our thread because the graph lives in a separate thread
    //Hide the (now empty/black) popup window
    IVideoWindow* myVideoWindow=NULL;
    CameraIn->_CameraGraphP->lpVtbl->QueryInterface(CameraIn->_CameraGraphP,&IID_IVideoWindow,(void*)&myVideoWindow);
    CameraIn->_MediaControlP->lpVtbl->Pause(CameraIn->_MediaControlP); //do this to make return of run dependent on if the camera is already in use
    if(S_OK!=CameraIn->_MediaControlP->lpVtbl->Run(CameraIn->_MediaControlP)){
        printf("Alert camera already in use abort!\n");
        return S_FALSE;
    }else{
        printf("Debug: Camera not in use, continue\n");
    }
    myVideoWindow->lpVtbl->put_Visible(myVideoWindow,0);
    *p=(INT_PTR)callbackForGraphviewFuncPointer;
    //printf("Debug: address after change: %lx\n",*p);
    return S_OK;
}
