/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#include "TBJEnvContext.h"

const char* TBJEnvContext::DRIVER = "driver";
const char* TBJEnvContext::GLVERSION = "glversion";
const char* TBJEnvContext::APPNAME = "app";
const char* TBJEnvContext::GAMEURL = "game";
const char* TBJEnvContext::ORIENTATION = "orientation";

int TBJEnvContext::getInt(const char* key)
{
    const char* str = getString(key);
    return str == NULL ? 0 : atoi(str);
}

void TBJEnvContext::put(const char* key, const char* value)
{
    if (key != NULL && strlen(key) > 0 && value != NULL && strlen(value) > 0)
    {
        mConfig[key] = value;
    }
}

const char* TBJEnvContext::getString(const char* key)
{
    auto iter = mConfig.find(key);
    if (iter != mConfig.end())
    {
        return iter->second.c_str();
    }
    return NULL;
}

bool TBJEnvContext::parseFromConifg(const char* curl)
{
    if (curl == nullptr || strlen(curl) == 0) return false;
    
    std::string url = curl;
    
    auto index = url.find_first_of("?");
    if (index != std::string::npos)
    {
        url = url.substr(index + 1);
    }
    
    std::string key, value;
    while (url.length() > 0)
    {
        auto keyIndex = url.find_first_of("=");
        if (keyIndex != std::string::npos)
        {
            key = url.substr(0, keyIndex);
            url = url.substr(keyIndex + 1);
            
            auto valueIndex = url.find_first_of("&");
            if (valueIndex != std::string::npos)
            {
                value = url.substr(0, valueIndex);
                url = url.substr(valueIndex + 1);
            } else
            {
                value = url;
                url = "";
            }
        } else
        {
            key = url;
            value = "";
        }
        
        if (key == "game" && value != "")
        {
            value = unescape(value);
        }
        this->put(key.c_str(), value.c_str());
    }
    return true;
}

std::string TBJEnvContext::unescape(const std::string &str)
{
    std::string ret;
    char ch;
    int i, ii, len = str.length();
    
    for (i = 0; i < len; i++)
    {
        if (str[i] != '%') {
            if (str[i] == '+') ret += ' ';
            else ret += str[i];
        } else {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
    }
    return ret;
}
