/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include "TBJGameEnviroment.h"

extern "C" int tbjNativeMain(int argc, char* argv[])
{
    printf("fasdfasdfasdf\n");
}

int main(int argc, char* argv[])
{
    return TBJGameNativeMain(tbjNativeMain, argc, argv);
}
