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
#include <ctype.h>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
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
}
#include "system_management.h"
#include "image_processing.h"
#include "cv.h"
#include "calibration.h"

extern std::ofstream fileout;
extern System_resource system_resource;
using namespace cv;
using namespace std;

#define DUMP_MSGQ_KEY           1020
#define DUMP_MSGQ_MSG_TYPE      0x02

Mat image;
CvConnectedComp comp;
bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection(120,80,120,80);
int vmin = 10, vmax = 256, smin = 30;

/**
  * @brief  Required threads, functions, and structures.\
  */
void * main_thread(void *arg);
void * secondary_thread(void *arg);

uint16_t determine_direction(uint8_t *image_buf);

typedef enum {
    DUMP_NONE,
    DUMP_CMD,
    DUMP_READY,
    DUMP_WRITE_TO_FILE,
    DUMP_DONE
}DumpState;

typedef struct _DumpMsg{
    long type;
    int  state_msg;
}DumpMsg;

struct thr_data {
    struct display *disp;
    struct v4l2 *v4l2;
    struct vpe *vpe;
    struct buffer **input_bufs;

    DumpState dump_state;
    unsigned char dump_img_data[VPE_OUTPUT_IMG_SIZE]; // dump image size

    int msgq_id;
    bool bfull_screen; // true : 480x272 disp 화면에 맞게 scale 그렇지 않을 경우 false.
    bool bstream_start; // camera stream start 여부
    pthread_t threads[2];
};

/**
  * @brief  Alloc vpe input buffer and a new buffer object
  * @param  data: pointer to parameter of thr_data
  * @retval none
  */
static int allocate_input_buffers(struct thr_data *data)
{
    int i;
    struct vpe *vpe = data->vpe;

    data->input_bufs = (buffer**)calloc(NUMBUF, sizeof(*data->input_bufs));
    for(i = 0; i < NUMBUF; i++) {
        data->input_bufs[i] = alloc_buffer(vpe->disp, vpe->src.fourcc, vpe->src.width, vpe->src.height, false);
    }
    if (!data->input_bufs)
        ERROR("allocating shared buffer failed\n");

    for (i = 0; i < NUMBUF; i++) {
        /** Get DMABUF fd for corresponding buffer object */
        vpe->input_buf_dmafd[i] = omap_bo_dmabuf(data->input_bufs[i]->bo[0]);
        data->input_bufs[i]->fd[0] = vpe->input_buf_dmafd[i];
    }
    return 0;
}

/**
  * @brief  Free vpe input buffer and destroy a buffer object
  * @param  buffer: pointer to parameter of buffer object
                  n : count of buffer object
                  bmultiplanar : multipanar value of buffer object
  * @retval none
  */
static void free_input_buffers(struct buffer **buffer, uint32_t n, bool bmultiplanar)
{
    uint32_t i;
    for (i = 0; i < n; i++) {
        if (buffer[i]) {
            close(buffer[i]->fd[0]);
            omap_bo_del(buffer[i]->bo[0]);
            if(bmultiplanar){
                close(buffer[i]->fd[1]);
                omap_bo_del(buffer[i]->bo[1]);
            }
        }
    }
    free(buffer);
}
/**
  * @brief  Main_thread, capture image covert by VPE and call the required threads.
  * @param  arg: pointer to parameter of thr_data
  * @retval none
  */
