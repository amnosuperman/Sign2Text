#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include "cv.h"
#include "cvaux.h"
#include "cxcore.h"
#include "highgui.h"
#include "cxmisc.h"
#include <math.h>
#include "handdet.h"
#include "handdet.c"
#include "tracker.cpp"
#include "eigenface.c"
#include "facedetect.c"
#include "text.c"
#include "mouse.c"

//// Constants
const char * DISPLAY_WINDOW = "DisplayWindow";
//// Global variables
IplImage  * pframe = 0,*piframe;
CvCapture *capture;
//// Function definitions
int initAll();
void exitProgram(int code);

IplImage* invert(IplImage* img)
{
	int height,width,step,channels;
	int i1,j1,k1;
	uchar *data,*data2;
	IplImage * img2 = NULL;
	img2=cvCreateImage(cvGetSize(img),img->depth,3);		
	height= img->height;
	width=img->width;
	step= img->widthStep;
	channels = img->nChannels;
	data= (uchar *)img->imageData;
	data2= (uchar *)img2->imageData;
	for(i1=0;i1<height;i1++) 
		for(j1=0;j1<width;j1++) 
			for(k1=0;k1<channels;k1++)
				data2[(i1+1)*step-(j1+1)*channels+k1]=data[i1*step+j1*channels+k1];
	return img2;
}	

// main()
int main( int argc, char** argv )
{

	int starting = 3;
	int flag=0;
	CvPoint pt;
	int diffx=0,prevx=0;
	int diffy=0,prevy=0;
	char* word1="Is'nt";
	char* word2="this";
	char* word3="exciting?";
 
  	// Open X display
	Display *display = XOpenDisplay (NULL);
	if (display == NULL)
        {
      		fprintf (stderr, "Can't open display!\n");
      		return -1;
    	}
  
  	// Wait 3 seconds to start
  	printf ("Starting in ");
  		fflush (stdout);
  	while (starting > 0)
    	{
      		printf ("\b\b\b %d...", starting);
      		fflush (stdout);
      		sleep (1);
      		starting--;
    	}
  	printf ("\n");

    IplImage* temp=cvCreateImage(cvSize(80,120),8,3);
	IplImage* pframe1;
	
	CvRect *pHandRect=0,*vrect=NULL;
	capture=cvCaptureFromCAM(0);	
	if( !initAll() ) exitProgram(-1);
	int g;
	piframe=cvQueryFrame(capture);
	pframe=invert(piframe);
	pframe1=cvCloneImage(piframe);
	// Capture and display video frames until a hand
	// is detected
	int i=0;
	char c;	
	//char* path="/home/vignesh/imgproc/hand.xml";
    char ch;
	initPCA();

	x :
	printf("came to x\n");
	while( 1 )
	{		
		// Look for a hand in the next video frame
		pframe=cvQueryFrame(capture);
		pframe1=cvCloneImage(pframe);
    		detect_and_draw(pframe);
		//pframe=invert(piframe);		
		pHandRect = detectHand(pframe);
		if((pHandRect)&&(pHandRect->x>4)&&(pHandRect->y>4)&&(pHandRect->x*pHandRect->y<(240*300))&&(pHandRect->x<630)&&(pHandRect->y<470))
		{	
			cvRectangle(pframe1,cvPoint((pHandRect->x-4),pHandRect->y-4),cvPoint((pHandRect->x+pHandRect->width+4),pHandRect->y+pHandRect->height+4),CV_RGB(255,0,0),1,8,0);		
			i++;
		}
		else
			i=0;
		// Show the display image
		cvShowImage( DISPLAY_WINDOW, pframe1 );
		cvMoveWindow(DISPLAY_WINDOW,0,0);
		c=cvWaitKey(10); 
		if(c==27)
                {   
        	exitProgram(0);
		}
		if(i>2)
			// exit loop when a hand is detected
			if(pHandRect) {
				prevx=pHandRect->x;
				prevy=pHandRect->y+pHandRect->height;
				flag=3;
				break;
			}
	}

	// initialize tracking
	KalmanFilter kfilter;
	startTracking(pframe, *pHandRect,kfilter);
	// Track the detected hand using CamShift
	while( 1 )
	{
		CvRect handBox;

		// get the next video frame
		pframe=cvQueryFrame(capture);
		pframe1=cvCloneImage(pframe);

		// track the hand in the new video frame
		handBox = combi_track(pframe,kfilter);
        int old_ht;
        int a;
		IplImage* temp;
		if(handBox.x<0)
			goto x;
		if(!((handBox.x<0)||(handBox.y<0)||((handBox.x+handBox.width)>pframe->width)||((handBox.y+handBox.height)>pframe->height))) 
        {
            if(handBox.height>(1.3*handBox.width))
            {
                old_ht=handBox.height;
                handBox.height=2.4*handBox.width;
                handBox.y-=handBox.height-old_ht;
            }
            cvSetImageROI(pframe,handBox);
            temp=cvCreateImage(cvGetSize(pframe),8,3);

            cvCopy(pframe,temp,NULL);

	        a=recognize(temp);
	        cvReleaseImage(&temp);
	        if(handBox.height>(2.3*handBox.width))
            {	
            	if(a==3)
            		a=5;
            }

	        if(a==3)
	        {
	       		pframe1=outtext(pframe1,word1);
	       	}

	        if((a==4)||(a==1))
	        {
	       		pframe1=outtext(pframe1,word2);
	       	}

	        if(a==5)
	        {
	       		pframe1=outtext(pframe1,word3);
	       	}

			diffx=handBox.x+(handBox.width/2)-prevx;
			diffy=handBox.y+handBox.height-(handBox.width/2)-prevy;
            
			prevx=handBox.x+(handBox.width/2);
			prevy=handBox.y+handBox.height-(handBox.width/2);

	        cvResetImageROI(pframe);
    		cvRectangle(pframe1,cvPoint(handBox.x,handBox.y),cvPoint(handBox.x+handBox.width,handBox.y+handBox.height),CV_RGB(0,0,255),3,8,0);		

        }
		else 
			goto x;

		cvShowImage( DISPLAY_WINDOW, pframe1 );

		ch=cvWaitKey(10);
		if( ch==27 ) {
			exitProgram(0);			
			break;
		}
	}
	return 0;
}


//////////////////////////////////
// initAll()
//
int initAll()
{
	// Create the display window
	cvNamedWindow( DISPLAY_WINDOW, 1 );
	// Initialize tracker
	pframe=cvQueryFrame(capture);
	if( !createTracker(pframe) ) 
		return 0;
	initHandDet("/home/vignesh/Downloads/hand.xml");
	//initHandDet1("v.xml");
	// Set Camshift parameters
	setVmin(60);
	setSmin(50);
	initface();
	return 1;
}


//////////////////////////////////
// exitProgram()
//
void exitProgram(int code)
{
	// Release resources allocated in this file
	cvDestroyWindow( DISPLAY_WINDOW );
	cvReleaseImage( &pframe );
	releaseface();
	// Release resources allocated in other project files
	cvReleaseCapture(&capture);
	closeHandDet();
	releaseTracker();

	exit(code);
}

