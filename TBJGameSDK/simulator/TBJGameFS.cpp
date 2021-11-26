/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#include "TBJGameFS.h"
#include <stdio.h>
#include <string.h>
#include <atomic>
#include <stdlib.h>
#include <string>
#include <vector>

#ifdef _WIN32
#define HAVE_STRUCT_TIMESPEC 1
#include <Windows.h>
#define sleep Sleep
#endif
#include <pthread.h>

#define DEBUG_MEM_LEAK 0
#if DEBUG_MEM_LEAK
#include <unordered_map>
#include <mutex>
std::mutex lock;
std::unordered_map<void*, size_t> gAllocatedMemMap;
#endif

static void* TBJGameFS_malloc(size_t size)
{
#if DEBUG_MEM_LEAK
	void* p = malloc(size);
	if (!p) return NULL;

	lock.lock();
	gAllocatedMemMap[p] = size;
	lock.unlock();

	return p;
#else
	return malloc(size);
#endif
}

void TBJGameFS_free(void* mem)
{
#if DEBUG_MEM_LEAK
	lock.lock();
	auto it = gAllocatedMemMap.find(mem);
	if (it == gAllocatedMemMap.end())
	{
		TBJGameFS_logError("fatal error !!!\n");
		exit(255);
	}
	gAllocatedMemMap.erase(mem);
	lock.unlock();

	free(mem);
#else
	free(mem);
#endif
}

/*
#import <Foundation/Foundation.h>

// get bundle file full path in iOS
const char* getFullPath(const char* relativePath)
{
    return [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: [NSString stringWithUTF8String: relativePath]].UTF8String;
}

// set root path in iOS
void setRootPath()
{
    std::string rootPath = std::string([[[NSBundle mainBundle] bundlePath] UTF8String]) + "/";
    TBJGameFS_setRootPath(rootPath.c_str());
}
*/

static std::string sRootPath;

void TBJGameFS_setRootPath(const char* path)
{
    TBJGameFS_logInfo("[TBJGameFS_setRootPath] path: %s", path);
	sRootPath = path;
}

static void getFullPath(const char* relativePath, std::string& fullPath)
{
	if (sRootPath.empty())
	{
		fullPath = relativePath;
	}
	else
	{
		fullPath = sRootPath + relativePath;
	}
}

// task id generator
static std::atomic_int sCurTaskId(0);

class BaseTask
{
public:
	void* operator new(size_t size)
	{
		return TBJGameFS_malloc(size);
	}

	void operator delete(void* p)
	{
		TBJGameFS_free(p);
	}

	void destroy()
	{
		delete this;
	}
};

class AsyncTask : public BaseTask
{
public:
	static AsyncTask* create(const char* path, TBJGameFS_AsyncLoadFinished callback, void* userData)
	{
		AsyncTask* task = new AsyncTask();
		task->id = sCurTaskId++;
		task->path = path;
		task->callback = callback;
		task->userData = userData;
		return task;
	}

	//void executeCallback()
	//{

	//}

public:
	int id;
	std::string path;
	TBJGameFS_AsyncLoadFinished callback;
	void* userData;
};

struct BatchAsyncTask : public BaseTask
{
public:
	static BatchAsyncTask* create(const char** paths, int count, 
		TBJGameFS_BatchAsyncLoadProgressChanged progressFn, TBJGameFS_BatchAsyncLoadFinished finishedFn, void* userData)
	{
		BatchAsyncTask* task = new BatchAsyncTask();
		task->id = sCurTaskId++;
		task->paths.resize(count);
		for (int i = 0; i < count; ++i)
		{
			task->paths[i] = paths[i];
		}
		task->count = count;
		task->progressFn = progressFn;
		task->finishedFn = finishedFn;
		task->userData = userData;
		return task;
	}

public:
	int id;
	std::vector<std::string> paths;
	int count;
	TBJGameFS_BatchAsyncLoadProgressChanged progressFn;
	TBJGameFS_BatchAsyncLoadFinished finishedFn;
	void* userData;
};

