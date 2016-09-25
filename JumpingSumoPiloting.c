/*
  Copyright (C) 2014 Parrot SA

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.
  * Neither the name of Parrot nor the names
  of its contributors may be used to endorse or promote products
  derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.
*/
/**
 * @file JumpingSumoPiloting.c
 * @brief This file contains sources about basic arsdk example sending commands to a JumpingSumo for piloting it and make it jump it and receiving its battery level
 * @date 15/01/2015
 */

/*****************************************
 *
 *             include file :
 *
 *****************************************/

#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include <libARSAL/ARSAL.h>
#include <libARController/ARController.h>
#include <libARDiscovery/ARDiscovery.h>

#include "JumpingSumoPiloting.h"
#include "ihm.h"

//Open CV Libs
#include <opencv/cv.h>
#include <opencv/highgui.h>

/*****************************************
 *
 *             define :
 *
 *****************************************/
#define TAG "Group106"

#define ERROR_STR_LENGTH 2048

#define JS_IP_ADDRESS "192.168.2.1"
#define JS_DISCOVERY_PORT 44444

#define DISPLAY_WITH_MPLAYER 1

#define FIFO_DIR_PATTERN "/tmp/arsdk_XXXXXX"
#define FIFO_NAME "arsdk_fifo"

#define IHM

#define DEBUG_MOVE 0
/*****************************************
 *
 *             private header:
 *
 ****************************************/


/*****************************************
 *
 *             implementation :
 *
 *****************************************/

static char fifo_dir[] = FIFO_DIR_PATTERN;
static char fifo_name[128] = "";

int run = 1;

int gIHMRun = 1;
char gErrorStr[ERROR_STR_LENGTH];
IHM_t *ihm = NULL;

FILE *videoOut = NULL;
int writeImgs = 0;
int frameNb = 0;
ARSAL_Sem_t stateSem;


int iLastX = -1;
int iLastY = -1;

static void signal_handler(int signal)
{
    gIHMRun = 0;
}

int main (int argc, char *argv[])
{
    //Create Window
    cvNamedWindow("OpenCV Window", CV_WINDOW_AUTOSIZE);
    int failed = 0;
    ARDISCOVERY_Device_t *device = NULL;
    ARCONTROLLER_Device_t *deviceController = NULL;
    eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
    eARCONTROLLER_DEVICE_STATE deviceState = ARCONTROLLER_DEVICE_STATE_MAX;
    pid_t child = 0;

    /* Set signal handlers */
    struct sigaction sig_action = {
        .sa_handler = signal_handler,
    };

    int ret = sigaction(SIGINT, &sig_action, NULL);
    if (ret < 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, "ERROR", "Unable to set SIGINT handler : %d(%s)",
                    errno, strerror(errno));
        return 1;
    }
    ret = sigaction(SIGPIPE, &sig_action, NULL);
    if (ret < 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, "ERROR", "Unable to set SIGPIPE handler : %d(%s)",
                    errno, strerror(errno));
        return 1;
    }


    if (mkdtemp(fifo_dir) == NULL)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, "ERROR", "Mkdtemp failed.");
        return 1;
    }
    snprintf(fifo_name, sizeof(fifo_name), "%s/%s", fifo_dir, FIFO_NAME);

    if(mkfifo(fifo_name, 0666) < 0)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, "ERROR", "Mkfifo failed: %d, %s", errno, strerror(errno));
        return 1;
    }

    ARSAL_Sem_Init (&(stateSem), 0, 0);

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- Jumping Sumo Piloting --");



    if (!failed)
    {
        if (DISPLAY_WITH_MPLAYER)
        {
            // fork the process to launch ffplay
            if ((child = fork()) == 0)
            {
                execlp("ffplay", "ffplay", "-i", fifo_name, "-f", "mjpeg", NULL);
                //execlp("mplayer", "mplayer", fifo_name, NULL);
                //execlp("xterm", "xterm", "-e", "mplayer", "-demuxer",  "lavf", fifo_name, "-benchmark", "-really-quiet", NULL);
                ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Missing mplayer, you will not see the video. Please install mplayer and xterm.");
                return -1;
            }
        }
        else
        {
            /*
            // create the video folder to store video images
            char answer = 'N';
            ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Do you want to write image files on your file system ? You should have at least 50Mb. Y or N");
            scanf(" %c", &answer);
            if (answer == 'Y' || answer == 'y')
            {
                ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "You choose to write image files.");
                writeImgs = 1;
                mkdir("video", S_IRWXU);
            }
            else
            {
                ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "You did not choose to write image files.");
            }*/
        }

        if (DISPLAY_WITH_MPLAYER)
        {
            videoOut = fopen(fifo_name, "w");
        }
    }

