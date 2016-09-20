#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <stdio.h>
#include <unistd.h>

/* Frame issue in [492]'eARCONTROLLER_ERROR didReceiveFrameCallback'
 * This looks at [505] 'else if (writeImgs)'
 * It looks at saving the frame and then exporting it.
*/

int main(int argc, char** argv )
{
    //Gets Command Line Argument
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

    //[507] Create a filename
    char filename[20];

    //[508] File name change (In this case nothing happens)
    snprintf(filename, sizeof(filename), "%s", argv[1]);

/*    //[511-512] File is opened then frame data is written into it
    // We don't have a frame so we can ignore this line
    FILE *img = fopen(filename, "w");
    fwrite(frame, frame->used, 1, img);

    // [513] After the Image has been closed we can assume it's saved in the folder
    //fclose(fileRead);*/

    //-- Alternative method to create a copy then open it --//
    //Create a filename to copy existing jpeg
    char fileNameCopy[20] = "CopiedJpg.jpg";

    // Open saved jpeg to be read from
    FILE *fileRead = fopen(filename, "rb");

    // Open the new file to be written into
    FILE *fileW = fopen(fileNameCopy, "wb");
 
    //Copying One Jpeg to Another
    unsigned char buf[255];
    size_t size;
    while( (size = fread(buf, 1, sizeof(buf), fileRead) ) > 0)
        fwrite(buf, 1, size, fileW);

    //Close files
    fclose(fileW);
    fclose(fileRead);
    //------------ End of Alternative Method -------------//


    // [514+] Load and Display image in OpenCV
    // All you need is the File Name Nothing
    IplImage *image = cvLoadImage(fileNameCopy, CV_LOAD_IMAGE_COLOR);

    if (!image)
    {
        printf("Not a valid image\n");
        return 1;
    }

    cvNamedWindow(fileNameCopy, CV_WINDOW_AUTOSIZE); 
    cvShowImage(fileNameCopy, image );

    cvWaitKey(0);
    cvReleaseImage(&image);

    return 0;
}