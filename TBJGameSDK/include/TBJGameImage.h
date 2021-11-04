/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#ifndef _TAOBAO_JENOVA_IMAGE_H_
#define _TAOBAO_JENOVA_IMAGE_H_

#include "TBJGameMacro.h"

TAOBAO_NS_BEGIN

typedef enum TBJImageFormat
{
    TBJIMAGE_RGBA = 0,
    TBJIMAGE_RGBA_FLOAT,
    TBJIMAGE_RGB,
    TBJIMAGE_R,
    TBJIMAGE_R_FLOAT,
    TBJIMAGE_UNKNOW
} TBJImageFormat;

typedef struct TBJImage
{
    int width;
    int height;
    int channel;
    TBJImageFormat format;
} TBJImage;

TBJImage* TBJGameLoadImage(const char* path);
TBJImage* TBJGameLoadImageFromData(const char* data, int size);
int glTexTBJImage2D(int target, int level, TBJImage* image);
void TBJGameReleaseImage(TBJImage* image);

TAOBAO_NS_END


#endif
