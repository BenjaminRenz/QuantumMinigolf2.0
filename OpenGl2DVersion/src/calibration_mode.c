#include "calibration_mode.h"


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)


#define filepath_calibration_lb ".\\res\\calib\\lb.bmp"
#define filepath_calibration_lt ".\\res\\calib\\lt.bmp"
#define filepath_calibration_rb ".\\res\\calib\\rb.bmp"
#define filepath_calibration_rt ".\\res\\calib\\rt.bmp"

//Graphic part
enum {G_OBJECT_INIT,G_OBJECT_DRAW,G_OBJECT_UPDATE,G_OBJECT_DEINIT};
void drawCalibPicture(int G_OBJECT_STATE,mat4x4 mvp4x4_local,uint8_t* texture_if_update);

//UI
void mouse_button_callback_calibrate(GLFWwindow* window, int button, int action, int mods);
//logic
float* iscalibrated(int mouseClicked);

void enter_calibration_mode(){

    iscalibrated(0); //initialize
    float* CalibrationRawData;
    do{
        CalibrationRawData=iscalibrated(0); //for window refresh/so that it does not freeze
    }while(!CalibrationRawData);
    printf("Got Points: %f, %f, %f, %f, %f, %f, %f, %f\n",CalibrationRawData[0],CalibrationRawData[1],CalibrationRawData[2],CalibrationRawData[3],CalibrationRawData[4],CalibrationRawData[5],CalibrationRawData[6],CalibrationRawData[7]);
    camera_perspec_calibrating(CalibData,CalibrationRawData);
    free(CalibrationRawData);
    return;
}

