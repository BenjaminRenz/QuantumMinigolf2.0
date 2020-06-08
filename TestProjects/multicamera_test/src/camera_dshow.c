#include "camera_dshow.h"
#include "Debug/Debug.h"
//hack to still use SampleGrabber
extern GUID CLSID_SampleGrabber;
extern CLSID CLSID_NullRenderer;
//const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
//const IID IID_ISampleGrabber = { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };
//const CLSID CLSID_NullRenderer = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

HRESULT FindFirstUnconnectedPin(IBaseFilter* Filter,PIN_DIRECTION WantedPinDir,IPin** ReturnFilterPinpp){
    HRESULT hr=S_OK;
    //get first free output pin of CameraFilter
    IEnumPins* FilterPinsEnump=0;
    if(S_OK!=(hr=Filter->lpVtbl->EnumPins(Filter,&FilterPinsEnump))){
        dprintf(DBGT_ERROR,"Can't create enum for pins of Filter");
        return hr;
    }
    IPin* FilterPinp;
    while(S_OK == (hr=FilterPinsEnump->lpVtbl->Next(FilterPinsEnump,1,&FilterPinp,NULL))){  //1 stands for "only recieve 1 pin ata a time"
        PIN_DIRECTION PinDir;
        if(S_OK!=(hr=FilterPinp->lpVtbl->QueryDirection(FilterPinp,&PinDir))){
            dprintf(DBGT_ERROR,"Could not querry pin direction of Filter pin");
            FilterPinp->lpVtbl->Release(FilterPinp);
            FilterPinsEnump->lpVtbl->Release(FilterPinsEnump);
            return hr;
        }
        if(PinDir==WantedPinDir){
            FilterPinsEnump->lpVtbl->Release(FilterPinsEnump);
            break;
        }
        FilterPinp->lpVtbl->Release(FilterPinp);    //release Pinp for next cycle of while loop
    }
    if(hr!=S_OK){//Check if the search was unsucessfull and the while loop exited because hr was not equal to S_OK
        FilterPinsEnump->lpVtbl->Release(FilterPinsEnump);
        dprintf(DBGT_ERROR,"Could not find any free pin with the requested direction");
        return S_FALSE;
    }
    *ReturnFilterPinpp=FilterPinp;
    return hr;
}

void closeCamera(struct CameraStorageObject* Camera){
    Camera->_MediaControlP->lpVtbl->Stop(Camera->_MediaControlP);

    // Enumerate the filters in the graph.
    IEnumFilters* FilterEnum = NULL;
    if (S_OK==Camera->_GraphP->lpVtbl->EnumFilters(Camera->_GraphP,&FilterEnum))
    {
        IBaseFilter* Filter = NULL;
        while (S_OK == FilterEnum->lpVtbl->Next(FilterEnum,1, &Filter, NULL))
        {
            // Remove the filter.
            Camera->_GraphP->lpVtbl->RemoveFilter(Camera->_GraphP,Filter);
            // Reset the enumerator.
            FilterEnum->lpVtbl->Reset(FilterEnum);
            Filter->lpVtbl->Release(Filter);
        }
        FilterEnum->lpVtbl->Release(FilterEnum);
    }
    CoUninitialize();
}

//see: https://docs.microsoft.com/de-de/office/client-developer/outlook/mapi/implementing-objects-in-c
//see: https://www.codeproject.com/Articles/13601/COM-in-plain-C

//Define sample IGrabberCB object
ULONG WINAPI IUNKN_AddRef(IGlobalInterfaceTable* this);
ULONG WINAPI IUNKN_Release(IGlobalInterfaceTable* this);
HRESULT IUNKN_QueryInterface(IGlobalInterfaceTable* this, REFIID riid, LPVOID* lppvObj);


struct SampleGrabberCB_iface_struct{
    struct SampleGrabberCB_lpVtbl* lpVtbl;
    ULONG ref;      //reference count, works like smart pointer
};

