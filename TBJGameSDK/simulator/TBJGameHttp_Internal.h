/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */
#ifndef WASMGAMECONTAINER_TBJGAMEHTTP_INTERNAL_H
#define WASMGAMECONTAINER_TBJGAMEHTTP_INTERNAL_H

#include <TBJGameHttp.h>
#include <map>
#include <string>

#ifdef JENOVA_SIMULATOR
#include "Queue.h"
#else
#include "Thread/Queue.h"
#endif

TAOBAO_NS_BEGIN

TBJHttpRequestp TBJNewHttpRequestWithGameInstance(const char* url, TBJMethod method, void* gameInstance);

enum TBJHttpReqPriority {
    VeryLow,
    Low,
    Normal,
    High,
    VeryHigh
};

struct TBJHttpRequest {
    TBJMethod method;
    std::string url;
    const char* body;
    int timeout;
    long bodySize;
    std::string username;
    std::string password;
    std::map<std::string, std::string> headerMap;
    TBJResponseCallback callBack;
    TBJHttpReqPriority priority;
    size_t wasmFunctionIndex;
} ;

const char *stringFromTBJMethod(enum TBJMethod method);

struct TBJHttpResponseInternal {
    TBJHttpResponse response;
    std::map<std::string, std::string> responseHeaders;
    
    WasmGame::Vector<unsigned char> mResponceData;
};

struct TBJHttpRequestInternal {
    TBJHttpRequest request;
    TBJHttpResponseInternal responseInternal;
    void* gameInstance;
    
};

typedef struct TBJHttpRequestInternal* TBJHttpRequestInternalp;

void setTBJHttpCallback(TBJHttpRequestp httpRequest, TBJResponseCallback callBack);

void TBJHttpSetPriority(TBJHttpRequestp httpRequest, TBJHttpReqPriority priority);

//void TBJInovokeHttpCallbacks(void* gameInstance);

TAOBAO_NS_END
#endif //WASMGAMECONTAINER_TBJGAMEHTTP_INTERNAL_H
