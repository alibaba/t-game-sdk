/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#include "TBJGameFS.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <unordered_map>

#ifdef _WIN32
#define HAVE_STRUCT_TIMESPEC 1
#include <windows.h>
#define sleep Sleep
#else
#include <unistd.h>
#define sleep(x) usleep((x) * 1000)
#endif

#include <pthread.h>

#define DEBUG_MEM_LEAK 0

static const char* rootPath = "C:/WorkSpace/Jenova/TBJGameFS/data/";
static const char* testFilePaths[] = { "test.txt", "test.bin" };

void testLoadFile()
{
	const char* path = testFilePaths[0];
	void* buffer = NULL;
	int size = 0;
	int error = TBJGameFS_loadFile(path, &buffer, &size);
	if (error == 0)
	{
		printf("succeed reading file, size: %d\n", size);
	}
	else
	{
		printf("fail to load file %s, error: %d\n", path, error);
	}
	TBJGameFS_free(buffer);
}

void onSyncLoadProgressChanged(int loadedCount, int totalCount, void* userData)
{
	printf("[onSyncLoadProgressChanged] progress: %d / %d\n", loadedCount, totalCount);
}

void testBatchLoadFiles()
{
	int count = sizeof(testFilePaths) / sizeof(const char*);
	void** buffers = NULL;
	int* sizes = NULL;
	int error = TBJGameFS_batchLoadFilesWithHandler(testFilePaths, count, &buffers, &sizes, onSyncLoadProgressChanged, NULL);
	if (error == 0)
	{
		for (int i = 0; i < count; ++i)
		{
			printf("succeed reading file %d, size: %d\n", i, sizes[i]);
		}
        TBJGameFS_freeBuffers(buffers, count);
        TBJGameFS_free(sizes);
	}
	else
	{
		printf("fail to load file, error: %d\n", error);
	}
}

void asyncLoadFileCallback(int taskId, int error, const char* path, void* buffer, int size, void* userData)
{
	printf("[asyncLoadFileCallback] thread id %x, taskId: %d, error: %d, size: %d\n", pthread_self(), taskId, error, size);
	if (error == 0)
	{
		TBJGameFS_free(buffer);
	}
}

void testLoadFileAsync()
{
	int taskId;
	int error = TBJGameFS_loadFileAsync(testFilePaths[0], asyncLoadFileCallback, NULL, &taskId);
	printf("[testLoadFileAsync] error: %d, taskId: %d\n", error, taskId);
}

void onBatchAsyncLoadProgressChanged(int taskId, int loadedCount, int totalCount, void* userData)
{
	printf("[onBatchAsyncLoadProgressChanged] thread id %x, taskId: %d, progress: %d / %d\n", pthread_self(), taskId, loadedCount, totalCount);
}

void onBatchAsyncLoadFinished(int taskId, int error, const char** paths, void** buffers, int* sizes, int count, void* userData)
{
	printf("[batchAsyncLoadFileCallback] thread id %x, taskId: %d, error: %d, count: %d\n", pthread_self(), taskId, error, count);
	if (error == 0)
	{
		for (int i = 0; i < count; ++i)
		{
			printf("[onBatchAsyncLoadFinished] file %d size: %d\n", i, sizes[i]);
		}
		TBJGameFS_freeBuffers(buffers, count);
		TBJGameFS_free(sizes);
	}
}

void testbatchLoadFileAsync()
{
	int count = sizeof(testFilePaths) / sizeof(const char*);
	int taskId;
	int error = TBJGameFS_batchLoadFilesAsync(testFilePaths, count, onBatchAsyncLoadProgressChanged, onBatchAsyncLoadFinished, nullptr, &taskId);
	printf("[testbatchLoadFileAsync] error: %d, taskId: %d\n", error, taskId);
}

#if DEBUG_MEM_LEAK
extern std::unordered_map<void*, size_t> gAllocatedMemMap;
#endif

int main()
{
	printf("main thread id: %x\n", pthread_self());

	TBJGameFS_setRootPath(rootPath);

	testLoadFile();
	testBatchLoadFiles();
	testLoadFileAsync();
	testbatchLoadFileAsync();

	sleep(1000);

#if DEBUG_MEM_LEAK
	printf("allocated map size: %d\n", (int)gAllocatedMemMap.size());
#endif


	printf("main\n");
}
