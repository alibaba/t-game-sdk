/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */
#include "TBJGameHttp_Internal.h"
#include <cstdlib>
#include "TBJGameHttpTask.h"
#include <string.h>
#include <unistd.h>
#include <chrono>

#ifdef JENOVA_SIMULATOR

#include "TBJAppFrameworkInternal.h"

#else

#if defined(ANDROID) || defined(__ANDROID__)
#include "HttpDelegator.h"

extern "C"
{
    extern JavaVM * _jvm;
}
#endif

#endif

const char *stringFromTBJMethod(enum TBJMethod method) {
static const char *strings[] = { "GET", "POST", "PUT", "DELETE", "UNKNOWN"};
return strings[method];
}

TBJHttpRequestp TBJNewHttpRequestWithGameInstance(const char* url, TBJMethod method, void* gameInstance) {
    TBJHttpRequestInternalp requestInternalp = new TBJHttpRequestInternal();
    
    if (url != nullptr) {
        int len = strlen(url);
        requestInternalp->request.url = url;
    }
    
    requestInternalp->request.method = method;
    requestInternalp->request.body = nullptr;
    requestInternalp->request.callBack = nullptr;
    requestInternalp->request.priority = TBJHttpReqPriority::High;
    requestInternalp->gameInstance = gameInstance;
    requestInternalp->responseInternal.response.tmpUserBuffer = nullptr;
    requestInternalp->responseInternal.response.tmpUserBufferSize = 0;

    requestInternalp->responseInternal.response.httpRequest = &requestInternalp->request;
    return (TBJHttpRequestp)requestInternalp;
}

void TBJHttpSetUserData(TBJHttpRequestp httpRequest, void* userData) {
    if (userData != nullptr) {
        TBJHttpRequestInternalp requestInternalp =  (TBJHttpRequestInternalp) httpRequest;
        requestInternalp->responseInternal.response.userData = userData;
    }
}

void setTBJHttpCallback(TBJHttpRequestp httpRequest, TBJResponseCallback callBack) {
    if (callBack != nullptr) {
        httpRequest->callBack = callBack;
    }
}

void TBJSetHttpAuth(TBJHttpRequestp httpRequest, const char *username, const char *password) {
    if (username != nullptr && password != nullptr) {
        httpRequest -> username = username;
        httpRequest -> password = password;
    }
}

void TBJHttpSetPriority(TBJHttpRequestp httpRequest, TBJHttpReqPriority priority) {
    TBJHttpRequestInternalp reqInternal = (TBJHttpRequestInternalp)httpRequest;
    reqInternal->request.priority = priority;
}

TBJHttpRequestp TBJNewHttpRequest(const char* url, TBJMethod method) {
#ifdef JENOVA_SIMULATOR
    return TBJNewHttpRequestWithGameInstance(url, method, tbjGlobalInstance);
#else
    return TBJNewHttpRequestWithGameInstance(url, method, (void*)1L);
#endif
}

void TBJFreeHttpRequest(TBJHttpRequestp p) {
    TBJHttpRequestInternalp internalp = (TBJHttpRequestInternalp)p;
    
    if (internalp->request.body != nullptr) {
        free((void *) internalp->request.body);
        internalp->request.body = nullptr;
    }
    
    if (internalp->responseInternal.response.msg != nullptr) {
        free((void*)internalp->responseInternal.response.msg);
    }
    if (internalp->responseInternal.response.data != nullptr) {
        free((void*)internalp->responseInternal.response.data);
    }
    delete internalp;
}

void TBJAddHeader(TBJHttpRequestp httpRequest, const char *key, const char *value) {
    if (key != nullptr && value != nullptr) {
        httpRequest->headerMap[key] = value;
    }
}

void TBJAddContent(TBJHttpRequestp httpRequest, const char* body, int size) {
    if (body != nullptr) {
        httpRequest->body = (const char*) malloc(size);
        httpRequest->bodySize = size;
        memcpy((void *) httpRequest->body, body, size);
    }
}

void TBJHttpSetTimeout(TBJHttpRequestp httpRequest, int timeout) {
    httpRequest -> timeout = timeout;
}

#ifdef JENOVA_SIMULATOR
void TBJSetHttpCallback(TBJHttpRequestp httpRequest, TBJResponseCallback callBack)
{
    setTBJHttpCallback(httpRequest, callBack);
}
#endif

void TBJSetHttpCallback(TBJHttpRequestp httpRequest, TBJResponseCallback callBack)
{
    httpRequest->callBack = callBack;
}

void TBJSendRequest(TBJHttpRequestp p, TBJResponseCallback callBack) {
    if (callBack != nullptr)
    {
        setTBJHttpCallback(p, callBack);
    }
#if defined(__APPLE__) || defined(TBJGAME_DESKTOP)
    TBJAddHttpTask((TBJHttpRequestInternalp)p, 1);
#else
    Jenova::HttpDelegator httpDelegator(_jvm);
    httpDelegator.addHttpTask(p, false);
#endif
}

TBJHttpResponsep TBJGetResponse(TBJHttpRequestp httpRequest) {
    TBJHttpRequestInternalp internalp = (TBJHttpRequestInternalp)httpRequest;
    return (TBJHttpResponsep)(&(internalp->responseInternal));
}

const char* TBJGetResponseHeader(TBJHttpRequestp httpRequest, const char *key) {
    TBJHttpRequestInternalp internalp = (TBJHttpRequestInternalp)httpRequest;
    return internalp->responseInternal.responseHeaders[key].c_str();
}

TBJHttpResponsep TBJSyncSendRequest(TBJHttpRequestp p) {
    TBJHttpRequestInternalp reqInternPtr = (TBJHttpRequestInternalp)p;
#if defined(__APPLE__) || defined(TBJGAME_DESKTOP)
    TBJAddHttpTask(reqInternPtr, 1);
    // sec
    long timeout = 10;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    
    bool stop = false;
    while (!stop) {
        // 50ms
        usleep(50 * 1000.0);
        stop = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - begin).count() > timeout;
        stop = stop || (reqInternPtr -> responseInternal.response.code != 0);
    }
#else
    Jenova::HttpDelegator httpDelegator(_jvm);
    httpDelegator.addHttpTask(p, true);
#endif
    return (TBJHttpResponsep)(&reqInternPtr->responseInternal);
}


const char* TBJGetRequestUrl(TBJHttpRequestp httpRequest) {
    return httpRequest->url.c_str();
}
