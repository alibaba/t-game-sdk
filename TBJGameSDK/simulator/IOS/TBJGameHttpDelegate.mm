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
    NSFileHandle *_localFile;
    BOOL _needRefreshETag;
}

@property(nonatomic, strong) NSString* url;
@property(nonatomic, strong) NSString* LocalPath;
@property(nonatomic, strong) NSURLConnection* Connection;
@property(nonatomic, strong) NSString* ETag;
@property(nonatomic, strong) NSString* LastModify;

@property (strong) NSMutableData *responseData;
@property (readonly) NSInteger responseCode;
@property (readonly) NSString *statusString;

-(instancetype)initWithNativeTask:(std::shared_ptr<TBJGameHttpTask>) task;

-(std::shared_ptr<TBJGameHttpTask>) getNativeTask;

-(void)startDownloadTask;

-(void)startHttpTask;

@end

@implementation TBJGameNSURLDelegateTask

-(std::shared_ptr<TBJGameHttpTask>) getNativeTask {
    return _task;
}

-(void)startDownloadTask {
    _localFile = NULL;
    self.responseData = NULL;
    
    NSString* parentPath = [self.LocalPath stringByDeletingLastPathComponent];
    NSFileManager* fm =[NSFileManager defaultManager];
    if(![fm fileExistsAtPath:parentPath]) {
        [fm createDirectoryAtPath:parentPath withIntermediateDirectories:YES attributes:nil error:nil];
    }
    
    NSString* localMD5Path = [NSString stringWithFormat:@"%@.version", self.LocalPath];
    
    //read local etag
    FILE* localMD5File = fopen([localMD5Path UTF8String], "r");
    if(localMD5File) {
        struct stat st;
        int ret = stat([localMD5Path UTF8String], &st);
        if(ret != -1) {
            void* local_data = malloc(st.st_size);
            fread(local_data, st.st_size, 1, localMD5File);
            fclose(localMD5File);
            
            NSDictionary* data_dic = [NSJSONSerialization JSONObjectWithData:[NSData dataWithBytes:local_data length:st.st_size] options:0 error:nil];
            
            self.ETag = data_dic[@"ETag"];
            self.LastModify = data_dic[@"LastModify"];
        }
    }
    
    //start to download file
    NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:self.url] cachePolicy:NSURLRequestReloadIgnoringLocalAndRemoteCacheData timeoutInterval:5.0];
    
    
    //set etag
    if(self.ETag && self.LastModify) {
        NSMutableURLRequest* request_muta = [request mutableCopy];
        //[request setValue:self.ETag for:@"If-None-Match"];
        [request_muta setValue:self.ETag forHTTPHeaderField:@"If-None-Match"];
        [request_muta setValue:self.LastModify forHTTPHeaderField:@"If-Modified-Since"];
        request = [request_muta copy];
    }
    
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
    NSString* urlstring = [NSString stringWithUTF8String:_task -> httpRequstInternalp->request.url.c_str()];
    NSURL *url = [NSURL URLWithString:urlstring];
    
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

