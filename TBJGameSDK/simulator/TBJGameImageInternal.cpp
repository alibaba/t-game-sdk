/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#include "TBJGameImageInternal.h"
#include <cstring>
#include <png.h>
#include <pngconf.h>
#include <cstdlib>
#include <jpeglib.h>
#include <string>
#include <sstream>
#include <vector>
#include "math.h"

#ifdef TBJGAME_DESKTOP
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#elif defined(__APPLE__)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

bool check_png(const unsigned char *data, ssize_t dataLen);

bool check_Jpg(const unsigned char *data, ssize_t dataLen);

void initPngData(const char *data, int size, TBJImageInternal *image);

void initJpgData(const char *data, int size, TBJImageInternal *image);

void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length);

typedef struct {
    const unsigned char *data;
    ssize_t size;
    int offset;
} tImageSource;

void TBJGameLoadImageFromData_Internal(const char *data, int size, TBJImageInternal *image) {
    if (check_png((const unsigned char *) data, size)) {
        initPngData(data, size, image);
    } else if (check_Jpg((const unsigned char *) data, size)) {
        initJpgData(data, size, image);
    } else {
        printf("TBJGameLoadImageFromData_Internal unsupport texture format\n");
    }
}

void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
//        printf("png Read CallBack\n");

    tImageSource *isource = (tImageSource *) png_get_io_ptr(png_ptr);

//        printf("address %d\n", (intptr_t)isource);
//        printf("isource->offset %d\n", isource->offset);
//        printf("isource->size %d\n", isource->size);
//        printf("length %d\n", length);

    if ((int) (isource->offset + length) <= isource->size) {
        memcpy(data, isource->data + isource->offset, length);
        isource->offset += length;
    } else {
        png_error(png_ptr, "pngReaderCallback failed");
    }
}

void initPngData(const char *data, int size, TBJImageInternal *image) {
    image->textureFormat = TBJImageTextureFormat::PNG;
    image->compressed = false;
    bool ret = false;
    const int PNGSIGSIZE = 8;
    png_byte header[PNGSIGSIZE] = {0};
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;

    do {
        // png header len is 8 bytes
        if (size < PNGSIGSIZE) {
            printf("TBJGameLoadImageFromData_Internal size:%d illegal\n", size);
            break;
        }

        // check the data is png or not
        memcpy(header, data, PNGSIGSIZE);
        if (png_sig_cmp(header, 0, PNGSIGSIZE)) {
            printf("TBJGameLoadImageFromData_Internal png_sig_cmp failed\n");
            break;
        }

        // init png_struct
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
        if (!png_ptr) {
            printf("TBJGameLoadImageFromData_Internal png_create_read_struct failed\n");
            break;
        }

        // init png_info
        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            printf("TBJGameLoadImageFromData_Internal png_create_info_struct failed\n");
            break;
        }

//#if (CC_TARGET_PLATFORM != CC_PLATFORM_BADA && CC_TARGET_PLATFORM != CC_PLATFORM_NACL && CC_TARGET_PLATFORM != CC_PLATFORM_TIZEN)
//        CC_BREAK_IF(setjmp(png_jmpbuf(png_ptr)));
//#endif

        // set the read call back function
        tImageSource imageSource;
        imageSource.data = (unsigned char *) data;
        imageSource.size = size;
        imageSource.offset = 0;
        png_set_read_fn(png_ptr, &imageSource, pngReadCallback);

        // read png header info

        // read png file info
        png_read_info(png_ptr, info_ptr);

        image->image.width = png_get_image_width(png_ptr, info_ptr);
        image->image.height = png_get_image_height(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);

        //CCLOG("color type %u", color_type);

        // force palette images to be expanded to 24-bit RGB
        // it may include alpha channel
        if (color_type == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(png_ptr);
        }
        // low-bit-depth grayscale images are to be expanded to 8 bits
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
            bit_depth = 8;
            png_set_expand_gray_1_2_4_to_8(png_ptr);
        }
        // expand any tRNS chunk data into a full alpha channel
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png_ptr);
        }
        // reduce images with 16-bit samples to 8 bits
        if (bit_depth == 16) {
            png_set_strip_16(png_ptr);
        }

        // Expanded earlier for grayscale, now take care of palette and rgb
        if (bit_depth < 8) {
            png_set_packing(png_ptr);
        }

        // update info
        png_read_update_info(png_ptr, info_ptr);

        color_type = png_get_color_type(png_ptr, info_ptr);

        image->compressed = false;
        switch (color_type) {
            case PNG_COLOR_TYPE_GRAY:
                image->image.format = TBJImageFormat::TBJIMAGE_R;
                image->image.channel = 1;
                image->glInternalFormat = GL_LUMINANCE;
                image->glTexture2dType = GL_UNSIGNED_BYTE;
                image->bpp = 8;
                image->hasAlpha = false;
                break;
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                image->image.format = TBJImageFormat::TBJIMAGE_RA;
                image->image.channel = 2;
                image->glInternalFormat = GL_LUMINANCE_ALPHA;
                image->glTexture2dType = GL_UNSIGNED_BYTE;
                image->bpp = 16;
                image->hasAlpha = true;
                break;
            case PNG_COLOR_TYPE_RGB:
                image->image.format = TBJImageFormat::TBJIMAGE_RGB;
                image->image.channel = 3;
                image->glInternalFormat = GL_RGB;
                image->glTexture2dType = GL_UNSIGNED_BYTE;
                image->bpp = 24;
                image->hasAlpha = false;
                break;
            case PNG_COLOR_TYPE_RGB_ALPHA:
                image->image.format = TBJImageFormat::TBJIMAGE_RGBA;
                image->image.channel = 4;
                image->glInternalFormat = GL_RGBA;
                image->glTexture2dType = GL_UNSIGNED_BYTE;
                image->bpp = 32;
                image->hasAlpha = true;
                break;
            default:
                break;
        }

        // read png data
        png_size_t rowbytes;
        png_bytep *row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * image->image.height);

        rowbytes = png_get_rowbytes(png_ptr, info_ptr);

        image->decodeLength = rowbytes * image->image.height;
        image->decodeData = static_cast<unsigned char *>(malloc(image->decodeLength * sizeof(unsigned char)));
        if (!image->decodeData) {
            printf("malloc failed %d\n", (image->decodeLength * sizeof(unsigned char)));

            if (row_pointers != nullptr) {
                free(row_pointers);
            }
            break;
        }

        for (unsigned short i = 0; i < image->image.height; ++i) {
            row_pointers[i] = image->decodeData + i * rowbytes;
        }

        png_read_image(png_ptr, row_pointers);
        png_read_end(png_ptr, nullptr);

