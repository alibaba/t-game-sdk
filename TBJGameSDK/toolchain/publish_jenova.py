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


def copy_gles_header(emscripten_root):
    gl_header_dir = os.path.abspath(os.path.join(emscripten_root, "cache", "sysroot", "include"))
    duplicate_gl_dir = os.path.join(gl_header_dir, "OpenGLES")
    if not os.path.exists(duplicate_gl_dir):
        os.makedirs(duplicate_gl_dir)
    duplicate_es2_dir = os.path.join(duplicate_gl_dir, "ES2")
    duplicate_es3_dir = os.path.join(duplicate_gl_dir, "ES3")
    if not os.path.exists(duplicate_es2_dir):
        os.makedirs(duplicate_es2_dir)
    if not os.path.exists(duplicate_es3_dir):
        os.makedirs(duplicate_es3_dir)

    gles2_source_dir = os.path.join(gl_header_dir, "GLES2")
    gles3_source_dir = os.path.join(gl_header_dir, "GLES3")
    recursive_copy(gles2_source_dir, duplicate_es2_dir, True, None)
    recursive_copy(gles3_source_dir, duplicate_es3_dir, True, None)

    source_gl2 = os.path.join(duplicate_es2_dir, "gl2.h")
    source_gl2_ext = os.path.join(duplicate_es2_dir, "gl2ext.h")
    if os.path.exists(source_gl2):
        duplicate_gl = os.path.join(duplicate_es2_dir, "gl.h")
        shutil.copyfile(source_gl2, duplicate_gl)
    if os.path.exists(source_gl2_ext):
        duplicate_gl_ext = os.path.join(duplicate_es2_dir, "glext.h")
        shutil.copyfile(source_gl2_ext, duplicate_gl_ext)

    source_gl3 = os.path.join(duplicate_es3_dir, "gl32.h")
    source_gl3_ext = os.path.join(duplicate_es3_dir, "gl2ext.h")
    if os.path.exists(source_gl3):
        duplicate_gl = os.path.join(duplicate_es3_dir, "gl.h")
        shutil.copyfile(source_gl2, duplicate_gl)
    if os.path.exists(source_gl3_ext):
        duplicate_gl_ext = os.path.join(duplicate_es3_dir, "glext.h")
        shutil.copyfile(source_gl3_ext, duplicate_gl_ext)


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

    current_dir = os.getcwd()

    def chmod_file(src_path, path):
        if path.endswith(".py") and os.path.exists("%sc" % path):
            os.remove("%sc" % path)

        if not os.path.basename(path).startswith(".") and not path.endswith(".py"):
            st = os.stat(path)
            os.chmod(path, st.st_mode | stat.S_IEXEC)

    recursive_copy(os.path.join(script_root, "emscripten"), emscripten_root, True, chmod_file)

    if os.path.exists(os.path.join(script_root, "..", "include")):
        recursive_copy(os.path.join(script_root, "..", "include"),
                       os.path.join(emscripten_root, "tao", "include"), True, None)

    copy_gles_header(emscripten_root)

    if os.path.exists(os.path.join(script_root, "thirdparty", "get_thirdparty_header.py")):
        os.system("python3 %s" % os.path.abspath(os.path.join(script_root, "thirdparty", "get_thirdparty_header.py")))

    recursive_copy(os.path.join(script_root, "thirdparty"), os.path.join(emscripten_root, "tao", "thirdparty"), True,
                   None)

    if args.test:
        test_simple_cache(args.clear_after_test)

    os.chdir(current_dir)