struct LoadSceneTask : public BaseTask
{
public:
	static LoadSceneTask* create(const char* sceneId, TBJGameFS_onLoadSceneProgressChanged progressFn, TBJGameFS_onLoadSceneFinished finishFn, void* userData)
	{
		LoadSceneTask* task = new LoadSceneTask();
		task->id = -1; // will be set to the id of its async batch loading task
		task->sceneId = sceneId;
		task->progressFn = progressFn;
		task->finishFn = finishFn;
		task->userData = userData;
		return task;
	}

public:
	int id;
	std::string sceneId;
	TBJGameFS_onLoadSceneProgressChanged progressFn;
	TBJGameFS_onLoadSceneFinished finishFn;
	void* userData;
};

static int getFileSize(FILE* f)
{
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	return size;
}

static void runtTask(void*(*fn)(void*), void* userData)
{
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, fn, userData);
}

int TBJGameFS_loadFile(const char* path, void** bufferPtr, int* sizePtr)
{
	std::string fullPath;
	getFullPath(path, fullPath);

	FILE* f = fopen(fullPath.c_str(), "rb");
	if (!f) return 1;

	int size = getFileSize(f);
	if (size <= 0)
	{
		*bufferPtr = NULL;
		*sizePtr = 0;
		fclose(f);
		return 0;
	}

	void* buff = TBJGameFS_malloc(size);
	if (!buff)
	{
		fclose(f);
		return 2;
	}

	int error = 0;
	if (fread(buff, 1, size, f) != size)
	{
		TBJGameFS_free(buff);
		fclose(f);
		return 3;
	}

	*bufferPtr = buff;
	*sizePtr = size;
	fclose(f);
	return 0;
}

int TBJGameFS_batchLoadFilesWithHandler(const char** paths, int count, void*** buffersPtr, int** sizesPtr, 
	TBJGameFS_BatchSyncLoadProgressChanged progressFn, void* userData)
{
	if (count == 0) return 0;

	void** buffers = (void**)TBJGameFS_malloc(sizeof(void*) * count);
	if (!buffers)
	{
		TBJGameFS_logError("fail to malloc buffers");
		return 1;
	}

	int* sizes = (int*)TBJGameFS_malloc(sizeof(int) * count);
	if (!sizes)
	{
		TBJGameFS_logError("fail to malloc sizes buffer");
		TBJGameFS_free(buffers);
		return 2;
	}

	for (int i = 0; i < count; ++i)
	{
		int error = TBJGameFS_loadFile(paths[i], &buffers[i], &sizes[i]);
		if (error != 0)
		{
			TBJGameFS_logError("fail to load file %s", paths[i]);

			// TBJGameFS_free all buffers
			for (int j = 0; j < i; ++j)
			{
				TBJGameFS_free(buffers[j]);
			}
			TBJGameFS_free(buffers);
			TBJGameFS_free(sizes);
			return 1;
		}
		
		if (progressFn)
		{
			progressFn(i + 1, count, userData);
		}
	}

	*buffersPtr = buffers;
	*sizesPtr = sizes;
	return 0;
}

int TBJGameFS_batchLoadFiles(const char** paths, int count, void*** buffersPtr, int** sizesPtr)
{
	return TBJGameFS_batchLoadFilesWithHandler(paths, count, buffersPtr, sizesPtr, NULL, NULL);
}

static void* loadFileAsync(void* userData)
{
	AsyncTask* task = (AsyncTask*)userData;
	void* buffer = NULL;
	int size = 0;
	int error = TBJGameFS_loadFile(task->path.c_str(), &buffer, &size);
	if (task->callback)
	{
		task->callback(task->id, error, task->path.c_str(), buffer, size, task->userData);
	}
	task->destroy();
	return NULL;
}

int TBJGameFS_loadFileAsync(const char* path, TBJGameFS_AsyncLoadFinished callback, void* userData, int* taskId)
{
	if (!path || !callback) return 0;
	
	std::string fullPath;
	getFullPath(path, fullPath);
	
	AsyncTask* task = AsyncTask::create(fullPath.c_str(), callback, userData);
	*taskId = task->id;
	runtTask(loadFileAsync, task);

	return 0;
}