//            // premultiplied alpha for RGBA8888
//            if (PNG_PREMULTIPLIED_ALPHA_ENABLED && color_type == PNG_COLOR_TYPE_RGB_ALPHA)
//            {
//                premultipliedAlpha();
//            }

        if (row_pointers != nullptr) {
            free(row_pointers);
        }

        ret = true;
    } while (0);
    
    if (png_ptr) {
        png_destroy_read_struct(&png_ptr, (info_ptr) ? &info_ptr : 0, 0);
    }
}

struct MyErrorMgr {
    struct jpeg_error_mgr pub;  /* "public" fields */
    jmp_buf setjmp_buffer;  /* for return to caller */
};

typedef struct MyErrorMgr *MyErrorPtr;

void myErrorExit(j_common_ptr cinfo) {
/* cinfo->err really points to a MyErrorMgr struct, so coerce pointer */
    MyErrorPtr myerr = (MyErrorPtr) cinfo->err;

/* Always display the message. */
/* We could postpone this until after returning, if we chose. */
/* internal message function can't show error message in some platforms, so we rewrite it here.
 * edit it if has version conflict.
 */
//(*cinfo->err->output_message) (cinfo);
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    printf("jpeg error: %s\n", buffer);

/* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}

void initJpgData(const char *data, int size, TBJImageInternal *image) {
    image->textureFormat = TBJImageTextureFormat::JPG;
    image->compressed = false;
    /* these are standard libjpeg structures for reading(decompression) */
    struct jpeg_decompress_struct cinfo;
    /* We use our private extension JPEG error handler.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct MyErrorMgr jerr;
    /* libjpeg data structure for storing one row, that is, scanline of an image */
    JSAMPROW row_pointer[1] = {0};
    unsigned long location = 0;

    image->compressed = false;

    bool ret = false;
    do {
        /* We set up the normal JPEG error routines, then override error_exit. */
        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = myErrorExit;
        /* Establish the setjmp return context for MyErrorExit to use. */
        if (setjmp(jerr.setjmp_buffer)) {
            /* If we get here, the JPEG code has signaled an error.
             * We need to clean up the JPEG object, close the input file, and return.
             */
            jpeg_destroy_decompress(&cinfo);
            break;
        }

        /* setup decompression process and source, then read JPEG header */
        jpeg_create_decompress(&cinfo);

//#ifndef CC_TARGET_QT5
        void* testData = (void*)data;
        jpeg_mem_src(&cinfo, (unsigned char *)testData, size);

//#endif /* CC_TARGET_QT5 */

        /* reading the image header which contains image information */
#if (JPEG_LIB_VERSION >= 90)
        // libjpeg 0.9 adds stricter types.
        jpeg_read_header(&cinfo, TRUE);
#else
        jpeg_read_header(&cinfo, TRUE);
#endif

        // we only support RGB or grayscale
        if (cinfo.jpeg_color_space == JCS_GRAYSCALE) {
            image->image.format = TBJImageFormat::TBJIMAGE_R;
            image->image.channel = 1;
            image->glInternalFormat = GL_LUMINANCE;
            image->glTexture2dType = GL_UNSIGNED_BYTE;
            image->bpp = 8;
            image->hasAlpha = false;
        } else {
            cinfo.out_color_space = JCS_RGB;

            image->image.format = TBJImageFormat::TBJIMAGE_RGB;
            image->image.channel = 3;
            image->glInternalFormat = GL_RGB;
            image->glTexture2dType = GL_UNSIGNED_BYTE;
            image->bpp = 24;
            image->hasAlpha = false;
        }

        /* Start decompression jpeg here */
        jpeg_start_decompress(&cinfo);

        /* init image info */
        image->image.width = cinfo.output_width;
        image->image.height = cinfo.output_height;

        image->decodeLength = cinfo.output_width * cinfo.output_height * cinfo.output_components;
        image->decodeData = static_cast<unsigned char *>(malloc(image->decodeLength * sizeof(unsigned char)));
        if (!image->decodeData) {
            printf("TBJGameLoadImageFromData_Internal load jpg data failed:%d illegal\n", size);
            break;
        }

        /* now actually read the jpeg into the raw buffer */
        /* read one scan line at a time */
        while (cinfo.output_scanline < cinfo.output_height) {
            row_pointer[0] = image->decodeData + location;
            location += cinfo.output_width * cinfo.output_components;
            jpeg_read_scanlines(&cinfo, row_pointer, 1);
        }

        /* When read image file with broken data, jpeg_finish_decompress() may cause error.
         * Besides, jpeg_destroy_decompress() shall deallocate and release all memory associated
         * with the decompression object.
         * So it doesn't need to call jpeg_finish_decompress().
         */
        //jpeg_finish_decompress( &cinfo );
        jpeg_destroy_decompress(&cinfo);
        /* wrap up decompression, destroy objects, free pointers and close open files */
        ret = true;
    } while (0);
}