#ifdef IHM
    ihm = IHM_New (&onInputEvent);
    if (ihm != NULL)
    {
        gErrorStr[0] = '\0';
        ARSAL_Print_SetCallback (customPrintCallback); //use a custom callback to print, for not disturb ncurses IHM

        IHM_PrintHeader (ihm, "-- Jumping Sumo Piloting --");
    }
    else
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "Creation of IHM failed.");
        failed = 1;
    }
#endif

    // create a discovery device
    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- init discovey device ... ");
        eARDISCOVERY_ERROR errorDiscovery = ARDISCOVERY_OK;

        device = ARDISCOVERY_Device_New (&errorDiscovery);

        if (errorDiscovery == ARDISCOVERY_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - ARDISCOVERY_Device_InitWifi ...");
            // create a JumpingSumo discovery device (ARDISCOVERY_PRODUCT_JS)
            errorDiscovery = ARDISCOVERY_Device_InitWifi (device, ARDISCOVERY_PRODUCT_JS, "JS", JS_IP_ADDRESS, JS_DISCOVERY_PORT);

            if (errorDiscovery != ARDISCOVERY_OK)
            {
                failed = 1;
                ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Discovery error :%s", ARDISCOVERY_Error_ToString(errorDiscovery));
            }
        }
        else
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "Discovery error :%s", ARDISCOVERY_Error_ToString(errorDiscovery));
            failed = 1;
        }
    }

    //Discover device
    
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- init discovey device ... ");
    eARDISCOVERY_ERROR errorDiscovery = ARDISCOVERY_OK;

    device = ARDISCOVERY_Device_New (&errorDiscovery);

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - ARDISCOVERY_Device_InitWifi ...");
    // create a JumpingSumo discovery device (ARDISCOVERY_PRODUCT_JS)
    errorDiscovery = ARDISCOVERY_Device_InitWifi (device, ARDISCOVERY_PRODUCT_JS, "JS", JS_IP_ADDRESS, JS_DISCOVERY_PORT);
    

    // create a device controller
    if (!failed)
    {
        deviceController = ARCONTROLLER_Device_New (device, &error);

        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "Creation of deviceController failed.");
            failed = 1;
        }
        else
        {
            IHM_setCustomData(ihm, deviceController);
        }
    }

    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- delete discovey device ... ");
        ARDISCOVERY_Device_Delete (&device);
    }

    // add the state change callback to be informed when the device controller starts, stops...
    if (!failed)
    {
        error = ARCONTROLLER_Device_AddStateChangedCallback (deviceController, stateChanged, deviceController);

        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "add State callback failed.");
            failed = 1;
        }
    }

    // add the command received callback to be informed when a command has been received from the device
    if (!failed)
    {
        error = ARCONTROLLER_Device_AddCommandReceivedCallback (deviceController, commandReceived, deviceController);

        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT (ARSAL_PRINT_ERROR, TAG, "add callback failed.");
            failed = 1;
        }
    }

    // add the frame received callback to be informed when a streaming frame has been received from the device
    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- set Video callback ... ");
        error = ARCONTROLLER_Device_SetVideoStreamCallbacks (deviceController, decoderConfigCallback, didReceiveFrameCallback, NULL , NULL);

        if (error != ARCONTROLLER_OK)
        {
            failed = 1;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%", ARCONTROLLER_Error_ToString(error));
        }
    }

    
    if (!failed)
    {
        IHM_PrintInfo(ihm, "Connecting ...");
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Connecting ...");
        error = ARCONTROLLER_Device_Start (deviceController);

        if (error != ARCONTROLLER_OK)
        {
            failed = 1;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
        }
    }

    // Checks device state
    if (!failed)
    {
        // wait state update update
        ARSAL_Sem_Wait (&(stateSem));

        deviceState = ARCONTROLLER_Device_GetState (deviceController, &error);

        if ((error != ARCONTROLLER_OK) || (deviceState != ARCONTROLLER_DEVICE_STATE_RUNNING))
        {
            failed = 1;
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- deviceState :%d", deviceState);
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
        }
    }

    // send the command that tells to the Jumping to begin its streaming
    if (!failed)
    {
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- send StreamingVideoEnable ... ");
        error = deviceController->jumpingSumo->sendMediaStreamingVideoEnable (deviceController->jumpingSumo, 1);
        if (error != ARCONTROLLER_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "- error :%s", ARCONTROLLER_Error_ToString(error));
            failed = 1;
        }
    }

    if (!failed)
    {
        IHM_PrintInfo(ihm, "Running ... (Arrow keys to move ; Spacebar to jump ; 'q' to quit)");

#ifdef IHM
    while (gIHMRun)
    {
        detectObject(deviceController);
        usleep(500);
    }
#else
        int i = 20;
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- sleep 20 ... ");
        while (gIHMRun && i--)
            sleep(1);
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "- sleep end ... ");
#endif
    }

