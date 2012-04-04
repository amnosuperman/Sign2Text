#include <stdio.h>
#include "cv.h"
#include "highgui.h"

#include "text.h"
 
IplImage* outtext(IplImage* img, char* text)
{
 
    /* initialize font and add text */
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 3, CV_AA);
    cvPutText(img, text, cvPoint(img->width/10, 4*img->height/5), &font, cvScalar(255, 0, 0, 0));
 
    /* display the image */
    //cvNamedWindow("image", CV_WINDOW_AUTOSIZE);
    //cvShowImage("image", img);
    //cvWaitKey(0);
    //cvDestroyWindow("image");
    //cvReleaseImage( &img );
 
    return img;
}