-(void)refreshETag {
    if(_needRefreshETag) {
        if(_task->status == TBJGameHttpTask::Status::Finished){
            if(self.ETag && self.LastModify) {
                NSString* localMD5Path = [NSString stringWithFormat:@"%@.version", self.LocalPath];
                FILE* localMD5File = fopen([localMD5Path UTF8String], "w+");
                if(localMD5File) {
                    NSDictionary* data_dic = @{@"ETag" :self.ETag, @"LastModify": self.LastModify};
                    NSData *data = [NSJSONSerialization dataWithJSONObject:data_dic options:0 error:0];
                    fwrite(data.bytes, data.length, 1, localMD5File);
                    fclose(localMD5File);
                }
            }
        }
    }
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
    //    if (self.responseData) {
    //        [self.responseData release]
    //    }
    //    if (self.url) {
    //        [self.url release]
    //    }
    //    if (self.LocalPath) {
    //        [self.LocalPath release]
    //    }
    //    if (self.ETag) {
    //        [self.ETag release]
    //    }
    //    if (self.LastModify) {
    //        [self.LastModify release]
    //    }
    //    if (self.responseCode) {
    //        [self.responseCode release]
    //    }
    //    if (self.statusString) {
    //        [self.statusString release]
    //    }
    [self closeLocalFile];
//    [super dealloc];
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
    }
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response {
    // A response has been received, this is where we initialize the instance var you created
    // so that we can append data to it in the didReceiveData method
    // Furthermore, this method is called each time there is a redirect so reinitializing it
    // also serves to clear it
    
    NSHTTPURLResponse* resp = (NSHTTPURLResponse*)response;
    //save etag
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
        self.ETag = dict[@"ETag"];
        self.LastModify = dict[@"Last-Modified"];
        _needRefreshETag = NO;
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
            }
            
            _needRefreshETag = YES;
            NSLog(@"start http connection %@", self.url);
        } else if(resp.statusCode == 304) {
            //remote is same
            _task->localPath = [self.LocalPath UTF8String];
            _task->status = TBJGameHttpTask::Status::Finished;
            [connection cancel];
            NSLog(@"file %@ not modified on remote", self.url);
            return;
        } else if(resp.statusCode == 404) {
            _task->localPath = [self.LocalPath UTF8String];
            _task->status = TBJGameHttpTask::Status::Failed;
            [connection cancel];
            NSLog(@"file %@ not found on remote", self.url);
            return;
        } else {
            _task->status = TBJGameHttpTask::Status::Failed;
            [connection cancel];
            NSLog(@"url %@ reponse code not illegal", self.url);
            return;
        }
    }
    
    _task->status = TBJGameHttpTask::Status::Running;
    
    long long dataSize = [response expectedContentLength];
    NSLog(@"http req received size %lld", dataSize);
    if(dataSize == 0) {
        _task->status = TBJGameHttpTask::Status::Failed;
        [self.Connection cancel];
        return;
    }
    
    if (!_task -> httpReq) {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        if([fileManager fileExistsAtPath:self.LocalPath]) {
            NSError* error;
            [fileManager removeItemAtPath:self.LocalPath error:&error];
            if(error) {
                NSLog(@"tbj game http connection error : %@", error);
                _task->status = TBJGameHttpTask::Status::Failed;
                [self.Connection cancel];
                return;
            }
        }
        
        [fileManager createFileAtPath:self.LocalPath contents:nil attributes:nil];
        
        _localFile = [NSFileHandle fileHandleForWritingAtPath:self.LocalPath];
        if(!_localFile) {
            NSLog(@"open local path %@ failed", self.LocalPath);
            _task->status = TBJGameHttpTask::Status::Failed;
            [self.Connection cancel];
            return;
        }
    }
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
    // Append the new data to the instance variable you declared
    if(_localFile) {
        @try {
            [_localFile writeData:data];
        } @catch (NSException *exception) {
            _localFile = nil;
            _task->status = TBJGameHttpTask::Status::Failed;
            [self.Connection cancel];
        }
    }
    
    
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
    // The request is complete and data has been received
    // You can parse the stuff in your instance variable now
    //wite file to local
    
    [self closeLocalFile];
    if (self.LocalPath) {
        _task->localPath = [self.LocalPath UTF8String];
    }
    
    _task->status = TBJGameHttpTask::Status::Finished;
    
    NSLog(@"http delegator task finished");
}

-(void)closeLocalFile {
    if(_localFile) {
        [_localFile closeFile];
        _localFile = nil;
    }
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
    // The request has failed for some reason!
    // Check the error var
    
    [self closeLocalFile];
    
    NSFileManager* fm =[NSFileManager defaultManager];
    if(![fm fileExistsAtPath:self.LocalPath]) {
        [fm removeItemAtPath:self.LocalPath error:nil];
    }
    
    NSString* e = [NSString stringWithFormat:@"%@", error];
    _task->message = [e UTF8String];
    _task->localPath = [self.LocalPath UTF8String];
    _task->status = TBJGameHttpTask::Status::Failed;
}

@end


@interface TBJGameHttpDelegator() {
    int _taskIDS;
    
