#!/usr/bin/python

# Copyright (C) 2021-2014 Alibaba Group Holding Limited

import os
import stat
import shutil
import argparse

script_root = os.path.abspath(os.path.dirname(__file__))
emscripten_root = None


def recursive_copy(src, dst, override, copy_call_back):
    if os.path.isdir(src):
        if not os.path.isdir(dst):
            os.makedirs(dst)
        files = os.listdir(src)
        for f in files:
            recursive_copy(os.path.join(src, f), os.path.join(dst, f), override, copy_call_back)
    else:
        if override or not os.path.exists(dst):
            shutil.copyfile(src, dst)
            if copy_call_back:
                copy_call_back(src, dst)


def copy_tao_tool():
    tao_tool_dir = os.path.join(script_root, "tool")

    def chmod_file(src_path, path):
        if path.endswith(".py") and os.path.exists("%sc" % path):
            os.remove("%sc" % path)

        if not os.path.basename(path).startswith(".") and not path.endswith(".py"):
            st = os.stat(path)
            os.chmod(path, st.st_mode | stat.S_IEXEC)

    recursive_copy(tao_tool_dir, emscripten_root, True, chmod_file)


def auto_search_xcode_project(xcode_dir):
    files = os.listdir(xcode_dir)
    for f in files:
        if f.endswith(".xcodeproj"):
            return os.path.join(xcode_dir, f)
    return None


def test_simple_cache(remove_dst):
    taomake = os.path.join(emscripten_root, "taomake.py")
    test_src = os.path.join(script_root, "test")
    test_dirs = os.listdir(test_src)
    for test_dir in test_dirs:
        test_dir_abs = os.path.abspath(os.path.join(test_src, test_dir))
        if not os.path.isdir(test_dir_abs):
            continue

        test_dst = os.path.join(test_dir_abs, "build")
        if os.path.exists(test_dst):
            shutil.rmtree(test_dst)

        if test_dir_abs.endswith("_xcode"):
            # test xcodeproject, xcode project only support python3
            xcode_project_path = auto_search_xcode_project(test_dir_abs)
            if xcode_project_path is None:
                continue
            test_command = "python3 %s %s --override --wat -o %s" % (taomake, xcode_project_path, test_dst)
            print(test_command)
            os.system(test_command)
        else:
            test_command = "python %s %s --wat -o %s" % (taomake, test_dir_abs, test_dst)
            print(test_command)
            os.system(test_command)

        if remove_dst and os.path.exists(test_dst):
            shutil.rmtree(test_dst)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='jenova_emscripten_installer.')
    parser.add_argument('--emscripten_dir', dest='emscripten_dir', action='store', help='emscripten directory')
    parser.add_argument('--test', dest='test', action='store_true', help='if compile test projects')
    parser.add_argument('--clear_after_test', dest='clear_after_test', action='store_true', help='clear test objects after test')
    args = parser.parse_args()

    if args.emscripten_dir:
        emscripten_root = os.path.abspath(args.emscripten_dir)
    else:
        emscripten_root = os.path.abspath(os.path.join(script_root, "..", "..", "..", "emsdk", "upstream", "emscripten"))
        test_emcc = os.path.join(emscripten_root, "emcc.py")
        if not os.path.exists(test_emcc):
            test_another_root = os.path.abspath(os.path.join(script_root, "..", "emsdk", "upstream", "emscripten"))
            test_emcc = os.path.join(test_another_root, "emcc.py")
            if os.path.exists(test_emcc):
                emscripten_root = test_another_root

    if not os.path.exists(os.path.join(emscripten_root, "emcmake")):
        print("emscripten not found. install emscripten first!")
        print("cd ..")
        print("./emsdk install latest")
        print("./emsdk activate latest")
        print("source ./emsdk_env.sh")
        exit(1)

    if os.path.exists(os.path.join(script_root, "thirdparty", "get_thirdparty_header.py")):
        os.system("python3 %s" % os.path.abspath(os.path.join(script_root, "thirdparty", "get_thirdparty_header.py")))

    current_dir = os.getcwd()
    recursive_copy(os.path.join(script_root, "thirdparty"), os.path.join(emscripten_root, "tao", "thirdparty"), True, None)
    if os.path.exists(os.path.join(script_root, "..", "include")):
        recursive_copy(os.path.join(script_root, "..", "include"),
                       os.path.join(emscripten_root, "tao", "include"), True, None)
    copy_tao_tool()

    if os.path.exists(os.path.join(script_root, "emscripten_modified")):
        recursive_copy(os.path.join(script_root, "emscripten_modified", "cache"), os.path.join(emscripten_root, "cache"), True, None)

    if args.test:
        test_simple_cache(args.clear_after_test)

    os.chdir(current_dir)