void get_result(uint32_t optime, struct timeval st, struct timeval et )
{
    gettimeofday(&et, NULL);
    optime = ((et.tv_sec - st.tv_sec)*1000) + ((int)et.tv_usec/1000 - (int)st.tv_usec/1000);
    printf("Operating time : %d.%dms\n", optime, abs((int)et.tv_usec%1000 - (int)st.tv_usec%1000));
}
void * main_thread(void *arg)
{
    struct thr_data *data = (struct thr_data *)arg;
    struct v4l2 *v4l2 = data->v4l2;
    struct vpe *vpe = data->vpe;
    struct buffer *capt;
    bool isFirst = true;
    int index;
    int i;
    // Variables for performance measurement
    uint32_t optime = 0;
    struct timeval st;
    struct timeval et;
    // Class declaration
    BGR24_to_HSV hsvConverter;
    Draw draw;
    colorFilter yellow(YELLOW);
    Navigator navigator(lineDectectionTHRESHOLD);

    v4l2_reqbufs(v4l2, NUMBUF);
    vpe_input_init(vpe);
    allocate_input_buffers(data);
    if(vpe->dst.coplanar)    vpe->disp->multiplanar = true;
    else                       vpe->disp->multiplanar = false;
    printf("disp multiplanar:%d \n", vpe->disp->multiplanar);
    vpe_output_init(vpe);
    vpe_output_fullscreen(vpe, data->bfull_screen);
    // caputre image is used as a vpe input buffer
    for (i = 0; i < NUMBUF; i++)    v4l2_qbuf(v4l2,vpe->input_buf_dmafd[i], i);
    for (i = 0; i < NUMBUF; i++)    vpe_output_qbuf(vpe, i);
    v4l2_streamon(v4l2);
    vpe_stream_on(vpe->fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    vpe->field = V4L2_FIELD_ANY;

    PositionControlOnOff_Write(UNCONTROL);
    SpeedControlOnOff_Write(CONTROL);
    DesireSpeed_Write(50);

    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    Rect trackWindow;
    int hsize = 16;
    float hranges[] = {0,180};
    const float* phranges = hranges;

    while(1)
    {   
        gettimeofday(&st, NULL);
        index = v4l2_dqbuf(v4l2, &vpe->field);
        vpe_input_qbuf(vpe, index);

        if (isFirst) {
        vpe_stream_on(vpe->fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
        isFirst = false;
        MSG("streaming started...");
        data->bstream_start = true;
        }
        index = vpe_output_dqbuf(vpe);
        capt = vpe->disp_bufs[index];

        /** OpenCV tracking 
         *
         */
        Mat image(VPE_OUTPUT_H, VPE_OUTPUT_W, CV_8UC3, omap_bo_map(capt->bo[0]));
        cvtColor(image, hsv, COLOR_BGR2HSV);
        int _vmin = vmin, _vmax = vmax;
        inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)), Scalar(180, 256, MAX(_vmin, _vmax)), mask);
        int ch[] = {0, 0};
        hue.create(hsv.size(), hsv.depth());
        mixChannels(&hsv, 1, &hue, 1, ch, 1);
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
        if( selectObject && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);
        }
        /** END
         *
         */

        uint8_t image_buf[VPE_OUTPUT_H][VPE_OUTPUT_W*3];
        uint8_t display_buf[VPE_OUTPUT_H][VPE_OUTPUT_W*3];
        memcpy(display_buf, omap_bo_map(capt->bo[0]), VPE_OUTPUT_IMG_SIZE);

        if(pthread_create(&(data->threads[1]), NULL, secondary_thread, data)) {
            MSG("Failed creating Secondary thread");
        }
        pthread_detach(data->threads[1]);
        
        if (disp_post_vid_buffer(vpe->disp, capt, 0, 0, vpe->dst.width, vpe->dst.height)) {
            ERROR("Post buffer failed");
            return NULL;
        }
        vpe_output_qbuf(vpe, index);
        index = vpe_input_dqbuf(vpe);
        v4l2_qbuf(v4l2, vpe->input_buf_dmafd[index], index);
        get_result(optime, st, et);
    }
    MSG("Ok!");
    return NULL;
}

/**
  * @brief  secondary_thread, assist the main thread.
  * @param  arg: pointer to parameter of thr_data
  * @retval none
  */
void * secondary_thread(void *arg)
{
    return NULL;
}

static struct thr_data* pexam_data = NULL;
/**
  * @brief  handling an SIGINT(CTRL+C) signal
  * @param  sig: signal type
  * @retval none
  */
