/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#ifndef _TBJGAME_FS_
#define _TBJGAME_FS_

#if _WIN32
#ifdef DEBUG
#define TBJGameFS_logDebug(format, ...) printf("[TBJGameFS Debug] " ## format ## "\n", ##__VA_ARGS__);
#else
#define TBJGameFS_logDebug(format, ...)
#endif
#define TBJGameFS_logInfo(format, ...) printf("[TBJGameFS Info] " ## format ## "\n", ##__VA_ARGS__);
#define TBJGameFS_logError(format, ...) printf("[TBJGameFS Error] " ## format ## "\n", ##__VA_ARGS__);
#else
#ifdef DEBUG
#define TBJGameFS_logDebug(format, ...) printf("[TBJGameFS Debug] "); printf(format, ##__VA_ARGS__); printf("\n");
#else
#define TBJGameFS_logDebug(format, ...) 
#endif
#define TBJGameFS_logInfo(format, ...) printf("[TBJGameFS Info] "); printf(format, ##__VA_ARGS__); printf("\n");
#define TBJGameFS_logError(format, ...) printf("[TBJGameFS Error] "); printf(format, ##__VA_ARGS__); printf("\n");
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	typedef void(*TBJGameFS_BatchSyncLoadProgressChanged)(int loadedCount, int totalCount, void* userData);
	typedef void(*TBJGameFS_AsyncLoadFinished)(int taskId, int error, const char* path, void* buffer, int size, void* userData);
	typedef void(*TBJGameFS_BatchAsyncLoadProgressChanged)(int taskId, int loadedCount, int totalCount, void* userData);
	typedef void(*TBJGameFS_BatchAsyncLoadFinished)(int taskId, int error, const char** paths, void** buffer, int* sizes, int count, void* userData);

	typedef void(*TBJGameFS_onLoadSceneProgressChanged)(const char* sceneId, int loadedCount, int totalCount, void* userData);
	typedef void(*TBJGameFS_onLoadSceneFinished)(const char* sceneId, int errorCode, void* userData);

	void TBJGameFS_setRootPath(const char* path);

	// sync load file
	int TBJGameFS_loadFile(const char* path, void** buffer, int* size);

	// sync batch load
	int TBJGameFS_batchLoadFiles(const char** paths, int count, void*** buffers, int** sizes);

	// sync batch load, support progress handler
	int TBJGameFS_batchLoadFilesWithHandler(const char** paths, int count, void*** buffersPtr, int** sizesPtr, TBJGameFS_BatchSyncLoadProgressChanged progressFn, void* userData);

	// async load file
	int TBJGameFS_loadFileAsync(const char* path, TBJGameFS_AsyncLoadFinished callback, void* userData, int* taskId);

	// async batch load
	int TBJGameFS_batchLoadFilesAsync(const char** paths, int count, TBJGameFS_BatchAsyncLoadProgressChanged progressFn, TBJGameFS_BatchAsyncLoadFinished finishedFn, void* userData, int* taskId);
	
	// async load resources in scene
	int TBJGameFS_EnterScene(const char* sceneId, TBJGameFS_onLoadSceneProgressChanged progressCallback, TBJGameFS_onLoadSceneFinished finishCallback, void* userData);

	// 
	void TBJGameFS_ExitScene(const char* sceneId);

	// free buffers given by callback
	void TBJGameFS_free(void* mem);

	// free buffers given by callback
	void TBJGameFS_freeBuffers(void** buffers, int count);

#ifdef __cplusplus
}
#endif
#endif
