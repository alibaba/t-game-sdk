/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */
#ifdef JENOVA_SIMULATOR
#import "TBJGameHttpTask.h"
#include "TBJHttpCallbackManager.h"
#else
#include "TBJGameHttp/TBJGameHttpTask.h"
#endif

#include "TBJGameHttpDelegate.h"
#include <vector>

#import <Foundation/Foundation.h>

#include <regex>
#include <map>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/sysctl.h>

#ifdef JENOVA_SIMULATOR
#include "TBJGameInstance.h"
#else
#include "WasmGameInstance.h"
#endif

@interface TBJGameNSURLDelegateTask : NSObject<NSURLConnectionDelegate>{
    std::shared_ptr<TBJGameHttpTask> _task;
}

@property(nonatomic) NSOperationQueuePriority priority;

@property (nonatomic) NSInteger contentLength;

@property(nonatomic, strong) NSString* url;
@property(nonatomic, strong) NSURLConnection* Connection;

@property (strong) NSMutableData *responseData;
@property (readonly) NSInteger responseCode;
@property (readonly) NSString *statusString;

-(instancetype)initWithNativeTask:(std::shared_ptr<TBJGameHttpTask>) task;

-(std::shared_ptr<TBJGameHttpTask>) getNativeTask;

-(void)startDownloadTask;

-(void)startHttpTask;

@end

@interface TBJGameHttpOperation : NSOperation {
@public
    TBJGameNSURLDelegateTask *delegateTask;
}
@end

@implementation TBJGameHttpOperation
-(void) main
{
    [[NSThread currentThread] setName:@"tbjgame http thread"];
    
    NSRunLoop* curRunLoop = [NSRunLoop currentRunLoop];
    
    TBJGameNSURLDelegateTask* task = self -> delegateTask;
    
    if ([[TBJGameHttpDelegator shareInstance] willStop]) {
        return;
    }
    
    if ([task getNativeTask]->httpReq) {
        [task startHttpTask];
    } else {
        [task startDownloadTask];
    }
    
    [curRunLoop run];
    NSLog(@"current runloop exit, %@\n", [[NSThread currentThread] name]);
}
@end

@implementation TBJGameNSURLDelegateTask


-(void) handleResult {
    NSLog(@"task enter result handler");

    if ([self getNativeTask]->downloadTask != nullptr) {
        uint8_t length = (size_t)([[self responseData] length]);
        TBJGameFSNS::Buffer buffer(length);
        memcpy(buffer.getData(), ([self responseData].mutableBytes), length);

        [self getNativeTask]->downloadTask->onFinishedDownload(std::move(buffer), [self getNativeTask]->localPath, [self getNativeTask]->index, (int) [self responseCode] == 200 ? 0 : -1);
    }

    if ([self getNativeTask] -> httpReq) {
        TBJHttpResponse* response = &([self getNativeTask]->httpRequstInternalp->responseInternal.response);

        int len = strlen([[self statusString] UTF8String]);
        if (len > 0) {
            char* tmpMsg = (char*) malloc(len + 1);
            memcpy((void *) tmpMsg, [[self statusString] UTF8String], len);
            tmpMsg[len] = 0;
            response->msg = tmpMsg;
        }
        response->dataLen = (int)([[self responseData] length]);
        if (response->dataLen > 0) {
            response->data = (const char*)malloc(response->dataLen);
            memcpy((void *)response->data, ([self responseData].mutableBytes), response->dataLen);
        }
        response->success = ((int)[self responseCode] == 200);
        response->code = (int)[self responseCode];
    }

    if ([self getNativeTask] -> httpResponseCallback != nullptr) {
        TBJResponseCallback responseCallback = [self getNativeTask]->httpResponseCallback;
        TBJHttpResponseInternal* response = &([self getNativeTask]->httpRequstInternalp->responseInternal);
#ifdef JENOVA_SIMULATOR
        TBJGameInstance* gameInstance = (TBJGameInstance* )[self getNativeTask]->httpRequstInternalp-> gameInstance;

        TBJHttpStage stage = response->response.success ? TBJHttpStage::OnRequestComplete : TBJHttpStage::OnRequestError;
        gameInstance -> mHttpCallbackManager.readyToInvoke(response, responseCallback, stage);
#else
        WasmGameInstance* gameInstance = (WasmGameInstance* )[self getNativeTask]->httpRequstInternalp-> gameInstance;

        TBJHttpStage stage = response->response.success ? TBJHttpStage::OnRequestComplete : TBJHttpStage::OnRequestError;
        gameInstance -> mHttpCallbackManager.readyToInvoke(response, responseCallback, stage);
#endif
    }
}