bool check_png(const unsigned char *data, ssize_t dataLen) {
    if (dataLen <= 8) {
        return false;
    }

    static const unsigned char PNG_SIGNATURE[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    return memcmp(PNG_SIGNATURE, data, sizeof(PNG_SIGNATURE)) == 0;
}

bool check_Jpg(const unsigned char *data, ssize_t dataLen) {
    if (dataLen <= 4) {
        return false;
    }

    static const unsigned char JPG_SOI[] = {0xFF, 0xD8};

    return memcmp(data, JPG_SOI, 2) == 0;
}

void TBJGameReleaseImage_Internal(TBJImageInternal *image) {
    glDeleteTextures(1, &image->glTextureId);
    image->glTextureId = 0;
}


void glTexTBJImage2D_Internal(int target, int level, unsigned int glTextureId, TBJImageInternal *image) {
    image->glTextureId = glTextureId;
    if (image->compressed) {

        glCompressedTexImage2D(target, level, image->glInternalFormat, image->image.width, image->image.height, 0,
                               image->decodeLength, image->decodeData);
    } else {
        glTexImage2D(target,
                     level,                  // level
                     image->glInternalFormat,            // internal format
                     image->image.width,    // width
                     image->image.height,   // height
                     0,                  // border
                     image->glInternalFormat,            // format
                     image->glTexture2dType,   // type
                     image->decodeData);             // pixel data
    }
}
