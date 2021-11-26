#!/usr/bin/python

# Copyright (C) 2021-2014 Alibaba Group Holding Limited

import os
import platform
import shutil
import sys

script_root = os.path.abspath(os.path.dirname(__file__))


def is_mac_arm64():
    return platform.system() == "Darwin" and platform.machine() == "arm64"


def is_mac_x64():
    return platform.system() == "Darwin" and platform.machine() != "arm64"


def is_windows():
    return platform.system() == "Windows"


def copy_emscripten_dep_thirdparty(emsdk_dir):
    python_dir = os.path.join(emsdk_dir, "python")

    mac_arm64_dir = os.path.join(script_root, "emscripten_dep_thirdparty", "mac_arm64")
    mac_x64_dir = os.path.join(script_root, "emscripten_dep_thirdparty", "mac_x64")
    windows_dir = os.path.join(script_root, "emscripten_dep_thirdparty", "windows")

    if not os.path.exists(python_dir):
        if is_mac_arm64():
            shutil.copytree(os.path.join(mac_arm64_dir, "python"), python_dir)
        elif is_mac_x64():
            shutil.copytree(os.path.join(mac_x64_dir, "python"), python_dir)
        elif is_windows():
            shutil.copytree(os.path.join(windows_dir, "python"), python_dir)


if __name__ == '__main__':
    parent_dir = os.path.abspath(os.path.join(script_root, ".."))
    if os.path.basename(script_root) == 'toolchain':
        parent_dir = os.path.abspath(os.path.join(script_root, "..", "..", ".."))
    emsdk_dir = os.path.join(parent_dir, "emsdk")

    if not os.path.exists(emsdk_dir):
        os.chdir(parent_dir)
        if platform.system() == "Windows":
            os.system("git clone https://github.com/emscripten-core/emsdk.git")
        else:
            os.system("git clone https://github.com/emscripten-core/emsdk.git")

    emscripten_version = "2.0.32" # "latest"
    os.chdir(emsdk_dir)
    if platform.system() == "Windows":
        os.system("git pull")
        # copy_emscripten_dep_thirdparty(emsdk_dir)
        os.system("./emsdk install %s" % emscripten_version)
        os.system("./emsdk activate %s" % emscripten_version)
        os.system("./emsdk_env.bat")
    else:
        os.system("git pull")
        # copy_emscripten_dep_thirdparty(emsdk_dir)
        os.system("./emsdk install %s" % emscripten_version)
        os.system("./emsdk activate %s" % emscripten_version)
        os.system("source ./emsdk_env.sh")

    if len(sys.argv) > 1:
        os.chdir(script_root)
        os.system("python3 install_taobao_emscripten.py")
