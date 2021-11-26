/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#include <sqlite3.h>
#include "TBJStorageContext.h"

TBJStorageContext::TBJStorageContext(const char* path):mPath(path != NULL ? path : ""), mInited(false), mDb(nullptr), mSelect(nullptr), mRemove(nullptr), mUpdate(nullptr), mClear(nullptr)
{
}


TBJStorageContext::~TBJStorageContext()
{
    this->deinit();
}

void TBJStorageContext::putValue(const char* key, const char* value)
{
    if (key == nullptr || value == nullptr) return;
    if (!mInited)
    {
        init();
    }
    
    int ok = sqlite3_bind_text((sqlite3_stmt*)mUpdate, 1, key, -1, SQLITE_TRANSIENT);
    ok |= sqlite3_bind_text((sqlite3_stmt*)mUpdate, 2, value, -1, SQLITE_TRANSIENT);

    ok |= sqlite3_step((sqlite3_stmt*)mUpdate);
    ok |= sqlite3_reset((sqlite3_stmt*)mUpdate);
    
    if (ok != SQLITE_OK && ok != SQLITE_DONE)
        printf("Error in WasmStorageContext::putValue(%s, %s)\n", key, value);
}
const char* TBJStorageContext::getValue(const char* key)
{
    if (!mInited)
    {
        init();
    }

    int ok = sqlite3_reset((sqlite3_stmt*)mSelect);
    ok |= sqlite3_bind_text((sqlite3_stmt*)mSelect, 1, key, -1, SQLITE_TRANSIENT);
    ok |= sqlite3_step((sqlite3_stmt*)mSelect);
    const unsigned char *text = sqlite3_column_text((sqlite3_stmt*)mSelect, 0);

    if (ok != SQLITE_OK && ok != SQLITE_DONE && ok != SQLITE_ROW)
    {
        printf("Error in WasmStorageContext::getValue(%s)\n", key);
        return nullptr;
    }
    else if (!text)
    {
        return nullptr;
    }
    else
    {
        return reinterpret_cast<const char*>(text);
    }
}

void TBJStorageContext::removeKey(const char *key)
{
    int ok = sqlite3_bind_text((sqlite3_stmt*)mRemove, 1, key, -1, SQLITE_TRANSIENT);
    
    ok |= sqlite3_step((sqlite3_stmt*)mRemove);
    ok |= sqlite3_reset((sqlite3_stmt*)mRemove);
    if (ok != SQLITE_OK && ok != SQLITE_DONE)
        printf("Error in WasmStorageContext::removeKey(%s)\n", key);
}

void TBJStorageContext::clear()
{
    if (!mInited) return;
    int ok = sqlite3_step((sqlite3_stmt*)mClear);
    if( ok != SQLITE_OK && ok != SQLITE_DONE)
        printf("Error in WasmStorageContext::clear()\n");
}

void TBJStorageContext::init()
{
    if (mInited) return;
    mInited = true;
    
    int ret = 0;
    if (mPath.empty())
        ret = sqlite3_open(":memory:", reinterpret_cast<sqlite3 **>(&mDb));
    else
        ret = sqlite3_open(mPath.c_str(), reinterpret_cast<sqlite3 **>(&mDb));

    sqlite3* db = (sqlite3*)mDb;
    const char *sql_createtable = "CREATE TABLE IF NOT EXISTS data(key TEXT PRIMARY KEY,value TEXT);";
    sqlite3_stmt *stmt;
    int ok = sqlite3_prepare_v2(db, sql_createtable, -1, &stmt, nullptr);
    ok |= sqlite3_step(stmt);
    ok |= sqlite3_finalize(stmt);
    
    if (ok != SQLITE_OK && ok != SQLITE_DONE)
    {
        printf("Error in CREATE TABLE\n");
        return;
    }
        
    const char *select = "SELECT value FROM data WHERE key=?;";
    const char *update = "REPLACE INTO data (key, value) VALUES (?,?);";
    const char *remove = "DELETE FROM data WHERE key=?;";
    const char *clear = "DELETE FROM data;";
    ret |= sqlite3_prepare_v2(db, select, -1, reinterpret_cast<sqlite3_stmt**>(&mSelect), nullptr);
    ret |= sqlite3_prepare_v2(db, update, -1, reinterpret_cast<sqlite3_stmt**>(&mUpdate), nullptr);
    ret |= sqlite3_prepare_v2(db, remove, -1, reinterpret_cast<sqlite3_stmt**>(&mRemove), nullptr);
    ret |= sqlite3_prepare_v2(db, clear, -1, reinterpret_cast<sqlite3_stmt**>(&mClear), nullptr);

    if (ret != SQLITE_OK) {
        printf("Error\n");
    }
}

void TBJStorageContext::deinit()
{
    if (mInited)
    {
        mInited = false;
        
        sqlite3_finalize((sqlite3_stmt*)mSelect);
        sqlite3_finalize((sqlite3_stmt*)mRemove);
        sqlite3_finalize((sqlite3_stmt*)mUpdate);
        sqlite3_finalize((sqlite3_stmt*)mClear);
        sqlite3_close((sqlite3*)mDb);
    }
}
