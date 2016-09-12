
/*
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include <opencv/highgui.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
*/

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <stdio.h>
#include <unistd.h>

int main( int argc, char** argv )
{
	//Get webcam in slot 0
	CvCapture *capture = cvCreateCameraCapture(0);

	//Check if camera is connected
	if(!capture){
		printf("Capture failure\n");
		return -1;
	}

	//While loop to continously fetch frame
    while (1)
    {
	    //Setup frame
    	IplImage *frame = cvQueryFrame(capture);   

    	//Show Frame
		cvShowImage("openCV Video Feed", frame);
		 //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
	    if (cvWaitKey(30) == 27)
		{
            printf("esc key is pressed by user\n");
            break;
		}
    }

    return 0;
}
