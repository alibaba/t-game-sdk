/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */
#import <Foundation/Foundation.h>

#ifdef JENOVA_SIMULATOR
#import "TBJGameHttp_Internal.h"
#else
#import "TBJGameHttp/TBJGameHttp_Internal.h"
#import "TBJGameFS/TBJGameDLReq.h"
#endif

@interface TBJGameHttpDelegator : NSObject {
   
}

+(instancetype) shareInstance;

-(size_t)addDownloadTask:(NSString*)url path:(NSString*)path taskId:(int)taskId index:(int)index retryCount:(int) retry downloadTask:(TBJGameFSNS::DownloadTask*) downloadTask priority:(TBJHttpReqPriority) priority;

-(size_t)addHttpTask:(TBJHttpRequestInternalp) req retryCount:(int) retry;

-(bool)willStop;

-(void)shutDown;

@end
