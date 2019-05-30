#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <android/log.h>

#include "SDL.h"

#define LOG_TAG    "JNILOG"
#undef LOG
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGF(...)  __android_log_print(ANDROID_LOG_FATAL,LOG_TAG,__VA_ARGS__)


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

extern "C"
{
#include "SDL.h"
#include "SDL_thread.h"
};

#define SCREEN_WIDTH					562
#define SCREEN_HEIGHT					1000
#define CAMERA_WIDTH					1920
#define CAMERA_HEIGHT					1080
#define CAMERA_TRANSFORM_WIDTH			1080
#define CAMERA_TRANSFORM_HEIGHT			1920
#define CAMERA_TRANSFORM_LINE_SIZE		1080*3

using namespace cv;
SDL_Window *screen;
SDL_Renderer* sdlRenderer;
Mat camera, temp;
VideoCapture cap;
SDL_Texture	*cameraTexture;
SDL_Texture *picture;

SDL_mutex *mutex;

unsigned char *array;


int read_camera_thread(void *arg)
{
    Mat camera, temp;
    VideoCapture cap;
    //open camera
    cap.open(0);
    cap.set(CV_CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(CV_CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH);//宽度
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT);//高度
    //cap.set(CV_CAP_PROP_FPS, 30);//帧率 帧/秒 设置帧率会很卡
    //cap.set(CV_CAP_PROP_BRIGHTNESS, 128);//亮度 1
    //cap.set(CV_CAP_PROP_CONTRAST, 128);//对比度 40
    //cap.set(CV_CAP_PROP_SATURATION, 128);//饱和度 50

    printf("宽 = %.2f\n", cap.get(CV_CAP_PROP_FRAME_WIDTH));
    printf("高 = %.2f\n", cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    printf("帧率 = %.2f\n", cap.get(CV_CAP_PROP_FPS));
    printf("亮度 = %.2f\n", cap.get(CV_CAP_PROP_BRIGHTNESS));
    printf("对比度 = %.2f\n", cap.get(CV_CAP_PROP_CONTRAST));
    printf("饱和 = %.2f\n", cap.get(CV_CAP_PROP_SATURATION));

    LOGE("camera start Opened");
    if (!cap.isOpened())
    {
        LOGE("camera open error\n");
        return -1;
    }
    LOGE("camera isOpened()");
    while (true)
    {
        LOGE("camera while");
        //read camera data
        cap.read(camera);
        if (camera.empty())
        {
            printf("%s", "Camera data is empty");
            return -1;
        }
        //图片旋转
        transpose(camera, temp);

        //lock
        SDL_LockMutex(mutex);
        if (camera.isContinuous()) {
            array = temp.data;
            LOGE("camera isexec");

        }
        //unlock
        SDL_UnlockMutex(mutex);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    //Create a mutex
    mutex = SDL_CreateMutex();
//    av_register_all();

    //SDL_Init
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    //create window width and height
    screen = SDL_CreateWindow("starphoto", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_HIDDEN);

    if (!screen) {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }

    //create Renderer
    sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
    //image
    SDL_Surface *bmp = SDL_LoadBMP("image.bmp" );
    SDL_SetColorKey(bmp, SDL_TRUE, 0xffffff);
    picture = SDL_CreateTextureFromSurface(sdlRenderer, bmp);
    SDL_SetTextureBlendMode(picture, SDL_BLENDMODE_BLEND);
    //unlock
    cameraTexture= SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STREAMING, CAMERA_TRANSFORM_WIDTH, CAMERA_TRANSFORM_HEIGHT);
    //thread
    SDL_CreateThread(read_camera_thread, "read_camera_thread", NULL);


    SDL_LockMutex(mutex);
    SDL_UpdateTexture(cameraTexture, NULL, array, CAMERA_TRANSFORM_LINE_SIZE);
    SDL_UnlockMutex(mutex);

    SDL_RenderClear(sdlRenderer);

    SDL_RenderCopy(sdlRenderer, cameraTexture, NULL, NULL);
//    SDL_RenderCopy(sdlRenderer, picture, NULL, NULL);

    SDL_RenderPresent(sdlRenderer);

    int i=0;
    while (1)
    {
        ++i;
        SDL_Delay(1000);
        LOGE("main_thread i：%d", i);
        if (i==60*2) {
            break;
        }
    }

    return 0;
}
