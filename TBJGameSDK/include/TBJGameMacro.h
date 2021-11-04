/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#ifndef _TAOBAO_JENOVA_MACRO_H_
#define _TAOBAO_JENOVA_MACRO_H_

#include <stddef.h>

#ifdef __cplusplus
#define TAOBAO_NS_BEGIN extern "C" {
#define TAOBAO_NS_END }
#else
#define TAOBAO_NS_BEGIN
#define TAOBAO_NS_END
#endif

#endif