#ifdef IHM
    IHM_Delete (&ihm);
#endif

    // we are here because of a disconnection or user has quit IHM, so safely delete everything
    if (deviceController != NULL)
    {


        deviceState = ARCONTROLLER_Device_GetState (deviceController, &error);
        if ((error == ARCONTROLLER_OK) && (deviceState != ARCONTROLLER_DEVICE_STATE_STOPPED))
        {
            IHM_PrintInfo(ihm, "Disconnecting ...");
            ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "Disconnecting ...");

            error = ARCONTROLLER_Device_Stop (deviceController);

            if (error == ARCONTROLLER_OK)
            {
                // wait state update update
                ARSAL_Sem_Wait (&(stateSem));
            }
        }

        IHM_PrintInfo(ihm, "");
        ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "ARCONTROLLER_Device_Delete ...");
        ARCONTROLLER_Device_Delete (&deviceController);

        if (DISPLAY_WITH_MPLAYER)
        {
            fflush (videoOut);
            fclose (videoOut);

            if (child > 0)
            {
                kill(child, SIGKILL);
            }
        }
    }

    ARSAL_Sem_Destroy (&(stateSem));

    unlink(fifo_name);
    rmdir(fifo_dir);

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "-- END --");

    return EXIT_SUCCESS;
}

/*****************************************
 *
 *             private implementation:
 *
 ****************************************/

// called when the state of the device controller has changed
void stateChanged (eARCONTROLLER_DEVICE_STATE newState, eARCONTROLLER_ERROR error, void *customData)
{
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG, "    - stateChanged newState: %d .....", newState);

    switch (newState)
    {
    case ARCONTROLLER_DEVICE_STATE_STOPPED:
        ARSAL_Sem_Post (&(stateSem));
        //stop
        gIHMRun = 0;

        break;

    case ARCONTROLLER_DEVICE_STATE_RUNNING:
        ARSAL_Sem_Post (&(stateSem));
        break;

    default:
        break;
    }
}

