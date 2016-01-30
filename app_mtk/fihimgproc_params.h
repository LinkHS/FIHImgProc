#ifndef __FIHIMGPROC_PARAMS_H__
#define __FIHIMGPROC_PARAMS_H__

typedef struct
{
    unsigned char               *pAddr;
    int                         frame_type;
    int                         width;
    int                         height;
}FIH_ImageFrame;


/* FIH Image Process Mode */
typedef enum
{
    FIH_FaceBeauty_MODE,
    FIH_LocalHistEqual_MODE,
    FIH_Dehazing_MODE
}FIH_IMGPROC_CMD;


/* Face Beauty Parameters */
typedef struct
{
    /* Face Beauty Parameters */
    int                          fb_level;

    /* Local Histogram Equalization */
}FIH_FaceBeauty_Params;


/* SHDR Parameters
   Initialization:                                       
     FIH_SHDR_Params xxx = { .saturation = 30 };
   */
typedef struct
{
    /* [-100, 100] */
    int                          saturation; //default = 30

    /* Local Histogram Equalization */
}FIH_SHDR_Params;

#endif
