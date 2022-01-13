#!/usr/bin/python

# Copyright (C) 2021-2014 Alibaba Group Holding Limited

def get_emcc_compile_flag(debug, keep_symbol):
    # return "-Os -fno-builtin -fno-inline -Wno-unused-command-line-argument"
    # tried test more on -O3 and -Os
    return "%s %s -DJENOVA_SDK -fno-builtin -fno-builtin-function -Wno-unused-command-line-argument " % ("-O0" if debug else "-O3", "-g" if keep_symbol else "")


def get_emcc_link_flag():
    return "-s WASM=1 -s SUPPORT_LONGJMP=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s STANDALONE_WASM --strip-all " \
           "-Wno-error=address -Wno-pointer-to-int-cast -fno-builtin-function"


def get_emcc_allow_undefined_symbols():
    return ""


if __name__ == '__main__':
    print("If you compile your program use emcc command, add flag follows:")
    print("compile flag: %s" % get_emcc_compile_flag(False, False))
    print("link flag: %s" % get_emcc_link_flag())
