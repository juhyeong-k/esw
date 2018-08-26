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
#include "project_config.h"
#include "image_processing.h"

#define DUMP_MSGQ_KEY           1020
#define DUMP_MSGQ_MSG_TYPE      0x02

/**
  * @brief  Required threads, functions, and structures.\
  */
void * main_thread(void *arg);
void * secondary_thread(void *arg);

void detect_Yellow_color(uint8_t *image_buf);
uint16_t determine_direction(uint8_t *image_buf);

std::ofstream fileout("err");

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
    // Address value where the image is stored
    // unsigned char *addr;
    // Variables for performance measurement
    uint32_t optime = 0;
    struct timeval st;
    struct timeval et;
    
    // Class declaration
    BGR24_to_HSV hsvConverter;
    Draw draw;

    v4l2_reqbufs(v4l2, NUMBUF);

    // init vpe input
    vpe_input_init(vpe);

    // allocate vpe input buffer
    allocate_input_buffers(data);

    if(vpe->dst.coplanar)
        vpe->disp->multiplanar = true;
    else
        vpe->disp->multiplanar = false;
    printf("disp multiplanar:%d \n", vpe->disp->multiplanar);

    // init /allocate vpe output, vpe output buffer is used as a frame buffer for display
    vpe_output_init(vpe);
    vpe_output_fullscreen(vpe, data->bfull_screen);

    for (i = 0; i < NUMBUF; i++) {
        // caputre image is used as a vpe input buffer
        v4l2_qbuf(v4l2,vpe->input_buf_dmafd[i], i);
    }

    for (i = 0; i < NUMBUF; i++) {
        vpe_output_qbuf(vpe, i);
    }

    v4l2_streamon(v4l2);
    vpe_stream_on(vpe->fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

    vpe->field = V4L2_FIELD_ANY;
    PositionControlOnOff_Write(UNCONTROL);
    SpeedControlOnOff_Write(CONTROL);
    DesireSpeed_Write(50);
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
        uint8_t image_buf[VPE_OUTPUT_IMG_SIZE];
        memcpy(image_buf, omap_bo_map(capt->bo[0]), VPE_OUTPUT_IMG_SIZE);
        //addr = omap_bo_map(capt->bo[0]);
        //printf("Info : %d \n", *(    (unsigned char*)omap_bo_map(capt->bo[0])    ) );
        //printf("Info : %d \t", *addr);

        hsvConverter.bgr24_to_hsv(image_buf,image_buf);
        detect_Yellow_color(image_buf);

        SteeringServoControl_Write(determine_direction(image_buf));
        draw.horizontal_line(image_buf, UPPER_LINE);
        draw.horizontal_line(image_buf, LOWER_LINE);
        draw.vertical_line(image_buf, 160);

        memcpy(omap_bo_map(capt->bo[0]), image_buf, VPE_OUTPUT_IMG_SIZE);

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
    struct v4l2 *v4l2;
    struct vpe *vpe;
    struct thr_data tdata;
    int disp_argc = 3;
    char* disp_argv[] = {"dummy", "-s", "4:480x272", "\0"};
    int ret = 0;

    CarControlInit();
    CarLight_Write(ALL_OFF);
    CameraYServoControl_Write(CAMERA_Y_SERVO);
    SteeringServoControl_Write(2000);
    printf("-- Project Start --\n");

    tdata.dump_state = DUMP_NONE;
    memset(tdata.dump_img_data, 0, sizeof(tdata.dump_img_data));

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

/**
  * @breif  detect_Yellow_color
  *          Detect yellow from image_buf and save it as bgr24 black and white image.
  */
void detect_Yellow_color(uint8_t *image_buf)
{
    int i, j;
    uint8_t temp_buf[VPE_OUTPUT_IMG_SIZE];
    memcpy(temp_buf, image_buf, VPE_OUTPUT_IMG_SIZE);
    for(i = 0; i < VPE_OUTPUT_RESOLUTION; i++)
    {
        j = 3 * i;
        if( ( yellow_HUE_MIN < temp_buf[j] && temp_buf[j] < yellow_HUE_MAX ) && 
        	( yellow_SAT_MIN < temp_buf[j+1] && temp_buf[j+1] < yellow_SAT_MAX ) &&
        	( yellow_VAL_MIN < temp_buf[j+2] && temp_buf[j+2] < yellow_VAL_MAX) )
            image_buf[j] = image_buf[j+1] = image_buf[j+2] = 255;
        else
            image_buf[j] = image_buf[j+1] = image_buf[j+2] = 0;
    }
}

// DO NOT USE NUMBERS, USE DEFINE VALUE!!!
uint16_t determine_direction(uint8_t *image_buf)
{
    #ifdef bgr24
        float vector = 0;
        uint8_t flag = 0;
        uint16_t temp,i,j,k;
        uint16_t right_high, right_low, left_high, left_low;
        uint32_t index;

        right_high = right_low = left_high = left_low = 0;

        // detect direction from Right UPPER_LINE
        index = UPPER_LINE * VPE_OUTPUT_W * 3 + VPE_OUTPUT_W * 3 / 2;
        for(i = 0; i < VPE_OUTPUT_W / 2; i++)
        {
            j = 3 * i;
            if( image_buf[index + j] )
            {
                temp = 0;
                for(k = 1; k < 10; k++)
                {
                    if( image_buf[index + j + 3*k] )    temp++;
                }
                if(temp > 7)
                {
                    right_high = (( index + j ) % ( VPE_OUTPUT_W * 3 )) / 3;
                    flag += 1;
                    break;
                }
            }
        }
        // detect direction from Right LOWER_LINE
        index = LOWER_LINE * VPE_OUTPUT_W * 3 + VPE_OUTPUT_W * 3 / 2;
        for(i = 0; i < VPE_OUTPUT_W / 2; i++)
        {
            j = 3 * i;
            if( image_buf[index + j] )
            {
                temp = 0;
                for(k = 1; k < 10; k++)    if(image_buf[index + j + k])    temp++;
                if(temp > 7) 
                {
                    right_low = (( index + j ) % ( VPE_OUTPUT_W * 3 )) / 3;
                    flag += 2;
                    break;
                }
            }
        }
        // detect direction from Left UPPER_LINE
        index = UPPER_LINE * VPE_OUTPUT_W * 3 + VPE_OUTPUT_W * 3 / 2;
        for(i = 0; i < VPE_OUTPUT_W / 2; i++)
        {
            j = 3 * i;
            if( image_buf[index - j] )
            {
                temp = 0;
                for(k = 1; k < 10; k++)    if(image_buf[index - j - k])    temp++;
                if(temp > 7)
                {
                    left_high = (( index - j ) % (VPE_OUTPUT_W * 3)) / 3;
                    flag += 4;
                    break;
                }
            }
        }
        // detect direction from Left LOWER_LINE
        index = LOWER_LINE * VPE_OUTPUT_W * 3 + VPE_OUTPUT_W * 3 / 2;
        for(i = 0; i < VPE_OUTPUT_W / 2; i++)
        {
            j = 3 * i;
            if( image_buf[index - j] )
            {
                temp = 0;
                for(k = 1; k < 10; k++)    if(image_buf[index - j - k])    temp++;
                if(temp > 7) 
                {
                    left_low = (( index - j ) % (VPE_OUTPUT_W * 3)) / 3;
                    flag += 8;
                    break;
                }
            }
        }
        if (flag == 15)    {
            vector = (float)(right_high - right_low) / (LOWER_LINE - UPPER_LINE) + (float)(left_high - left_low) / (LOWER_LINE - UPPER_LINE);
        }
        else if ((flag & 1) && (flag & 2))     {
            vector = (float)(right_high - right_low) / (LOWER_LINE - UPPER_LINE);
        }
        else if ((flag & 4) && (flag & 8))     {
            vector = (float)(left_high - left_low) / (LOWER_LINE - UPPER_LINE);
        }
        else    {
            printf("Didn't detected\n");
        }
        printf("vector : %f\n", vector);

        if(vector > 1.11)         return 1000;
        else if (vector < -1.11) return 2000;
        else                       return (uint16_t)(1500 - 450 * vector);
    #endif
}
