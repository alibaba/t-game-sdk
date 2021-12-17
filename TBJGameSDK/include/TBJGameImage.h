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
    TBJIMAGE_RA,
    TBJIMAGE_R_FLOAT,
    TBJIMAGE_UNKNOW
} TBJImageFormat;

typedef struct TBJImage
{
    int width;
    int height;
    int channel;
    TBJImageFormat format;
    const char* path;
} TBJImage;

TBJImage* TBJGameLoadImage(const char* path);
TBJImage* TBJGameLoadImageFromData(const char* data, int size);
void glTexTBJImage2D(int target, int level, int glTextureId, TBJImage* image);
void glTexTBJSubImage2D(int target, int level, int offsetX, int offsetY, int width, int height, TBJImage* image);
void TBJGameReleaseImage(TBJImage* image);
void TBJGameReleaseImageCpuMemory(TBJImage* image);
void TBJGameReLoadImage(TBJImage* image);
void glTBJPixelStorei(TBJImage* image);

TAOBAO_NS_END


#endif
