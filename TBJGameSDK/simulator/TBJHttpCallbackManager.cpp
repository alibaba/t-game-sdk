/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */
#include "TBJHttpCallbackManager.h"
#include "TBJGameHttp_Internal.h"


void TBJHttpCallbackManager::readyToInvoke(TBJHttpResponseInternal* response, TBJResponseCallback callback, TBJHttpStage stage) {
    callbackQueue.push(std::make_shared<TBJHttpCallbackTask>(callback, response, stage));
}

void TBJHttpCallbackManager::inovokeHttpCallbacks(void* gameInstance) {
    std::shared_ptr<TBJHttpCallbackTask> ptr;
    while (callbackQueue.pop(ptr))
    {
        if (ptr->callback == nullptr) continue;
        if (ptr->stage == TBJHttpStage::OnReceivedData)
        {
            std::vector<unsigned char> bytes;
            bytes.reserve(1024);
            
            TBJHttpResponseInternal* responceInternal = ptr->response;
            responceInternal->mResponceData.swap(bytes);
            
            if (responceInternal->response.tmpUserBuffer == nullptr)
            {
                ptr->callback(&(responceInternal->response), TBJHttpStage::OnReceivedData);
            }
            else
            {
                for (int i = 0, isize = bytes.size(); i < isize;)
                {
                    int left = isize - i;
                    if (left <= responceInternal->response.tmpUserBufferSize)
                    {
                        responceInternal->response.tmpUserBufferSize = left;
                        memcpy(responceInternal->response.tmpUserBuffer, bytes.data(), left);
                        break;
                    } else
                    {
                        memcpy(responceInternal->response.tmpUserBuffer, bytes.data(), responceInternal->response.tmpUserBufferSize);
                        i += responceInternal->response.tmpUserBufferSize;
                    }
                    ptr->callback(&(responceInternal->response), TBJHttpStage::OnReceivedData);
                }
            }
        } else
        {
            TBJResponseCallback callback = ptr -> callback;
            TBJHttpResponseInternal* response = ptr -> response;
            callback(&(response->response), ptr->stage);
        }
    }
}