enum {cal_init,cal_left_bottom,cal_left_top,cal_right_bottom,cal_right_top,cal_deinit};
float* iscalibrated(int mouseClicked){ //Calibration Routine witch returns NULL when not finished, else the calibration array
    #define maxNumMeasPoints 5
    static float* calibArray;
    uint8_t* texture_for_arrows_from_file;
    static unsigned int calibrationstate=cal_init;
    static unsigned char* texture_for_arrows;
    static unsigned int numMeasPoints;
    static unsigned int aquisition_running;
    static int camera_already_started=0;
    mat4x4 mvp4x4;
    switch(calibrationstate){
        case cal_init:
            printf("Debug: Calibration Init\n");
            //Set callbacks for this mode
            glfwSetKeyCallback(MainWindow, NULL);
            glfwSetMouseButtonCallback(MainWindow, mouse_button_callback_calibrate);
            glfwSetDropCallback(MainWindow, NULL);
            glfwSetScrollCallback(MainWindow, NULL);
            glfwSetWindowSizeCallback(MainWindow, NULL);
            glfwSetCursorPosCallback(MainWindow, NULL);

            //Start data aquisition
            if(!camera_already_started){
                getBrightspot(brightspot_init);
                camera_already_started=1;
            }
            calibArray=calloc(8,sizeof(float));
            drawCalibPicture(G_OBJECT_INIT,NULL,NULL);
            drawCalibPicture(G_OBJECT_UPDATE,NULL,read_bmp(filepath_calibration_lb));
            calibrationstate=cal_left_bottom;
            aquisition_running=0;
            numMeasPoints=0;
            break;
        case cal_left_bottom:
            printf("Debug: Calibration lb\n");
            if(numMeasPoints<maxNumMeasPoints){
                if(mouseClicked&&!aquisition_running){ //for the first frame
                    aquisition_running=1;
                }
                int* rawDataIn;
                rawDataIn=getBrightspot(brightspot_get);
                if(rawDataIn&&aquisition_running){ //for every arrived Frame arrived
                    calibArray[0]+=(float) rawDataIn[0];
                    calibArray[1]+=(float) rawDataIn[1];
                    numMeasPoints++;
                }
            }else{
                calibArray[0]/=maxNumMeasPoints;
                calibArray[1]/=maxNumMeasPoints;
                drawCalibPicture(G_OBJECT_UPDATE,NULL,read_bmp(filepath_calibration_rb));
                calibrationstate=cal_right_bottom;
                aquisition_running=0;
                numMeasPoints=0;
            }
            break;
        case cal_right_bottom:
            printf("Debug: Calibration rb\n");
            if(numMeasPoints<maxNumMeasPoints){
                if(mouseClicked&&!aquisition_running){ //for the first frame
                    aquisition_running=1;
                }
                int* rawDataIn;
                rawDataIn=getBrightspot(brightspot_get);
                if(rawDataIn&&aquisition_running){ //for every arrived Frame arrived
                    calibArray[2]+=(float) rawDataIn[0];
                    calibArray[3]+=(float) rawDataIn[1];
                    numMeasPoints++;
                }
            }else{
                calibArray[2]/=maxNumMeasPoints;
                calibArray[3]/=maxNumMeasPoints;
                drawCalibPicture(G_OBJECT_UPDATE,NULL,read_bmp(filepath_calibration_rt));
                calibrationstate=cal_right_top;
                aquisition_running=0;
                numMeasPoints=0;
            }
            break;
        case cal_right_top:
            printf("Debug: Calibration rt\n");
            if(numMeasPoints<maxNumMeasPoints){
                if(mouseClicked&&!aquisition_running){ //for the first frame
                    aquisition_running=1;
                }
                int* rawDataIn;
                rawDataIn=getBrightspot(brightspot_get);
                if(rawDataIn&&aquisition_running){ //for every arrived Frame arrived
                    calibArray[4]+=(float) rawDataIn[0];
                    calibArray[5]+=(float) rawDataIn[1];
                    numMeasPoints++;
                }
            }else{
                calibArray[4]/=maxNumMeasPoints;
                calibArray[5]/=maxNumMeasPoints;
                drawCalibPicture(G_OBJECT_UPDATE,NULL,read_bmp(filepath_calibration_lt));
                calibrationstate=cal_left_top;
                aquisition_running=0;
                numMeasPoints=0;
            }
            break;
        case cal_left_top:
            printf("Debug: Calibration rt\n");
            if(numMeasPoints<maxNumMeasPoints){
                if(mouseClicked&&!aquisition_running){ //for the first frame
                    aquisition_running=1;
                }
                int* rawDataIn;
                rawDataIn=getBrightspot(brightspot_get);
                if(rawDataIn&&aquisition_running){ //for every arrived Frame arrived
                    calibArray[6]+=(float) rawDataIn[0];
                    calibArray[7]+=(float) rawDataIn[1];
                    numMeasPoints++;
                }
            }else{
                calibArray[6]/=maxNumMeasPoints;
                calibArray[7]/=maxNumMeasPoints;
                printf("TEESSSSTTTT %f\n",calibArray[7]);
                calibrationstate=cal_deinit;
            }
            break;
        case cal_deinit:
            if(!mouseClicked){
                free(texture_for_arrows);
                printf("TE %d\n",(int*)calibArray);
                printf("TS %f\n",calibArray[0]);
                //Set Callback function back to normal
                glfwSetKeyCallback(MainWindow, key_callback);
                glfwSetMouseButtonCallback(MainWindow, mouse_button_callback);
                glfwSetDropCallback(MainWindow, drop_file_callback);
                glfwSetScrollCallback(MainWindow, mouse_scroll_callback);
                glfwSetWindowSizeCallback(MainWindow, windows_size_callback);
                glfwSetCursorPosCallback(MainWindow, cursor_pos_callback);
                calibrationstate=cal_init;
                drawCalibPicture(G_OBJECT_DEINIT,NULL,NULL);
                return calibArray;
            }
            return NULL;
    }
    //@@Graphics
    int width=0;
    int height=0;
    glfwGetWindowSize(MainWindow, &width, &height);
    glViewport(0, 0, width, height);
    printf("windows size: %d, %d",width,height);
    mat4x4_ortho(mvp4x4, -0.55f*width/height, 0.55f*width/height, -0.55f, 0.55f, -1.0f, 5.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawCalibPicture(G_OBJECT_DRAW, mvp4x4, NULL);
    //Swap Buffers
    glFinish();
    glfwSwapBuffers(MainWindow);
    //Process Events
    glfwPollEvents();
    return NULL;
}


void drawCalibPicture(int G_OBJECT_STATE,mat4x4 mvp4x4_local,uint8_t* texture_if_update){
    #define PICTURE_USE_TEXTURE 1
    static GLuint vboPictureID=0;
    static GLuint PictureShaderID=0;
    static GLuint PictureTextureID=0;
    static GLuint mvpMatrixUniform = 0; //How is the Uniform variable called in the compiled shader
    if(G_OBJECT_STATE==G_OBJECT_INIT){
        //Compile Shaders
        PictureShaderID = glCreateProgram();              //create program to run on GPU
        glAttachShader(PictureShaderID, CompileShaderFromFile(".\\res\\shaders\\vertex_calib.glsl", GL_VERTEX_SHADER));       //attach vertex shader to new program
        glAttachShader(PictureShaderID, CompileShaderFromFile(".\\res\\shaders\\fragment_calib.glsl", GL_FRAGMENT_SHADER));      //attach fragment shader to new program
        glLinkProgram(PictureShaderID);

/*      glActiveTexture(GL_TEXTURE2);
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        void* tempClientGuiTexture = read_bmp("");
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tempClientGuiTexture);
        free(tempClientGuiTexture);
*/
        glUseProgram(PictureShaderID);
        glUniform1i(glGetUniformLocation(PictureShaderID, "texture"), PICTURE_USE_TEXTURE); //Hack, set to the same texture as the gui
        //Get Shader Variables
        glUseProgram(PictureShaderID);
        mvpMatrixUniform = glGetUniformLocation(PictureShaderID, "MVPmatrix");   //only callable after glUseProgramm has been called once
        glGenBuffers(1,&vboPictureID);
        float VertexData[]={
            -0.5,-0.5,
            0.5,-0.5,
            -0.5,0.5,
            0.5,-0.5,
            -0.5,0.5,
            0.5,0.5,

            0.0,0.0,
            1.0,0.0,
            0.0,1.0,
            1.0,0.0,
            0.0,1.0,
            1.0,1.0
        };
        printf("DEBUG: sizeof: %d\n",sizeof(VertexData));
        glBindBuffer(GL_ARRAY_BUFFER,vboPictureID);
        glBufferData(GL_ARRAY_BUFFER,sizeof(VertexData),VertexData,GL_STATIC_DRAW);//sizeof

        glActiveTexture(GL_TEXTURE0+PICTURE_USE_TEXTURE);
        glGenTextures(1, &PictureTextureID);
        glBindTexture(GL_TEXTURE_2D, PictureTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }else if(G_OBJECT_STATE==G_OBJECT_DRAW){
        glBindBuffer(GL_ARRAY_BUFFER, vboPictureID);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(float) * 12));
        glEnableVertexAttribArray(0);   //x,y,z
        glEnableVertexAttribArray(1);
        //Enabler Shader
        glUseProgram(PictureShaderID);
        //Set Shader Uniforms to render Grid
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, (GLfloat*)mvp4x4_local);
        //glUniform1f(IntensityFloatUniform,Intensity);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_DST_ALPHA);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES,0,6);
        glEnable(GL_CULL_FACE);
        //12-2 because bottom face is missing
        //glDrawArrays(GL_TRIANGLES,24,6);
        //glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }else if(G_OBJECT_STATE==G_OBJECT_UPDATE){
        glBindTexture(GL_TEXTURE_2D, PictureTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sim_res_x, sim_res_y, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texture_if_update);
        free(texture_if_update);

    }else if(G_OBJECT_STATE==G_OBJECT_DEINIT){
        glBindTexture(GL_TEXTURE_2D, PictureTextureID);
        glDeleteTextures(1,&PictureTextureID);
    }
}


void mouse_button_callback_calibrate(GLFWwindow* window, int button, int action, int mods){
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        printf("Clicked\n");
        iscalibrated(1);
    }
}