// called when a command has been received from the drone
void commandReceived (eARCONTROLLER_DICTIONARY_KEY commandKey, ARCONTROLLER_DICTIONARY_ELEMENT_t *elementDictionary, void *customData)
{
    ARCONTROLLER_Device_t *deviceController = customData;

    if (deviceController != NULL)
    {
        // if the command received is a battery state changed
        if (commandKey == ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED)
        {
            ARCONTROLLER_DICTIONARY_ARG_t *arg = NULL;
            ARCONTROLLER_DICTIONARY_ELEMENT_t *singleElement = NULL;

            if (elementDictionary != NULL)
            {
                // get the command received in the device controller
                HASH_FIND_STR (elementDictionary, ARCONTROLLER_DICTIONARY_SINGLE_KEY, singleElement);

                if (singleElement != NULL)
                {
                    // get the value
                    HASH_FIND_STR (singleElement->arguments, ARCONTROLLER_DICTIONARY_KEY_COMMON_COMMONSTATE_BATTERYSTATECHANGED_PERCENT, arg);

                    if (arg != NULL)
                    {
                        // update UI
                        batteryStateChanged (arg->value.U8);
                    }
                    else
                    {
                        ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "arg is NULL");
                    }
                }
                else
                {
                    ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "singleElement is NULL");
                }
            }
            else
            {
                ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "elements is NULL");
            }
        }
    }
}

void batteryStateChanged (uint8_t percent)
{
    // callback of changing of battery level

    if (ihm != NULL)
    {
        IHM_PrintBattery (ihm, percent);
    }

}

eARCONTROLLER_ERROR decoderConfigCallback (ARCONTROLLER_Stream_Codec_t codec, void *customData)
{
    ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG, "decoderConfigCallback codec.type :%d", codec.type);

    return ARCONTROLLER_OK;
}

eARCONTROLLER_ERROR didReceiveFrameCallback (ARCONTROLLER_Frame_t *frame, void *customData)
{
    //Video Out Code
    if (videoOut != NULL)
    {
        if (frame != NULL)
        {
            /*fwrite(frame->data, frame->used, 1, videoOut);

            fflush (videoOut);*/
                  // Create File Name
                char filename[20] = "frameImage.jpg";

                // Open File For Saving
                FILE *img = fopen(filename, "w");

                //Save Image to File
                fwrite(frame->data, frame->used, 1, img);

                //Close Image
                fclose(img);
        }
        else
        {
            ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "frame is NULL.");
        }
    }
    else
    {
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG, "videoOut is NULL.");
    }

    return ARCONTROLLER_OK;
}


// IHM callbacks:

void onInputEvent (eIHM_INPUT_EVENT event, void *customData)
{
    ARCONTROLLER_Device_t *deviceController = (ARCONTROLLER_Device_t *)customData;
    eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
    if(deviceController != NULL)
    {
        /*if(event == IHM_INPUT_EVENT_FORWARD)
        {
            //nothing
        }
        else
        {
            error = deviceController->jumpingSumo->setPilotingPCMDFlag (deviceController->jumpingSumo, 0);
            error = deviceController->jumpingSumo->setPilotingPCMDSpeed (deviceController->jumpingSumo, 0);
            error = deviceController->jumpingSumo->setPilotingPCMDTurn (deviceController->jumpingSumo, 0);
        }*/
        
        if(event == IHM_INPUT_EVENT_EXIT)
        {
            gIHMRun = 0;
        }
    }
    if (error != ARCONTROLLER_OK)
    {
        IHM_PrintInfo(ihm, "Error sending an event");
    }
}

int customPrintCallback (eARSAL_PRINT_LEVEL level, const char *tag, const char *format, va_list va)
{
    // Custom callback used when ncurses is runing for not disturb the IHM

    if ((level == ARSAL_PRINT_ERROR) && (strcmp(TAG, tag) == 0))
    {
        // Save the last Error
        vsnprintf(gErrorStr, (ERROR_STR_LENGTH - 1), format, va);
        gErrorStr[ERROR_STR_LENGTH - 1] = '\0';
    }

    return 1;
}