ULONG WINAPI IUNKN_SG_AddRef(IGlobalInterfaceTable* this){
    struct SampleGrabberCB_iface_struct* self=(struct SampleGrabberCB_iface_struct*) this;
    InterlockedIncrement(&(self->ref));
    return self->ref;
}
ULONG WINAPI IUNKN_SG_Release(IGlobalInterfaceTable* this){
    struct SampleGrabberCB_iface_struct* self=(struct SampleGrabberCB_iface_struct*) this;
    InterlockedDecrement(&(self->ref));
    if(self->ref==0){
        free(self);
        dprintf(DBGT_INFO,"TODO tell the outside world that the interface has been dealocated");
    }
    return self->ref;
}

HRESULT IUNKN_SG_QueryInterface(IGlobalInterfaceTable* this, REFIID riid, LPVOID* lppvObj){
    HRESULT hr = S_OK;
    // check for nullptrs
    if (!this || !lppvObj){
        hr = ResultFromScode(E_INVALIDARG);
        return hr;
    }
    *lppvObj = NULL;
    // the interface is supported, increment the reference count and return
    this->lpVtbl->AddRef(this);
    *lppvObj = this;
    return hr;
}





void* create_ISampleGrabberCB(long (*CBFunp)(double SampleTime,unsigned char *pBuffer,long BufferLen)){
    struct SampleGrabberCB_iface_struct* objp=(struct SampleGrabberCB_iface_struct*)malloc(sizeof(struct SampleGrabberCB_iface_struct));
    IGlobalInterfaceTableVtbl* lpVtbl=(IGlobalInterfaceTableVtbl*)malloc(sizeof(IGlobalInterfaceTableVtbl));
    IGlobalInterfaceTableVtbl temp={&IUNKN_SG_QueryInterface,&IUNKN_SG_AddRef,&IUNKN_SG_Release,CBFunp};
    (*lpVtbl)=temp;
    //SampleGrabberCB methods
    objp->lpVtbl=lpVtbl;
    return (void*)objp;
};

struct CameraListItem* getCameras(unsigned int* numberOfCameras)
{
    HRESULT hr=0;

    //Initialize Com library
    if(S_OK!=CoInitializeEx(NULL,COINIT_MULTITHREADED)){
        dprintf(DBGT_ERROR,"Com library could not be initialized");
        return NULL;
    }

    //create DeviceEnumerator
    ICreateDevEnum* myDeviceEnum=NULL;
    if(S_OK!=CoCreateInstance(&CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC,&IID_ICreateDevEnum,(void **)&myDeviceEnum)){
        dprintf(DBGT_ERROR,"failed to create DeviceEnumerator");
        return NULL;
    }

    //create ClassEnumerator
    IEnumMoniker* myCameralist=NULL;
    if(S_OK!=myDeviceEnum->lpVtbl->CreateClassEnumerator(myDeviceEnum,&CLSID_VideoInputDeviceCategory, &myCameralist, 0)){
        numberOfCameras=0;
        dprintf(DBGT_ERROR,"No camera found!\n");
        return NULL;
    }

    //Release the DeviceEnumerator after we have retrieved all available Video Devices

    //TODO this is not working, we probably have to do this later, after we also have freed the Class Enumerator
    /*if(S_OK!=myDeviceEnum->lpVtbl->Release(myDeviceEnum)){
        printf("Error: Could not release VideoDevice!\n");
        return 0;
    }*/



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

struct CameraStorageObject* getAvailableCameraResolutions(struct CameraListItem CameraIn){
    //Request storage for the struct we are going to return
    struct CameraStorageObject* CameraOut=(struct CameraStorageObject*) malloc(sizeof(struct CameraStorageObject));

    //Create Graph and Filters

    //Create Graph
    CameraOut->_GraphP=NULL;
    if(S_OK!=CoCreateInstance(&CLSID_FilterGraph,NULL,CLSCTX_INPROC,&IID_IGraphBuilder,(void **)&CameraOut->_GraphP)){
        dprintf(DBGT_ERROR,"Creating DirectShow graph failed");
        return NULL;
    }

