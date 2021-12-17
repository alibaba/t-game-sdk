/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#ifndef TBJGameImageInternal_h
#define TBJGameImageInternal_h

#include <stdio.h>
#include "TBJGameImage.h"
typedef enum TBJImageTextureFormat
{
    PNG = 0,
    JPG
} TBJImageTextureFormat;

typedef struct TBJImageInternal
{
    TBJImage image;
    int textureFormat;
    void* data;
    const char* path;
    unsigned char* decodeData;
    int decodeLength;
    bool ownMemory = false;

    unsigned int glTextureId;
    int glInternalFormat;
    int glTexture2dType;
    // byte per pixel
    int bpp;
    bool compressed;
    bool hasAlpha;
} TBJImageInternal;

void TBJGameLoadImageFromData_Internal(const char* data, int size, TBJImageInternal* image);

void TBJGameReleaseImage_Internal(TBJImageInternal* image);

void glTexTBJImage2D_Internal(int target, int level, unsigned int glTextureId, TBJImageInternal *image);

void glTexTBJSubImage2D_Internal(int target, int level, int offsetX, int offsetY, int width, int height, TBJImageInternal* image);

#endif /* TBJGameImageInternal_h */