void detectObject(ARCONTROLLER_Device_t *deviceController)
{
    eARCONTROLLER_ERROR error = ARCONTROLLER_OK;
    if(deviceController != NULL)
    {
        //Load Saved Frame Image to OpenCV
        IplImage *imgOriginal = cvLoadImage("frameImage.jpg", CV_LOAD_IMAGE_COLOR);
        
        //Check if Image is valid
        if (!imgOriginal)
        {
            printf("Not a valid image\n");
            //Can force exit here
            //return 1;
        } 
        else 
        {
            //--Detection Code--
            //Create Trackbar Window
             cvNamedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
            
            //Boundaries the define the middle of the screen
            int xUBound = 420;
            int xLBound = 220;
            int yUBound = 280;
            int yLBound = 160;

            //Detection Values


            int iLowH = 0;
            int iHighH = 14;

            int iLowS = 0;
            int iHighS = 255;

            int iLowV = 143;
            int iHighV = 255;


            //Create trackbars in "Control" window
            cvCreateTrackbar("LowH", "Control", &iLowH, 255, NULL); //Hue (0 - 179)
            cvCreateTrackbar("HighH", "Control", &iHighH, 255, NULL);

            cvCreateTrackbar("LowS", "Control", &iLowS, 255, NULL); //Saturation (0 - 255)
            cvCreateTrackbar("HighS", "Control", &iHighS, 255, NULL);

            cvCreateTrackbar("LowV", "Control", &iLowV, 255, NULL);//Value (0 - 255)
            cvCreateTrackbar("HighV", "Control", &iHighV, 255, NULL);



            //Get Size of original Image
            CvSize size = cvGetSize(imgOriginal);

            IplImage *imgHSV = cvCreateImage(size, IPL_DEPTH_8U, 3);
            //Convert the captured frame from BGR to HSV
            cvCvtColor(imgOriginal, imgHSV, CV_BGR2HSV); 

            //IplImage *imgThresholded;
            CvMat *imgThresholded = cvCreateMat(size.height, size.width, CV_8UC1);;

            // //Range
            cvInRangeS(imgHSV, cvScalar(iLowH, iLowS, iLowV, 0), cvScalar(iHighH, iHighS, iHighV, 0), imgThresholded); //Threshold the image

            //morphological opening (removes small objects from the foreground)
            cvErode(imgThresholded, imgThresholded, cvCreateStructuringElementEx(3, 3, 0, 0, CV_SHAPE_ELLIPSE, NULL), 1);
            
            cvDilate( imgThresholded, imgThresholded, cvCreateStructuringElementEx(3, 3, 0, 0, CV_SHAPE_ELLIPSE, NULL), 1);
            //morphological closing (removes small holes from the foreground)
            cvDilate( imgThresholded, imgThresholded, cvCreateStructuringElementEx(3, 3, 0, 0, CV_SHAPE_ELLIPSE, NULL), 1);
            cvErode(imgThresholded, imgThresholded, cvCreateStructuringElementEx(3, 3, 0, 0, CV_SHAPE_ELLIPSE, NULL), 1);

            //Calculate the moments of the thresholded image
            CvMoments oMoments;
            cvMoments(imgThresholded, &oMoments, 0);

            double dM01 = oMoments.m01;
            double dM10 = oMoments.m10;
            double dArea = oMoments.m00;

            // if the area <= 5000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
            if (dArea > 5000)
            {
                //calculate the position of the ball
                int posX = dM10 / dArea;
                int posY = dM01 / dArea;
            
                if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) {
                    // If object is in middle of screen
                    if (posX > xLBound && posX < xUBound) {
                        
                        //If it covers this amount of the screen then stop it from moving
                        if(abs(dArea) > 6000000){
                            if(DEBUG_MOVE == 1){
                                printf("GOAL\n");
                            }
                            // CALLUM: Stop the drone (Set movement to 0).
                            error = deviceController->jumpingSumo->setPilotingPCMDFlag (deviceController->jumpingSumo, 0);
                            error = deviceController->jumpingSumo->setPilotingPCMDSpeed (deviceController->jumpingSumo, 0);
                            error = deviceController->jumpingSumo->setPilotingPCMDTurn (deviceController->jumpingSumo, 0);
                            // *****END*****

                        } else {
                            if(DEBUG_MOVE == 1){
                                printf("Forward = %d\n", abs(dArea)) ;
                            }
                            // CALLUM: Move forward (Depending on the camera speed, this may need to be adjusted).
                            error = deviceController->jumpingSumo->setPilotingPCMDFlag (deviceController->jumpingSumo, 1);
                            error = deviceController->jumpingSumo->setPilotingPCMDSpeed (deviceController->jumpingSumo, 5);
                            /*usleep(1000000);
                            error = deviceController->jumpingSumo->setPilotingPCMDFlag (deviceController->jumpingSumo, 0);
                            error = deviceController->jumpingSumo->setPilotingPCMDSpeed (deviceController->jumpingSumo, 0);
                            error = deviceController->jumpingSumo->setPilotingPCMDTurn (deviceController->jumpingSumo, 0);*/
                            // *****END*****
                            
                        }
                    }
                    
                    else {
                        // If object is on the right of the screen
                        if (posX > xUBound ) {
                            if(DEBUG_MOVE == 1){
                                printf("Right\n");
                            }
                            // CALLUM: Move right (Again if the camera is too slow then you will need to slow down the turn speed).
                            error = deviceController->jumpingSumo->setPilotingPCMDFlag (deviceController->jumpingSumo, 1);
                            error = deviceController->jumpingSumo->setPilotingPCMDTurn (deviceController->jumpingSumo, 2);
                            /*usleep(1000000);
                            error = deviceController->jumpingSumo->setPilotingPCMDFlag (deviceController->jumpingSumo, 0);
                            error = deviceController->jumpingSumo->setPilotingPCMDSpeed (deviceController->jumpingSumo, 0);
                            error = deviceController->jumpingSumo->setPilotingPCMDTurn (deviceController->jumpingSumo, 0);*/
                            // *****END*****
                            
                        }
                        
                        // If object is on the left of the screen or not at all
                        else if (posX < xLBound) {
                            if(DEBUG_MOVE == 1){
                                printf("Left\n");
                            }
                            // CALLUM: Move left (The last movement behaviour, if testing is good then you are done.).
                            error = deviceController->jumpingSumo->setPilotingPCMDFlag (deviceController->jumpingSumo, 1);
                            error = deviceController->jumpingSumo->setPilotingPCMDTurn (deviceController->jumpingSumo, -2);
                            /*usleep(1000000);
                            error = deviceController->jumpingSumo->setPilotingPCMDFlag (deviceController->jumpingSumo, 0);
                            error = deviceController->jumpingSumo->setPilotingPCMDSpeed (deviceController->jumpingSumo, 0);
                            error = deviceController->jumpingSumo->setPilotingPCMDTurn (deviceController->jumpingSumo, 0);*/
                            // *****END*****
                        }
                    }
                }
                iLastX = posX;
                iLastY = posY;
            }

            //Display Threshold Image
            cvShowImage("Thresholded Image", imgThresholded);

            //Display Image
            cvShowImage("OpenCV Window", imgOriginal);

            //Delay Image by 1 Millisecond
            cvWaitKey(1);

            //Free Image
            cvReleaseImage(&imgOriginal);
        }

        if (error != ARCONTROLLER_OK)
        {
            IHM_PrintInfo(ihm, "Error sending an event");
        }
    } 
    else
    {
        printf("device NULL\n");
    }
}