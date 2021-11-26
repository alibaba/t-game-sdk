/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#include "TBJEnvContext.h"
#include "TBJStorageContext.h"
#include "TBJGameInstance.h"
#include "TBJAppFrameworkInternal.h"

TBJGameInstance::TBJGameInstance()
{
    tbjGlobalInstance = this;
}

TBJGameInstance::~TBJGameInstance()
{
    if (this->mStorageContext != nullptr)
    {
        delete this->mStorageContext;
        this->mStorageContext = nullptr;
    }
    if (this->mEnvContext != nullptr)
    {
        delete this->mEnvContext;
        this->mEnvContext = nullptr;
    }
    
    if (tbjGlobalInstance == this)
    {
        tbjGlobalInstance = nullptr;
    }
}

#define CHECK_GL_ERROR() ({ GLenum __error = glGetError(); if(__error) printf("OpenGL error 0x%04X in %s %d\n", __error, __FUNCTION__, __LINE__); })
void TBJGameInstance::initGLESFrameBuffer(int glVersion)
{
    glGenFramebuffers(1, &mDefaultFrameBuffer);
    if (mDefaultFrameBuffer == 0)
    {
        printf("Can't create default frame buffer\n");
    }
    glGenRenderbuffers(1, &mDefaultColorBuffer);
    if (mDefaultColorBuffer == 0)
    {
        printf("Can't create default color buffer\n");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFrameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mDefaultColorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mDefaultColorBuffer);
}

bool TBJGameInstance::resizeBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFrameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mDefaultColorBuffer);
    
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &mFrameBufferWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &mFrameBufferHeight);
    CHECK_GL_ERROR();
    
    if (mDefaultDepthBuffer == 0)
    {
        glGenRenderbuffers(1, &mDefaultDepthBuffer);
    }

#ifdef TBJGAME_DESKTOP
#elif defined(__APPLE__)

    glBindRenderbuffer(GL_RENDERBUFFER, mDefaultDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mFrameBufferWidth, mFrameBufferHeight);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDefaultDepthBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDefaultDepthBuffer);

#elif defined(ANDROID) || defined(__ANDROID__)
#endif

    // bind color buffer
    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFrameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mDefaultColorBuffer);
    
    CHECK_GL_ERROR();

    GLenum error;
    if( (error=glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Failed to make complete framebuffer object 0x%X", error);
        return false;
    }
    return true;
}
#undef CHECK_GL_ERROR

void TBJGameInstance::gameLoopOnce(float time)
{
    mEventPool.swap(mExecutingEvent);
    for (int i = 0, size = (int)mExecutingEvent.size(); i < size; i++)
    {
        if (mShouldClose) {
            mExecutingEvent.clear();
            return;
        }
        if (mEventListener != nullptr)
        {
            mEventListener(&(mExecutingEvent[i]));
        }
    }
    mExecutingEvent.clear();
    
    if (mGameLoop != nullptr)
    {
        mGameLoop(time);
    }
}