    bool _exit;
    bool _exited;
    bool _willExit;
    
    std::mutex _penddinglock;
}

@property(nonatomic, strong) NSString* mSubDir;
@property(nonatomic, strong) NSString* mRemoteURL;
@property(atomic, strong) NSMutableArray* mPendingTasks;
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
        self.mPendingTasks = [[NSMutableArray alloc] init];
        //        self.mPreloadTasks = [[NSMutableDictionary alloc] init];
        _exited = false;
        _exit = false;
        _willExit = false;
        
        unsigned int ncpu;
        size_t len = sizeof(ncpu);
        sysctlbyname("hw.ncpu", &ncpu, &len, NULL, 0);
        if(ncpu > 12) {
            ncpu = 12;
        }
        
        self.mTaskQueue = [[NSOperationQueue alloc] init];
        
        //        __weak __typeof(self)weakSelf = self;
        for(int i = 0; i < ncpu - 1; ++i){
            [self.mTaskQueue addOperationWithBlock:^{
                
                [[NSThread currentThread] setName:@"tbjgame http thread"];
                
                NSRunLoop* curRunLoop = [NSRunLoop currentRunLoop];
                
                //processing task
                TBJGameNSURLDelegateTask* task = nil;
                while (!_exit) {
                    if(task == nil) {
                        //get a task
                        {
                            std::unique_lock<std::mutex> lock(_penddinglock);
                            if(self.mPendingTasks.count > 0) {
                                task = self.mPendingTasks[self.mPendingTasks.count - 1];
                                [self.mPendingTasks removeLastObject];
                            }
                        }
                        
                        if(task != nil) {
                            if ([task getNativeTask]->httpReq) {
                                [task startHttpTask];
                            } else {
                                [task startDownloadTask];
                            }
                            
                        }
                    }
                    
                    if(_willExit) {
                        task = nil;
                    }
                    
                    if(task && [task getNativeTask]->status != TBJGameHttpTask::Status::Running) {
                        NSLog(@"task enter result handle");
                        
                        //update finished tasks
                        //                        [task refreshETag];
                        
                        if ([task getNativeTask]->onDownloadFinished != nullptr) {
                            uint8_t length = (size_t)([[task responseData] length]);
                            void* data = malloc(length);
                            memcpy(data, ([task responseData].mutableBytes), length);
                            
                            TBJGameHttpTask::OnDownloadFinished onDownloadFinished = [task getNativeTask]->onDownloadFinished;
                            onDownloadFinished([task getNativeTask]->taskID, (int) [task responseCode] == 200,
                                               [[task url] UTF8String]
                                               , data, length);
                        }
                        
                        if ([task getNativeTask] -> httpReq) {
                            TBJHttpResponse* response = &([task getNativeTask]->httpRequstInternalp->responseInternal.response);
                            
                            int len = strlen([[task statusString] UTF8String]);
                            char* tmpMsg = (char*) malloc(len + 1);
                            memcpy((void *) tmpMsg, [[task statusString] UTF8String], len);
                            tmpMsg[len] = 0;
                            response->msg = tmpMsg;
                            response->dataLen = (int)([[task responseData] length]);
                            response->data = (const char*)malloc(response->dataLen);
                            memcpy((void *)response->data, ([task responseData].mutableBytes), response->dataLen);
                            response->success = ((int)[task responseCode] == 200);
                            
                            response->code = (int)[task responseCode];
                        }
                        
                        if ([task getNativeTask] -> httpResponseCallback != nullptr) {
                            TBJResponseCallback responseCallback = [task getNativeTask]->httpResponseCallback;
                            TBJHttpResponseInternal* response = &([task getNativeTask]->httpRequstInternalp->responseInternal);
#ifdef JENOVA_SIMULATOR
                            TBJGameInstance* gameInstance = (TBJGameInstance* )[task getNativeTask]->httpRequstInternalp-> gameInstance;
                            
                            TBJHttpStage stage = response->response.success ? TBJHttpStage::OnRequestComplete : TBJHttpStage::OnRequestError;
                            gameInstance -> mHttpCallbackManager.readyToInvoke(response, responseCallback, stage);
#else
                            WasmGameInstance* gameInstance = (WasmGameInstance* )[task getNativeTask]->httpRequstInternalp-> gameInstance;
                            
                            TBJHttpStage stage = response->response.success ? TBJHttpStage::OnRequestComplete : TBJHttpStage::OnRequestError;
                            gameInstance -> mHttpCallbackManager.readyToInvoke(response, responseCallback, stage);
#endif
                        }
                                            
                        //fetch next
                        task = nil;
                    }
                    
                    [curRunLoop runUntilDate:[NSDate date]];
                    
                    [NSThread sleepForTimeInterval:0.001];
                }
                
                NSLog(@"tbj game http thread exited");
            }];
        }
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

    std::unique_lock<std::mutex> lock(_penddinglock);
    [self.mPendingTasks addObject:task];
}