-(std::shared_ptr<TBJGameHttpTask>) getNativeTask {
    return _task;
}

-(void)startDownloadTask {
    self.responseData = NULL;
    
    //start to download file
    NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:self.url] cachePolicy:NSURLRequestReloadIgnoringLocalAndRemoteCacheData timeoutInterval:5.0];
    
    
    self.Connection = [[NSURLConnection alloc]initWithRequest:request delegate:self startImmediately:NO];
    
    [self.Connection scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    
    [self.Connection start];
}

-(void)startHttpTask {
    NSString* requestType = nil;
    TBJMethod method = _task -> httpRequstInternalp -> request.method;
    // Process the request -> get response packet
    switch (method)
    {
        case TBJMethod::GET: // HTTP GET
            requestType = @"GET";
            break;
            
        case TBJMethod::POST: // HTTP POST
            requestType = @"POST";
            break;
            
        case TBJMethod::PUT:
            requestType = @"PUT";
            break;
            
        case TBJMethod::DELETE:
            requestType = @"DELETE";
            break;
        default:
            NSLog(@"tbj http delegator not support method %d", (int)method);
            break;
    }
    
    //create request with url
    self.url = [NSString stringWithUTF8String:_task -> httpRequstInternalp->request.url.c_str()];
    NSURL *url = [NSURL URLWithString:self.url];
    NSMutableURLRequest *nsrequest = [NSMutableURLRequest requestWithURL:url
                                                             cachePolicy:NSURLRequestReloadIgnoringLocalAndRemoteCacheData
                                                         timeoutInterval:_task -> httpRequstInternalp->request.timeout];
    
    //set request type
    [nsrequest setHTTPMethod:requestType];
    
    /* get custom header data (if set) */
    std::map<std::string, std::string> headers = _task -> httpRequstInternalp -> request.headerMap;
    if(!headers.empty())
    {
        /* append custom headers one by one */
        for (auto& header : headers)
        {
            const char* field = header.first.c_str();
            const char* value = header.second.c_str();
            NSString *headerField = [NSString stringWithUTF8String:field];
            NSString *headerValue = [NSString stringWithUTF8String:value];
            [nsrequest setValue:headerValue forHTTPHeaderField:headerField];
        }
    }
    
    //if request type is post or put,set header and data
    if([requestType  isEqual: @"POST"] || [requestType isEqual: @"PUT"])
    {
        const char* requestDataBuffer = _task -> httpRequstInternalp->request.body;
        if (nullptr !=  requestDataBuffer)
        {
            NSData *postData = [NSData dataWithBytes:requestDataBuffer length:_task -> httpRequstInternalp->request.bodySize];
            [nsrequest setHTTPBody:postData];
        }
    }
    
    self.responseData = [NSMutableData data];
    
    // create the connection with the target request and this class as the delegate
    self.Connection = [[NSURLConnection alloc]initWithRequest:nsrequest delegate:self startImmediately:NO];
    
    [self.Connection scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
    // start the connection
    [self.Connection start];
}

-(instancetype)initWithNativeTask:(std::shared_ptr<TBJGameHttpTask>)task {
    if(self = [super init]) {
        _task = task;
    }
    return self;
}

-(void)dealloc {
    _task = nil;
    if(self.Connection) {
        [self.Connection cancel];
        self.Connection = nil;
    }
}

#pragma mark NSURLConnection Delegate Methods
- (void)connection:(NSURLConnection *)connection didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge {
    if ([challenge previousFailureCount] == 0) {
        NSLog(@"received authentication challenge");
        NSString* username = [NSString stringWithFormat:@"%s", _task->httpRequstInternalp->request.username.c_str()];
        NSString* password = [NSString stringWithFormat:@"%s", _task->httpRequstInternalp->request.password.c_str()];
        
        NSURLCredential *newCredential = [NSURLCredential credentialWithUser:username
                                                                    password:password
                                                                 persistence:NSURLCredentialPersistenceForSession];
        NSLog(@"credential created");
        [[challenge sender] useCredential:newCredential forAuthenticationChallenge:challenge];
        NSLog(@"responded to authentication challenge");
    }
    else {
        NSLog(@"previous authentication failure");
        _task->status = TBJGameHttpTask::Status::Failed;
        [self handleResult];    }
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response {
    // A response has been received, this is where we initialize the instance var you created
    // so that we can append data to it in the didReceiveData method
    // Furthermore, this method is called each time there is a redirect so reinitializing it
    // also serves to clear it
    
    NSHTTPURLResponse* resp = (NSHTTPURLResponse*)response;
    if(resp) {
        _responseCode = resp.statusCode;
        _statusString = [NSHTTPURLResponse localizedStringForStatusCode:_responseCode];
        
        NSDictionary* headers = [(NSHTTPURLResponse *)response allHeaderFields];
        
        for (id key in headers) {
            //注意对于字典for遍历循环的是key
            TBJHttpResponseInternal* tbjHttpResponsePtr = &(_task -> httpRequstInternalp -> responseInternal);
            
            NSString* kNsString = [NSString stringWithFormat:@"%@", key];
            std::string k = [kNsString UTF8String];
            NSString* vNsString = [NSString stringWithFormat:@"%@", [headers objectForKey:key]];
            std::string v = [vNsString UTF8String];
            tbjHttpResponsePtr->responseHeaders[k] = v;
        }
        
        NSDictionary* dict = [resp allHeaderFields];
        if(resp.statusCode == 200) {
            if (_task->httpReq) {
                TBJResponseCallback responseCallback = _task->httpResponseCallback;
                TBJHttpResponseInternal* response = &(_task->httpRequstInternalp->responseInternal);
#ifdef JENOVA_SIMULATOR
                TBJGameInstance* gameInstance = (TBJGameInstance* )_task->httpRequstInternalp-> gameInstance;
                
                gameInstance -> mHttpCallbackManager.readyToInvoke(response, responseCallback,  TBJHttpStage::OnReceivedResponse);
#else
                WasmGameInstance* gameInstance = (WasmGameInstance* )_task->httpRequstInternalp-> gameInstance;
                
                gameInstance -> mHttpCallbackManager.readyToInvoke(response, responseCallback, TBJHttpStage::OnReceivedResponse);
#endif
            } else {
                self.contentLength = [response expectedContentLength];
                if (self.contentLength == -1) {
                    // default value
                    self.contentLength = 30 * 1024 * 1024;
                }
            }
            
            NSLog(@"start http connection %@, %d", self.url, _task->taskID);
        } else if(resp.statusCode == 404) {
            _task->status = TBJGameHttpTask::Status::Failed;
            [connection cancel];
            NSLog(@"file %@ not found on remote", self.url);
            [self handleResult];
            return;
        } else {
            _task->status = TBJGameHttpTask::Status::Failed;
            [connection cancel];
            NSLog(@"url %@ reponse code not illegal", self.url);
            [self handleResult];
            return;
        }
    }
    
    _task->status = TBJGameHttpTask::Status::Running;
    
    long long dataSize = [response expectedContentLength];
    NSLog(@"http req received size %lld", dataSize);
    if(dataSize == 0) {
        _task->status = TBJGameHttpTask::Status::Failed;
        [self.Connection cancel];
        [self handleResult];
        return;
    }
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
    if (_task->httpReq) {
        TBJHttpResponseInternal* responsePtr = &(_task -> httpRequstInternalp -> responseInternal);
        
        responsePtr->mResponceData.extend((unsigned char*)data.bytes, ((unsigned char*)(data.bytes)) + data.length);
        
        TBJResponseCallback responseCallback = _task->httpResponseCallback;
#ifdef JENOVA_SIMULATOR
        TBJGameInstance* gameInstance = (TBJGameInstance* )_task->httpRequstInternalp-> gameInstance;
        
        gameInstance -> mHttpCallbackManager.readyToInvoke(responsePtr, responseCallback, TBJHttpStage::OnReceivedData);
#else
        WasmGameInstance* gameInstance = (WasmGameInstance* )_task->httpRequstInternalp-> gameInstance;
        
        gameInstance -> mHttpCallbackManager.readyToInvoke(responsePtr, responseCallback, TBJHttpStage::OnReceivedData);
#endif
    } else {
        [self getNativeTask] -> downloadTask->updateProgress(self.responseData.length + [data length], [self contentLength]);
    }
    
    // read to memory
    [self.responseData appendData:data];
}

- (NSCachedURLResponse *)connection:(NSURLConnection *)connection
                  willCacheResponse:(NSCachedURLResponse*)cachedResponse {
    // Return nil to indicate not necessary to store a cached response for this connection
    return nil;
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection {
    _task->status = TBJGameHttpTask::Status::Finished;
    [self handleResult];
    NSLog(@"http delegator task finished");
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
    
    NSString* e = [NSString stringWithFormat:@"%@", error];
    _task->message = [e UTF8String];
    _task->status = TBJGameHttpTask::Status::Failed;
    [self handleResult];
}

@end


@interface TBJGameHttpDelegator() {
}

@property(nonatomic) bool willExit;
@property(atomic) int taskIdCount;
@property(nonatomic, strong) NSOperationQueue* mTaskQueue;
@end

@implementation TBJGameHttpDelegator;

static TBJGameHttpDelegator* gInstance = nil;

+(instancetype) shareInstance
{
    if(gInstance == nil) {
        gInstance = [[super allocWithZone:NULL] init] ;
    }
    return gInstance ;
}

class WasmGameInstance;

-(instancetype) init {
    if(self = [super init]) {
        self.willExit = false;
        
        unsigned int ncpu;
        size_t len = sizeof(ncpu);
        sysctlbyname("hw.ncpu", &ncpu, &len, NULL, 0);
        if(ncpu > 12) {
            ncpu = 12;
        }
        
        self.mTaskQueue = [[NSOperationQueue alloc] init];
        [self.mTaskQueue setMaxConcurrentOperationCount:ncpu - 1];
    }
    return self;
}

//-(void)dealloc {
////    [super dealloc];
//}

-(void)submit:(TBJGameNSURLDelegateTask*)task {
    
    NSString *key = [NSString stringWithFormat:@"%d", [task getNativeTask]->taskID];
    
    if(_exit) {
        [task getNativeTask]->retryCount = 0;
    }
        
    TBJGameHttpOperation* operation = [[TBJGameHttpOperation alloc] init];
    [operation setQueuePriority: [task priority]];

    operation -> delegateTask = task;
    [self.mTaskQueue addOperation:operation];
    
}

-(size_t)addDownloadTask:(NSString*)urlStr path:(NSString*)path taskId:(int) taskId index:(int)index retryCount:(int) retry downloadTask:(TBJGameFSNS::DownloadTask*) downloadTask priority:(TBJHttpReqPriority) priority {
    std::string mat_str = [urlStr UTF8String];
    if(mat_str.find("https://") == 0 || mat_str.find("http://") == 0) {
        NSURL* url = [NSURL URLWithString:urlStr];
    }
    
    std::shared_ptr<TBJGameHttpTask> task(new TBJGameHttpTask());
    task->taskID = taskId;
    task->retryCount = retry;
    task->status = TBJGameHttpTask::Status::Running;
    task->httpReq = false;
    task->downloadTask = downloadTask;
    task->index = index;
    task->localPath = [path UTF8String];
    
    TBJGameNSURLDelegateTask* ocTask = [[TBJGameNSURLDelegateTask alloc] initWithNativeTask:task];
    ocTask.priority = [self mapRequestPriority:priority];
    ocTask.url = urlStr;
    
    //submit task
    [self submit:ocTask];
    
    return (size_t)task.get();
}

-(bool)willStop {
    return self.willExit;
}

-(NSOperationQueuePriority) mapRequestPriority:(TBJHttpReqPriority) priority {
    NSOperationQueuePriority nsOperationQueuePriority = NSOperationQueuePriorityNormal;
    if (priority == TBJHttpReqPriority::High) {
        nsOperationQueuePriority = NSOperationQueuePriorityHigh;
    } else if (priority == TBJHttpReqPriority::VeryHigh) {
        nsOperationQueuePriority = NSOperationQueuePriorityVeryHigh;
    } else if (priority == TBJHttpReqPriority::Normal) {
        nsOperationQueuePriority = NSOperationQueuePriorityNormal;
    } else if (priority == TBJHttpReqPriority::Low) {
        nsOperationQueuePriority = NSOperationQueuePriorityLow;
    } else if (priority == TBJHttpReqPriority::VeryLow) {
        nsOperationQueuePriority = NSOperationQueuePriorityVeryLow;
    }
    return nsOperationQueuePriority;
}

-(size_t)addHttpTask:(TBJHttpRequestInternalp) req retryCount:(int) retry {
    std::shared_ptr<TBJGameHttpTask> task(new TBJGameHttpTask());
    task->taskID = ++self.taskIdCount;
    task->retryCount = retry;
    task->status = TBJGameHttpTask::Status::Running;
    task->httpRequstInternalp = req;
    task->httpReq = true;
    
    TBJGameNSURLDelegateTask* ocTask = [[TBJGameNSURLDelegateTask alloc] initWithNativeTask:task];
    ocTask.priority = [self mapRequestPriority:req->request.priority];
    
    //submit task
    [self submit:ocTask];
    
    return (size_t)task.get();
}

-(void)shutDown {
    _willExit = true;
    [self.mTaskQueue cancelAllOperations];
    [self.mTaskQueue waitUntilAllOperationsAreFinished];
    
    self.mTaskQueue = nil;
}

@end

TBJGameHttpTask* TBJAddDownloadTask(const std::string &url, const std::string &path, int taskId, int index, int retry, TBJGameFSNS::DownloadTask* downloadTask, TBJHttpReqPriority priority) {
    NSString* nsUrl = [[NSString alloc] initWithUTF8String:url.c_str()];
    NSString* nsLocalPath = [[NSString alloc] initWithUTF8String:path.c_str()];
    
    size_t taskPtr = [[TBJGameHttpDelegator shareInstance] addDownloadTask:nsUrl path:nsLocalPath taskId:taskId index:index retryCount:retry downloadTask:downloadTask priority:priority];
    TBJGameHttpTask* task = (TBJGameHttpTask*)taskPtr;
    return task;
}

TBJGameHttpTask* TBJAddHttpTask(TBJHttpRequestInternalp req, int retry) {
    size_t taskPtr = [[TBJGameHttpDelegator shareInstance] addHttpTask:req retryCount:retry];
    TBJGameHttpTask* task = (TBJGameHttpTask*)taskPtr;
    task->httpRequstInternalp = req;
    task->httpResponseCallback = req->request.callBack;
    return task;
}

void TBJNativeHttpDelegatorShutdown() {
    [[TBJGameHttpDelegator shareInstance] shutDown];
}