    //Create Control for Graph
    CameraOut->_MediaControlP=NULL;
    if(S_OK!=CameraOut->_GraphP->lpVtbl->QueryInterface(CameraOut->_GraphP,&IID_IMediaControl,(void**)&CameraOut->_MediaControlP)){
        dprintf(DBGT_ERROR,"Getting Control for Graph failed");
        return NULL;
    }

    //Create Camera Source Filter
    IBindCtx* myBindContext=NULL;
    CreateBindCtx(0,&myBindContext);            //TODO delete test!!!!!!!! free? unbind?
    CameraIn.MonikerPointer->lpVtbl->BindToObject(CameraIn.MonikerPointer,NULL,NULL,&IID_IBaseFilter,(void **)&(CameraOut->_camFilterP)); //Do not swap this and the line below, it will not work!
    if(S_OK!=CameraOut->_GraphP->lpVtbl->AddFilter(CameraOut->_GraphP, CameraOut->_camFilterP, L"Capture Source")){
        dprintf(DBGT_ERROR,"Could not add CaptureSource Filter to graph");
        return NULL;
    }

    //get CameraControl for hardware exposure adjustment
    IAMCameraControl *pCameraControl=0;
    if(S_OK!=CameraOut->_camFilterP->lpVtbl->QueryInterface(CameraOut->_camFilterP,&IID_IAMCameraControl,(void **)&pCameraControl)){
        dprintf(DBGT_ERROR,"Could not querry CameraControl");
        return NULL;
    }
    CameraOut->_CameraControlP=pCameraControl;

    //get first free output pin of CameraOut->_camFilterP TODO REMOVE
    IPin* CamFOutPinp;
    if(S_OK!=FindFirstUnconnectedPin(CameraOut->_camFilterP,PINDIR_OUTPUT,&CamFOutPinp)){
        dprintf(DBGT_ERROR,"Could not find an unconnected output pin on Camera Filter");
        return NULL;
    }


