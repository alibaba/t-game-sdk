/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#include "TBJGameImage.h"
#include <cstdlib>
#include <cstdio>
#include "TBJGameImageInternal.h"
#include <cstring>

#ifdef TBJGAME_DESKTOP
#include <OpenGL/gl3.h>
#elif defined(__APPLE__)
#include <OpenGLES/ES3/gl.h>
#else
#include <GLES2/gl2.h>
#endif

TBJImage *TBJGameLoadImage(const char *path) {
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *content = (char *) malloc(size);
    fread(content, 1, size, f);
    TBJImage * image = TBJGameLoadImageFromData(content, size);

    int len = strlen(path);
    image->path = (const char*) malloc(len);
    memcpy((void *) image->path, path, len);

    fclose(f);
    return image;
}

TBJImage *TBJGameLoadImageFromData(const char *data, int size) {
    TBJImageInternal *image = (TBJImageInternal *) malloc(sizeof(TBJImageInternal));
    image->ownMemory = false;
    image->data = (void *) data;
    image->path = nullptr;
    image->textureFormat = 0;
    image->decodeData = nullptr;
    image->glTextureId = 0;
    image->glInternalFormat = 0;
    image->glTexture2dType = 0;
    image->bpp = 0;
    image->compressed = false;
    image->hasAlpha = false;

    TBJGameLoadImageFromData_Internal(data, size, image);
    return (TBJImage *) image;
}

void glTexTBJImage2D(int target, int level, int glTextureId, TBJImage *image) {
    glTexTBJImage2D_Internal(target, level, glTextureId, (TBJImageInternal *) image);
}

void glTBJPixelStorei(TBJImage* image) {
    int param = 1;
    TBJImageInternal * imageInternal = (TBJImageInternal *) image;
    if (!imageInternal->compressed) {
        unsigned int bytesPerRow = imageInternal->image.width * imageInternal->bpp / 8;
        if(bytesPerRow % 8 == 0)
        {
            param = 8;
        }
        else if(bytesPerRow % 4 == 0)
        {
            param = 4;
        }
        else if(bytesPerRow % 2 == 0)
        {
            param = 2;
        }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, param);
}


void TBJGameReleaseImage(TBJImage *image) {
    TBJGameReleaseImage_Internal((TBJImageInternal *) image);
}

void TBJGameReleaseImageCpuMemory(TBJImage *image) {
    TBJImageInternal *imageInternal = (TBJImageInternal *) image;
    if (imageInternal->ownMemory) {
        free(imageInternal->data);
        free(imageInternal->decodeData);
        imageInternal->ownMemory = false;
        imageInternal->data = NULL;
    }
}

void TBJGameReLoadImage(TBJImage *image) {
    if (image->path == nullptr) {
        printf("image path is null\n");
        return;
    }

    FILE *f = fopen(image->path, "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *content = (char *) malloc(size);
    fread(content, 1, size, f);
    TBJGameLoadImageFromData(content, size);
}
