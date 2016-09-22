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
	CvCapture *capture = cvCreateCameraCapture(0);

	if(!capture){
		printf("Capture failure\n");
		return -1;
	}

    //Capture a temporary image from the camera
    IplImage *imgTmp;
    imgTmp = cvQueryFrame(capture);   

    cvNamedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
	
	//Boundaries the define the middle of the screen
	int xUBound = 420;
	int xLBound = 220;
	int yUBound = 280;
	int yLBound = 160;

    int iLowH = 170;
    int iHighH = 179;

    int iLowS = 150;
    int iHighS = 255;

    int iLowV = 60;
    int iHighV = 255;

    //Create trackbars in "Control" window
    cvCreateTrackbar("LowH", "Control", &iLowH, 255, NULL); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control", &iHighH, 255, NULL);

    cvCreateTrackbar("LowS", "Control", &iLowS, 255, NULL); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control", &iHighS, 255, NULL);

    cvCreateTrackbar("LowV", "Control", &iLowV, 255, NULL);//Value (0 - 255)
    cvCreateTrackbar("HighV", "Control", &iHighV, 255, NULL);

    int iLastX = -1;
    int iLastY = -1;

    CvSize tmpSize = cvGetSize(imgTmp);
    CvMat *imgLines = cvCreateMat(tmpSize.height, tmpSize.width, CV_8UC1);

    while (1)
    {
        IplImage *imgOriginal;

		if (!(imgOriginal = cvQueryFrame(capture))) //if not success, break loop
        {
		    printf("Cannot read a frame from video stream\n");
		    break;
        }

        CvSize size = cvGetSize(imgOriginal);

		IplImage *imgHSV = cvCreateImage(size, IPL_DEPTH_8U, 3);
		cvCvtColor(imgOriginal, imgHSV, CV_BGR2HSV); //Convert the captured frame from BGR to HSV

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
		    printf ("DArea Now: %d \n",abs(dArea));

		    //calculate the position of the ball
		    int posX = dM10 / dArea;
		    int posY = dM01 / dArea;

			
		    if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) {
				// If object is in middle of screen
				if (posX > xLBound && posX < xUBound) {
					// Move forward
					
					// Stop once object takes up too much of screen
				}
				
				else {
					// If object is on the right of the screen
					if (posX > xUBound ) {
						// Turn Drone right a small amount
					}
					
					// If object is on the left of the screen or not on screen at all
					else {
						// Turn Drone left a small amount
					}

				}
				
			}
			

		    iLastX = posX;
		    iLastY = posY;
		}

		cvShowImage("Thresholded Image", imgThresholded); //show the thresholded image

		// cvMerge(imgOriginal, imgLines, 0, 0, imgOriginal);

		cvShowImage("Original", imgOriginal); //show the original image

	    if (cvWaitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
            printf("esc key is pressed by user\n");
            break;
		}
    }

    return 0;
}