void signal_handler(int sig)
{
    if(sig == SIGINT) {
        pthread_cancel(pexam_data->threads[0]);
        pthread_cancel(pexam_data->threads[1]);
        
        msgctl(pexam_data->msgq_id, IPC_RMID, 0);
        
        v4l2_streamoff(pexam_data->v4l2);
        vpe_stream_off(pexam_data->vpe->fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
        vpe_stream_off(pexam_data->vpe->fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
        
        disp_free_buffers(pexam_data->vpe->disp, NUMBUF);
        free_input_buffers(pexam_data->input_bufs, NUMBUF, false);
        
        disp_close(pexam_data->vpe->disp);
        vpe_close(pexam_data->vpe);
        v4l2_close(pexam_data->v4l2);
        fileout.close();
        
        DesireSpeed_Write(0);
        SteeringServoControl_Write(1500);
        printf("-- Project End --\n");
    }
}

int main(int argc, char **argv)
{
	fileout << "System start.\n\n";
    struct v4l2 *v4l2;
    struct vpe *vpe;
    struct thr_data tdata;
    int disp_argc = 3;
    char* disp_argv[] = {"dummy", "-s", "4:480x272", "\0"};
    int ret = 0;

    CarControlInit();
    CarLight_Write(ALL_OFF);
    CameraXServoControl_Write(CAMERA_X_SERVO);
    CameraYServoControl_Write(CAMERA_Y_SERVO);
    SteeringServoControl_Write(1500);
    printf("-- Project Start --\n");
    
    // open vpe
    vpe = vpe_open();
    if(!vpe) {
        return 1;
    }
    // vpe input (v4l cameradata)
    vpe->src.width  = CAPTURE_IMG_W;
    vpe->src.height = CAPTURE_IMG_H;
    describeFormat(CAPTURE_IMG_FORMAT, &vpe->src);

    // vpe output (disp data)
    vpe->dst.width  = VPE_OUTPUT_W;
    vpe->dst.height = VPE_OUTPUT_H;
    describeFormat (VPE_OUTPUT_FORMAT, &vpe->dst);

    vpe->disp = disp_open(disp_argc, disp_argv);
    if (!vpe->disp) {
        ERROR("disp open error!");
        vpe_close(vpe);
        return 1;
    }

    //vpe->deint = 0;
    vpe->translen = 1;

    MSG ("Input(Camera) = %d x %d (%.4s)\nOutput(LCD) = %d x %d (%.4s)",
        vpe->src.width, vpe->src.height, (char*)&vpe->src.fourcc,
        vpe->dst.width, vpe->dst.height, (char*)&vpe->dst.fourcc);

    if (    vpe->src.height < 0 || vpe->src.width < 0 || vpe->src.fourcc < 0 || \
        vpe->dst.height < 0 || vpe->dst.width < 0 || vpe->dst.fourcc < 0) {
        ERROR("Invalid parameters\n");
    }
   
    v4l2 = v4l2_open(vpe->src.fourcc, vpe->src.width, vpe->src.height);
    if (!v4l2) {
        ERROR("v4l2 open error!");
        disp_close(vpe->disp);
        vpe_close(vpe);
        return 1;
    }

    tdata.disp = vpe->disp;
    tdata.v4l2 = v4l2;
    tdata.vpe = vpe;
    tdata.bfull_screen = true;
    tdata.bstream_start = false;

    if(-1 == (tdata.msgq_id = msgget((key_t)DUMP_MSGQ_KEY, IPC_CREAT | 0666))) {
        fprintf(stderr, "%s msg create fail!!!\n", __func__);
        return -1;
    }

    pexam_data = &tdata;

    ret = pthread_create(&tdata.threads[0], NULL, main_thread, &tdata);
    if(ret) {
        MSG("Failed creating main thread");
    }
    pthread_detach(tdata.threads[0]);

    /* register signal handler for <CTRL>+C in order to clean up */
    if(signal(SIGINT, signal_handler) == SIG_ERR) {
        MSG("could not register signal handler");
        closelog();
        exit(EXIT_FAILURE);
    }

    pause();

    return ret;
}