-(size_t)addDownloadTask:(NSString*)urlStr local:(NSString*) path retryCount:(int) retry {
    std::string mat_str = [urlStr UTF8String];
    if(mat_str.find("https://") == 0 || mat_str.find("http://") == 0) {
        NSURL* url = [NSURL URLWithString:urlStr];
    }

    _taskIDS++;
    
    std::shared_ptr<TBJGameHttpTask> task(new TBJGameHttpTask());
    task->taskID = _taskIDS;
    task->retryCount = retry;
    task->status = TBJGameHttpTask::Status::Running;
    task->httpReq = false;
    
    TBJGameNSURLDelegateTask* ocTask = [[TBJGameNSURLDelegateTask alloc] initWithNativeTask:task];
    ocTask.url = urlStr;
    ocTask.LocalPath = path;
        
    //submit task
    [self submit:ocTask];
    
    return (size_t)task.get();
}

-(size_t)addHttpTask:(TBJHttpRequestInternalp) req retryCount:(int) retry {
    _taskIDS++;
    
    std::shared_ptr<TBJGameHttpTask> task(new TBJGameHttpTask());
    task->taskID = _taskIDS;
    task->retryCount = retry;
    task->status = TBJGameHttpTask::Status::Running;
    task->httpRequstInternalp = req;
    task->httpReq = true;
    
    TBJGameNSURLDelegateTask* ocTask = [[TBJGameNSURLDelegateTask alloc] initWithNativeTask:task];
    
    //submit task
    [self submit:ocTask];
    
    return (size_t)task.get();
}

-(void)shutDown {
    //process all pendding tasks
    _willExit = true;
    while(self.mPendingTasks.count > 0) {
        //[self pool];
        [NSThread sleepForTimeInterval: 0.033];
    }
    
    //wait threads finished
    _exit = true;
    [self.mTaskQueue cancelAllOperations];
    [self.mTaskQueue waitUntilAllOperationsAreFinished];

    self.mPendingTasks = nil;
    self.mTaskQueue = nil;
}

@end

TBJGameHttpTask* TBJAddDownloadTask(const std::string &url, const std::string &localPath, int retry, TBJGameHttpTask::OnDownloadFinished onDownloadFinished) {
    NSString* nsUrl = [[NSString alloc] initWithUTF8String:url.c_str()];
    NSString* nsLocalPath = [[NSString alloc] initWithUTF8String:localPath.c_str()];
    
    size_t taskPtr = [[TBJGameHttpDelegator shareInstance] addDownloadTask:nsUrl local:nsLocalPath retryCount:retry];
    TBJGameHttpTask* task = (TBJGameHttpTask*)taskPtr;
    task->onDownloadFinished = onDownloadFinished;
    return task;
}

TBJGameHttpTask* TBJAddHttpTask(TBJHttpRequestInternalp req, int retry) {
    size_t taskPtr = [[TBJGameHttpDelegator shareInstance] addHttpTask:req retryCount:retry];
    TBJGameHttpTask* task = (TBJGameHttpTask*)taskPtr;
    task->httpRequstInternalp = req;
    task->httpResponseCallback = req->request.callBack;
    return task;
}