static void OnBatchLoadProgressChanged(int loadedCount, int totalCount, void* userData)
{
	BatchAsyncTask* task = (BatchAsyncTask*)userData;
	task->progressFn(task->id, loadedCount, totalCount, task->userData);
}

// void ConvertToCString(const std::vector<std::string>& paths, )

// batch loading thread
static void* batchLoadFileAsync(void* userData)
{
	// simply load file one by one
	BatchAsyncTask* task = (BatchAsyncTask*)userData;
	void** buffers;
	int* sizes;

	std::vector<const char*> paths;
	paths.resize(task->count);
	for (int i = 0; i < task->count; ++i)
	{
		paths[i] = task->paths[i].c_str();
	}

	int error = TBJGameFS_batchLoadFilesWithHandler(paths.data(), task->count, &buffers, &sizes, task->progressFn ? OnBatchLoadProgressChanged : NULL, task);
	task->finishedFn(task->id, error, paths.data(), buffers, sizes, task->count, task->userData);

	task->destroy();
	return NULL;
}

int TBJGameFS_batchLoadFilesAsync(const char** paths, int count, 
	TBJGameFS_BatchAsyncLoadProgressChanged progressFn, TBJGameFS_BatchAsyncLoadFinished finishedFn, void* userData, int* taskId)
{
	if (!paths || count <= 0) return 0;

	BatchAsyncTask* task = BatchAsyncTask::create(paths, count, progressFn, finishedFn, userData);
	*taskId = task->id;
	runtTask(batchLoadFileAsync, task);

	return 0;
}

void TBJGameFS_freeBuffers(void** buffers, int count)
{
	for (int i = 0; i < count; ++i)
	{
		TBJGameFS_free(buffers[i]);
	}
	TBJGameFS_free(buffers);
}

static void onLoadSceneProgressChanged(int taskId, int loadedCount, int totalCount, void* userData)
{
	LoadSceneTask* task = (LoadSceneTask*)userData;
	task->progressFn(task->sceneId.c_str(), loadedCount, totalCount, task->userData);
}

static void onLoadSceneFinished(int taskId, int error, const char** paths, void** buffer, int* sizes, int count, void* userData)
{
	LoadSceneTask* task = (LoadSceneTask*)userData;
	task->progressFn(task->sceneId.c_str(), count, count, task->userData);
	task->finishFn(task->sceneId.c_str(), 0, task->userData);
	task->destroy();
}

// async load scene thread
static void* loadSceneAsync(void* userData)
{
	LoadSceneTask* task = (LoadSceneTask*)userData;
	
	// load all files in scene
	/*
	const char* paths[] = { "", "" };
	int count = sizeof(paths) / sizeof(const char*);
	int taskId;
	if (TBJGameFS_batchLoadFilesAsync(paths, count, onLoadSceneProgressChanged, onLoadSceneFinished, task, taskId) != 0)
	{
		onLoadSceneFinished(-1, 1, paths, NULL, NULL, 0, userData);
	}
	else
	{
		task->id = taskId;
	}
	*/
	onLoadSceneFinished(-1, 0, NULL, NULL, NULL, 0, task);
	return NULL;
}

int TBJGameFS_EnterScene(const char* sceneId, TBJGameFS_onLoadSceneProgressChanged progressCallback, TBJGameFS_onLoadSceneFinished finishCallback, void* userData)
{
	TBJGameFS_logInfo("[TBJGameFS_EnterScene] sceneId: %s\n", sceneId);

	LoadSceneTask* task = LoadSceneTask::create(sceneId, progressCallback, finishCallback, userData);
	if (!task)
	{
		TBJGameFS_logError("fail to create load scene task\n");
		return 1;
	}

	runtTask(loadSceneAsync, task);

	return 0;
}

void TBJGameFS_ExitScene(const char* sceneId)
{
	TBJGameFS_logInfo("[TBJGameFS_ExitScene] sceneId: %s\n", sceneId);
}
