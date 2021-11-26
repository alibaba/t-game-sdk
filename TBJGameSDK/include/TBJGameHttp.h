/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#ifndef WASMGAMECONTAINER_TBJGAMEHTTP_H
#define WASMGAMECONTAINER_TBJGAMEHTTP_H

TAOBAO_NS_BEGIN

struct WasmHttpRequest;

typedef enum Method {
    GET,
    POST,
    PUT,
    DELETE,
    UNKNOWN,
};

typedef struct WasmHttpRequest* WasmHttpRequestp;

typedef struct HttpResponse {
    WasmHttpRequest *httpRequest;
    int code;
    char *msg;
    char *data;
    bool success;
};

typedef void (*onResponseCallback)(HttpResponse response);

WasmHttpRequestp newRequest();
void freeRequest(WasmHttpRequestp p);

void wasmAddUrl(WasmHttpRequestp httpRequest, const char* url);
void wasmAddHeader(WasmHttpRequestp httpRequest, const char *key, const char *value);
void wasmAddContent(WasmHttpRequestp httpRequest, const char* body);
void wasmSend(Method method, WasmHttpRequestp p, onResponseCallback callBack);

const char* wasmGetUrl(WasmHttpRequestp httpRequest);

TAOBAO_NS_END
#endif //WASMGAMECONTAINER_TBJGAMEHTTP_H
