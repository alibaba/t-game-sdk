/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */
#pragma once


#include <map>
#include <functional>
#include "string"
#include "TBJGameHttp_Internal.h"
#include "vector"
#include "TBJGameFS/TBJGameDLReq.h"

class TBJGameHttpTask {
public:    
    enum Status{
        Running,
        Finished,
        Failed
    };
    uint32_t taskID;
    volatile Status status;
    int retryCount;
    std::string localPath;
    std::string message;
    int index;
    
    TBJGameFSNS::DownloadTask * downloadTask;
    bool httpReq;
    TBJHttpRequestInternalp httpRequstInternalp;
    TBJResponseCallback httpResponseCallback;
};

#if defined(__APPLE__) || defined(TBJGAME_DESKTOP)
TBJGameHttpTask* TBJAddDownloadTask(const std::string &url, const std::string &path, int taskId, int index, int retry, TBJGameFSNS::DownloadTask* downloadTask, TBJHttpReqPriority priority);

TBJGameHttpTask* TBJAddHttpTask(TBJHttpRequestInternalp req, int retry);

void TBJNativeHttpDelegatorShutdown();
#endif
