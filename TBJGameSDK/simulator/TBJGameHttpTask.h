/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */
#pragma once


#include <map>
#include <functional>
#include "string"
#include "TBJGameHttp_Internal.h"
#include "vector"

class TBJGameHttpTask {
public:
    typedef std::function<void(int taskId, int error, const std::string& url, void* data, uint8_t length)> OnDownloadFinished;
    
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
    OnDownloadFinished onDownloadFinished;
    
    bool httpReq;
    TBJHttpRequestInternalp httpRequstInternalp;
    TBJResponseCallback httpResponseCallback;
};

#if defined(__APPLE__) || defined(TBJGAME_DESKTOP)
TBJGameHttpTask* TBJAddDownloadTask(const std::string &url, const std::string &localPath, int retry, TBJGameHttpTask::OnDownloadFinished onDownloadFinished);

TBJGameHttpTask* TBJAddHttpTask(TBJHttpRequestInternalp req, int retry);
#endif
