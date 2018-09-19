#include <signal.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <termios.h>
#include <sys/time.h>
#include <errno.h>
#include <syslog.h>
#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/gpu/device/utility.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/video/tracking.hpp>

extern "C" {
	#include "util.h"
	#include "display-kms.h"
	#include "v4l2.h"
	#include "vpe-common.h"
	#include "input_cmd.h"
	#include "car_lib.h"
    #include "exam_cv.h"
}
#include "system_management.h"
#include "image_processing.h"
#include "cv.h"
#include "calibration.h"

extern std::ofstream fileout;
extern System_resource system_resource;
using namespace cv;

#define DUMP_MSGQ_KEY           1020
#define DUMP_MSGQ_MSG_TYPE      0x02
#include <iostream>
#include <ctype.h>

using namespace std;

Mat image;
CvConnectedComp comp;
bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;
// User draws box around object to track. This triggers CAMShift to start tracking
static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);
        selection &= Rect(0, 0, image.cols, image.rows);
    }
    switch( event )
    {
    case EVENT_LBUTTONDOWN:
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectObject = true;
        break;
    case EVENT_LBUTTONUP:
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
            trackObject = -1;   // Set up CAMShift properties in main() loop
        break;
    }
}
int main( int argc, const char** argv )
{
    VideoCapture cap;
    Rect trackWindow;
    int hsize = 16;
    float hranges[] = {0,180};
    const float* phranges = hranges;
    //* cap open
    namedWindow( "Histogram", 0 );
    namedWindow( "CamShift Demo", 0 );
    setMouseCallback( "CamShift Demo", onMouse, 0 );
    createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
    createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
    createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );
    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    bool paused = false;
    while(1)
    {
        // capture at Mat image
        frame.copyTo(image);
        if( !paused )
        {
            // hsv convert at Mat hav
            cvtColor(image, hsv, COLOR_BGR2HSV);
            if( trackObject )
            {
                int _vmin = vmin, _vmax = vmax;
                // inRange Mat hsv
                inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
                        Scalar(180, 256, MAX(_vmin, _vmax)), mask);
                int ch[] = {0, 0};
                hue.create(hsv.size(), hsv.depth());
                mixChannels(&hsv, 1, &hue, 1, ch, 1);
                if( trackObject < 0 )
                {
                    // Object has been selected by user, set up CAMShift search properties once
                    Mat roi(hue, selection), maskroi(mask, selection);
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
                    normalize(hist, hist, 0, 255, NORM_MINMAX);
                    trackWindow = selection;
                    trackObject = 1; // Don't set up again, unless user selects new ROI
                    histimg = Scalar::all(0);
                    int binW = histimg.cols / hsize;
                    Mat buf(1, hsize, CV_8UC3);
                    for( int i = 0; i < hsize; i++ )
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                    cvtColor(buf, buf, COLOR_HSV2BGR);
                    for( int i = 0; i < hsize; i++ )
                    {
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
                        rectangle( histimg, Point(i*binW,histimg.rows),
                                   Point((i+1)*binW,histimg.rows - val),
                                   Scalar(buf.at<Vec3b>(i)), -1, 8 );
                    }
                }
                // Perform CAMShift
                calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
                backproj &= mask;
                IplImage tmp=backproj;
                int trackBox = cvCamShift(&tmp, trackWindow, TermCriteria( TermCriteria::EPS | TermCriteria::COUNT, 10, 1 ), &comp, NULL);
                if( trackWindow.area() <= 1 )
                {
                    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                                  Rect(0, 0, cols, rows);
                }
                if( backprojMode )
                    cvtColor( backproj, image, COLOR_GRAY2BGR );
                //ellipse( image, trackBox, Scalar(0,0,255), 3, 1 );
            }
        }
        else if( trackObject < 0 )
            paused = false;
        if( selectObject && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);
        }
        imshow( "CamShift Demo", image );
        imshow( "Histogram", histimg );
    }
    return 0;
}