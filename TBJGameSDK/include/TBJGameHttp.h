/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#ifndef WASMGAMECONTAINER_TBJGAMEHTTP_H
#define WASMGAMECONTAINER_TBJGAMEHTTP_H

#include "TBJGameMacro.h"

TAOBAO_NS_BEGIN

struct TBJHttpRequest;

typedef enum TBJMethod {
    GET,
    POST,
    PUT,
    DELETE,
    UNKNOWN,
} TBJMethod;

typedef struct TBJHttpRequest* TBJHttpRequestp;

typedef struct TBJHttpResponse {
    TBJHttpRequestp httpRequest;
    int code;
    const char *msg;
    const char *data;
    void* userData;
    void* tmpUserBuffer;
    int tmpUserBufferSize;
    int dataLen;
    bool success;
} TBJHttpResponse;

typedef enum TBJHttpStage {
    OnReceivedData,
    OnReceivedResponse,
    OnRequestComplete,
    OnRequestError
} TBJHttpStage;

typedef struct TBJHttpResponse* TBJHttpResponsep;
typedef void (*TBJResponseCallback)(TBJHttpResponsep response, TBJHttpStage stage);

TBJHttpRequestp TBJNewHttpRequest(const char* url, TBJMethod method);
void TBJFreeHttpRequest(TBJHttpRequestp p);

void TBJAddHeader(TBJHttpRequestp httpRequest, const char *key, const char *value);
void TBJAddContent(TBJHttpRequestp httpRequest, const char* body, int size);
void TBJSendRequest(TBJHttpRequestp p, TBJResponseCallback callBack);
TBJHttpResponsep TBJSyncSendRequest(TBJHttpRequestp p);

void TBJHttpSetUserData(TBJHttpRequestp httpRequest, void* userData);
void TBJHttpSetTimeout(TBJHttpRequestp httpRequest, int timeout);
void TBJSetHttpAuth(TBJHttpRequestp httpRequest, const char *username, const char *password);
const char* TBJGetRequestUrl(TBJHttpRequestp httpRequest);
TBJHttpResponsep TBJGetResponse(TBJHttpRequestp httpRequest);
const char* TBJGetResponseHeader(TBJHttpRequestp httpRequest, const char *key);

TAOBAO_NS_END
#endif //WASMGAMECONTAINER_TBJGAMEHTTP_H
