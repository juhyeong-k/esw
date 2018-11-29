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
extern "C" {
    #include "util.h"
    #include "display-kms.h"
    #include "v4l2.h"
    #include "vpe-common.h"
    #include "input_cmd.h"
    #include "car_lib.h"
}
#include "system_management.h"
#include "car_control.h"
#include "image_processing.h"
#include "cv.h"
#include "calibration.h"

extern std::ofstream fileout;
extern System_resource system_resource;

// Class declaration
BGR24_to_HSV hsvConverter;
Draw draw;

Task currentTask;

#define DUMP_MSGQ_KEY           1020
#define DUMP_MSGQ_MSG_TYPE      0x02

/**
  * @brief  Required threads, functions, and structures.\
  */
bool isWaitingGreen;

void * main_thread(void *arg);
void * CV_thread(void *arg);

uint16_t determine_direction(uint8_t *image_buf);

struct thr_data {
    struct display *disp;
    struct v4l2 *v4l2;
    struct vpe *vpe;
    struct buffer **input_bufs;
    int index;

    int msgq_id;
    bool bfull_screen; // true : 480x272 disp 화면에 맞게 scale 그렇지 않을 경우 false.
    bool bstream_start; // camera stream start 여부
    pthread_t threads[3];
};
struct v4l2 *v4l2;
struct vpe *vpe;
struct thr_data tdata;
struct thr_data *data = &tdata;
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
    PositionControlOnOff_Write(UNCONTROL);
    SpeedControlOnOff_Write(CONTROL);
    DesireSpeed_Write(70);
    // Variables for performance measurement
    uint32_t optime = 0;
    struct timeval st;
    struct timeval et;
    while(1)
    {
        //gettimeofday(&st, NULL);
        //get_result(optime, st, et);
    }
    return NULL;
}
void * CV_handlingThread(void *arg)
{
    /*
    uint8_t display_buf[VPE_OUTPUT_H][VPE_OUTPUT_W][3];
    struct buffer *capt;
    memcpy(display_buf, omap_bo_map(capt->bo[0]), VPE_OUTPUT_IMG_SIZE);
    */
    return NULL;
}
/**
  * @brief  CV_thread, assist the main thread.
  * @param  arg: pointer to parameter of thr_data
  * @retval none
  */
void * CV_thread(void *arg)
{
    struct buffer *capt;
    int i;

    isWaitingGreen = false;
    colorFilter red(RED);
    colorFilter green(GREEN);
    colorFilter yellow(YELLOW);
    colorFilter white(WHITE);

    Navigator navigator;
    Driver driver;

    v4l2_reqbufs(data->v4l2, NUMBUF);
    vpe_input_init(data->vpe);
    allocate_input_buffers(data);
    if(data->vpe->dst.coplanar)    data->vpe->disp->multiplanar = true;
    else                       data->vpe->disp->multiplanar = false;
    printf("disp multiplanar:%d \n", data->vpe->disp->multiplanar);
    vpe_output_init(data->vpe);
    vpe_output_fullscreen(data->vpe, data->bfull_screen);
    // caputre image is used as a vpe input buffer
    for (i = 0; i < NUMBUF; i++)    v4l2_qbuf(data->v4l2,data->vpe->input_buf_dmafd[i], i);
    for (i = 0; i < NUMBUF; i++)    vpe_output_qbuf(data->vpe, i);
    v4l2_streamon(data->v4l2);
    vpe_stream_on(data->vpe->fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    data->vpe->field = V4L2_FIELD_ANY;

    data->index = v4l2_dqbuf(data->v4l2, &data->vpe->field);
    vpe_input_qbuf(data->vpe, data->index);
    vpe_stream_on(data->vpe->fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
    data->bstream_start = true;
    data->index = vpe_output_dqbuf(data->vpe);
    capt = data->vpe->disp_bufs[data->index];
    if (disp_post_vid_buffer(data->vpe->disp, capt, 0, 0, data->vpe->dst.width, data->vpe->dst.height)) {
        ERROR("Post buffer failed");
        return NULL;
    }
    vpe_output_qbuf(data->vpe, data->index);
    data->index = vpe_input_dqbuf(data->vpe);
    v4l2_qbuf(data->v4l2, data->vpe->input_buf_dmafd[data->index], data->index);

    while(1)
    {   
        data->index = v4l2_dqbuf(data->v4l2, &data->vpe->field);
        vpe_input_qbuf(data->vpe, data->index);

        data->index = vpe_output_dqbuf(data->vpe);
        capt = data->vpe->disp_bufs[data->index];

        if (disp_post_vid_buffer(data->vpe->disp, capt, 0, 0, data->vpe->dst.width, data->vpe->dst.height)) {
            ERROR("Post buffer failed");
            return NULL;
        }
        vpe_output_qbuf(data->vpe, data->index);
        data->index = vpe_input_dqbuf(data->vpe);
        v4l2_qbuf(data->v4l2, data->vpe->input_buf_dmafd[data->index], data->index);
    }
    MSG("Ok!");
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
    char* disp_argv[] = {(char*)"dummy", (char*)"-s", (char*)"4:480x272", (char*)"\0"};
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

    vpe->disp = disp_open(3, disp_argv);
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
    currentTask = {0,1};

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
    ret = pthread_create(&tdata.threads[1], NULL, CV_thread, &tdata);
    if(ret) {
        MSG("Failed creating Secondary thread");
    }
    pthread_detach(tdata.threads[1]);

    /* register signal handler for <CTRL>+C in order to clean up */
    if(signal(SIGINT, signal_handler) == SIG_ERR) {
        MSG("could not register signal handler");
        closelog();
        exit(EXIT_FAILURE);
    }

    pause();

    return ret;
}