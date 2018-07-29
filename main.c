#include <signal.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <termios.h>
#include <sys/time.h>
#include <errno.h>
#include <syslog.h>

#include "util.h"

#include "display-kms.h"
#include "v4l2.h"
#include "vpe-common.h"
#include "input_cmd.h"

#include "car_lib.h"
#include "project_config.h"

#define DUMP_MSGQ_KEY           1020
#define DUMP_MSGQ_MSG_TYPE      0x02

/**
  * @brief  Required threads, functions, and structures.\
  */
void * main_thread(void *arg);
void * secondary_thread(void *arg);

uint8_t getMaxBGR_VBGR(uint8_t b, uint8_t g, uint8_t r, uint8_t *V_BGR);
uint8_t getMinBGR(uint8_t b, uint8_t g, uint8_t r);

void BGR24_to_HSV(uint8_t *image_buf);
void detect_Yellow_color(uint8_t *image_buf);

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

    data->input_bufs = calloc(NUMBUF, sizeof(*data->input_bufs));
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
    //unsigned char *addr;
    // Variables for performance measurement
    uint32_t optime = 0;
    struct timeval st;
    struct timeval et;
    
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

        BGR24_to_HSV(image_buf);
        detect_Yellow_color(image_buf);
        memcpy(omap_bo_map(capt->bo[0]), image_buf, VPE_OUTPUT_IMG_SIZE);

        if(pthread_create(&(data->threads[1]), NULL, secondary_thread, data)) {
            MSG("Failed creating Secondary thread");
        }
        pthread_detach(data->threads[1]);
        
        if (disp_post_vid_buffer(vpe->disp, capt, 0, 0, vpe->dst.width, vpe->dst.height)) {
            ERROR("Post buffer failed");
            return NULL;
        }
        /**
        * @breif    Transfer the image to another buffer
        * @caution  rgb24 format
        */

        /**
        * @breif    Transfer the image to another buffer
        * @caution  nv12 format
        unsigned char y_buf[VPE_OUTPUT_W*VPE_OUTPUT_H];
        unsigned char uv_buf[VPE_OUTPUT_W*VPE_OUTPUT_H/2];
        memcpy(y_buf, omap_bo_map(capt->bo[0]), VPE_OUTPUT_W*VPE_OUTPUT_H); // y data
        memcpy(uv_buf, omap_bo_map(capt->bo[1]), VPE_OUTPUT_W*VPE_OUTPUT_H/2); // uv data
        */

        /*
        if(data->dump_state == DUMP_READY) {
            DumpMsg dumpmsg;
            unsigned char* pbuf[4];

            if(get_framebuf(capt, pbuf) == 0) {
                switch(capt->fourcc) {
                    case FOURCC('Y','U','Y','V'):
                        memcpy(data->dump_img_data, pbuf[0], VPE_OUTPUT_IMG_SIZE);
                        break;
                    case FOURCC('N','V','1','2'):
                        memcpy(data->dump_img_data, pbuf[0], VPE_OUTPUT_W*VPE_OUTPUT_H); // y data
                        memcpy(data->dump_img_data+VPE_OUTPUT_W*VPE_OUTPUT_H, pbuf[1], VPE_OUTPUT_W*VPE_OUTPUT_H/2); // uv data
                        break;
                    default :
                        MSG("DUMP.. not yet support format : %.4s\n", (char*)&capt->fourcc);
                        break;
                }
            } else {
                MSG("dump capture buf fail !");
            }

            dumpmsg.type = DUMP_MSGQ_MSG_TYPE;
            dumpmsg.state_msg = DUMP_WRITE_TO_FILE;
            data->dump_state = DUMP_WRITE_TO_FILE;
            if (-1 == msgsnd(data->msgq_id, &dumpmsg, sizeof(DumpMsg)-sizeof(long), 0)) {
                MSG("state:%d, msg send fail\n", dumpmsg.state_msg);
            }
        }
        */
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
    CarLight_Write(ALL_ON);
    usleep(1000000);
    CarLight_Write(ALL_OFF);

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
  * @breif  get Max(B,G,R), Min(B,G,R), V_BGR for BGR24 to HSV
  */
uint8_t getMaxBGR_VBGR(uint8_t b, uint8_t g, uint8_t r, uint8_t *V_BGR) {
    // V_BGR : B = 0, G = 1, R = 2.
    *V_BGR = 0;
    uint8_t max = b;
    if (g > max) { max = g; *V_BGR = 1; }
    if (r > max) { max = r; *V_BGR = 2; }
    return max;
}
uint8_t getMinBGR(uint8_t b, uint8_t g, uint8_t r) {
    uint8_t min = b;
    if (g < min) min = g;
    if (r < min) min = r;
    return min;
}
/**
  * @breif  BGR24 to HSV
  *          Converts image_buffer of bgr24 format to hsv.
  */
void BGR24_to_HSV(uint8_t *image_buf)
{
    int i, j;
    uint8_t temp_buf[VPE_OUTPUT_IMG_SIZE];
    memcpy(temp_buf, image_buf, VPE_OUTPUT_IMG_SIZE);

    uint8_t V_BGR;
    uint8_t B, G, R; int16_t H; uint8_t S; uint8_t V;
    uint8_t Max, Min;

    for(i = 0; i < VPE_OUTPUT_RESOLUTION; i++)
    {
        j = 3*i;
        B = temp_buf[j];    G = temp_buf[j+1];    R = temp_buf[j+2];
        Max = getMaxBGR_VBGR(B, G, R, &V_BGR);
        Min = getMinBGR(B, G, R);// Obtaining V
        V = Max;
        // Obtaining S
        if (V == 0)    S = 0;
        else           S = 255 * (float)(V - Min) / V;
        // Obtaining H
        switch(V_BGR)
        {
            case 0 : H = 240 + (float)60 * (R - G) / (V - Min); break;    // V is Blue
            case 1 : H = 120 + (float)60 * (B - R) / (V - Min); break;    // V is Green
            case 2 : H =       (float)60 * (G - B) / (V - Min); break;    // V is Red
            default : H = 0;                                      break;
        }
        if(H < 0)    H = H + 360;
        H = H / 2;
        image_buf[j] = H; image_buf[j+1] = S; image_buf[j+2] = V;
    }
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
        if( ( minHue < temp_buf[j] && temp_buf[j] < maxHue ) && ( minSat < temp_buf[j+1] && temp_buf[j+1] < maxSat ) )
            image_buf[j] = image_buf[j+1] = image_buf[j+2] = 255;
        else
            image_buf[j] = image_buf[j+1] = image_buf[j+2] = 0;
    }
}