    //Get Resolution
    //get pin configuration-interface
    IAMStreamConfig* StreamCfg=(IAMStreamConfig*)malloc(sizeof(IAMStreamConfig));
    CamFOutPinp->lpVtbl->QueryInterface(CamFOutPinp,&IID_IAMStreamConfig, (void**) &StreamCfg);
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

//CBFun
int registerCameraCallback(struct CameraStorageObject* CameraIn,int selectedResolution,long (*CBFunp)(double SampleTime,unsigned char *pBuffer,long BufferLen)){ //selected resolution is position in array
    //Setup user selected format of CaptureSource Filter Stream
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

    //Add a SampleGrabberFilter
    IBaseFilter* SampleGrabberFp=NULL;
    if(S_OK!=CoCreateInstance(&CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter, (void**)&SampleGrabberFp)){
        dprintf(DBGT_ERROR,"Could not create SampleGrabber Filter");
        return 1;
    }
    if(S_OK!=CameraIn->_GraphP->lpVtbl->AddFilter(CameraIn->_GraphP, SampleGrabberFp,L"Sample Grabber")){
        dprintf(DBGT_ERROR,"Could not attach SampleGrabber to graph");
        return 1;
    }

    //Querry the interface of the SampleGrabberFilter
    ISampleGrabber* SampleGrabberp=NULL;
    HRESULT hr;
    if(S_OK!=(hr=SampleGrabberFp->lpVtbl->QueryInterface(SampleGrabberFp,&IID_ISampleGrabber,(void**)&SampleGrabberp))){
        dprintf(DBGT_ERROR,"Could not querry interface of SampleGrabber");
        return 1;
    }

    //setup MediaType of SampleGrabber, so that the autoconnect creates the neccesary decoding filters
    AM_MEDIA_TYPE tempMT;
    memset(&tempMT,0,sizeof(AM_MEDIA_TYPE));
    tempMT.majortype = MEDIATYPE_Video;
    tempMT.subtype = MEDIASUBTYPE_RGB24;    //for RGB with 8 bits per color and 24 bits per pixel
    if(S_OK!=SampleGrabberp->lpVtbl->SetMediaType(SampleGrabberp,&tempMT)){
        dprintf(DBGT_ERROR,"Could not set mediatype of SampleGrabber");
        return 1;
    }

    //Connect CameraSource and SampleGrabber with decoder in between
    IPin* CameraOutFPinp=NULL;
    IPin* SampleGrabberFInPinP=NULL;

    if(S_OK!=FindFirstUnconnectedPin(CameraIn->_camFilterP,PINDIR_OUTPUT,&CameraOutFPinp)){
        dprintf(DBGT_ERROR,"Could not find an unconnected output pin on Camera Filter");
        return 1;
    }
    if(S_OK!=FindFirstUnconnectedPin(SampleGrabberFp,PINDIR_INPUT,&SampleGrabberFInPinP)){
        dprintf(DBGT_ERROR,"Could not find an unconnected input pin on SampleGrabber Filter");
        return 1;
    }
    if(S_OK!=CameraIn->_GraphP->lpVtbl->Connect(CameraIn->_GraphP,CameraOutFPinp,SampleGrabberFInPinP)){
        dprintf(DBGT_ERROR,"Could not connect Camera and Sample Grabber");
        return 1;
    }

    //Create Null Renderer
    IBaseFilter* NullRendFp=NULL;
    if(S_OK!=CoCreateInstance(&CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter, (void**)&NullRendFp)){
        dprintf(DBGT_ERROR,"Could not create NullRenderer Filter");
        return 1;
    }
    if(S_OK!=CameraIn->_GraphP->lpVtbl->AddFilter(CameraIn->_GraphP,NullRendFp,L"Null Filter")){
        dprintf(DBGT_ERROR,"Could not add NullRenderer Filter to graph");
        return 1;
    }

    //Connect SampleGrabber output to null renderer
    IPin* SampleGrabberFOutPinP=NULL;
    IPin* NullRendFInPinP=NULL;
    if(S_OK!=FindFirstUnconnectedPin(SampleGrabberFp,PINDIR_OUTPUT,&SampleGrabberFOutPinP)){
        dprintf(DBGT_ERROR,"Could not find an unconnected input pin on NullRenderer Filter");
        return 1;
    }
    if(S_OK!=FindFirstUnconnectedPin(NullRendFp,PINDIR_INPUT,&NullRendFInPinP)){
        dprintf(DBGT_ERROR,"Could not find an unconnected input pin on NullRenderer Filter");
        return 1;
    }
    if(S_OK!=CameraIn->_GraphP->lpVtbl->Connect(CameraIn->_GraphP,SampleGrabberFOutPinP,NullRendFInPinP)){
        dprintf(DBGT_ERROR,"Could not connect SampleGrabber and NullRenderer");
        return 1;
    }

    //Free all querried Pins
    CameraOutFPinp->lpVtbl->Release(CameraOutFPinp);
    SampleGrabberFInPinP->lpVtbl->Release(SampleGrabberFInPinP);
    SampleGrabberFOutPinP->lpVtbl->Release(SampleGrabberFOutPinP);
    NullRendFInPinP->lpVtbl->Release(NullRendFInPinP);

    //Setup callback of SampleGrabber
    if(S_OK!=SampleGrabberp->lpVtbl->SetCallback(SampleGrabberp,create_ISampleGrabberCB(CBFunp),1)){ //1 stands for giving the called function an pointer to the buffer storing the media sample
        dprintf(DBGT_ERROR,"Error while setting callback of sampleGrabber");
        return 1;
    }
    dprintf(DBGT_INFO,"Sucessfully registered Callback");
    return S_OK;
}
