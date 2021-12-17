/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */
#import <Foundation/Foundation.h>

#ifdef JENOVA_SIMULATOR
#import "TBJGameHttp_Internal.h"
#else
#import "TBJGameHttp/TBJGameHttp_Internal.h"
#endif

@interface TBJGameHttpDelegator : NSObject {
   
}

+(instancetype) shareInstance;

-(size_t)addDownloadTask:(NSString*)url local:(NSString*) path retryCount:(int) retry;

-(size_t)addHttpTask:(TBJHttpRequestInternalp) req retryCount:(int) retry;

-(void)shutDown;

@end
