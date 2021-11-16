#!/usr/bin/python

# Copyright (C) 2021-2014 Alibaba Group Holding Limited

import os
import multiprocessing
import shutil

import taoflag
import argparse
import platform

script_root = os.path.abspath(os.path.dirname(__file__))


def check_build_dir_valid(src_dir, build_to_dir):
    if src_dir == build_to_dir:
        return False
    while len(src_dir) > 1 and src_dir != build_to_dir:
        src_dir = os.path.dirname(src_dir)
        if src_dir == build_to_dir:
            return False
    return True


def generate_wat_if_avaiable(cmake_file_dir, build_file_dir):
    def find_generated_path(build_dir):
        files = os.listdir(build_dir)
        for f in files:
            if f.endswith(".wasm"):
                return os.path.abspath(os.path.join(build_dir, f))

            elif os.path.isdir(f):
                sub_dir = os.path.join(build_dir, f)
                sub_path = find_generated_path(sub_dir)
                if sub_path is not None:
                    return sub_path

        return None

    wasm_path = find_generated_path(build_file_dir)
    if wasm_path is None:
        wasm_path = find_generated_path(cmake_file_dir)

    if wasm_path is None:
        return

    wat_architecture_dir = None
    if platform.system() == "Darwin" and platform.machine() == "arm64":
        wat_architecture_dir = "mac_arm64"
    elif platform.system() == "Darwin" and platform.machine() != "arm64":
        wat_architecture_dir = "mac_x64"
    elif platform.system() == "Windows":
        wat_architecture_dir = "windows"

    if wat_architecture_dir is None:
        return
    wasm2wat = os.path.abspath(os.path.join(script_root, "wasmtool", wat_architecture_dir, "wasm2wat"))
    if os.path.exists(wasm2wat):
        wat_path = wasm_path.replace(".wasm", ".wat")
        os.system("%s %s -o %s" % (wasm2wat, wasm_path, wat_path))


def get_precompiled_library_flag(emscripten_root, used_libraries):
    header_search_dir = [
        "include",
        "thirdparty/libjpg/include",
        "thirdparty/libpng/include",
        "thirdparty/libz/include"
    ]

    link_search_dir = []
    precomipled_pure_link_flag = ''

    for lib in used_libraries:
        header_search_dir.append("thirdparty/%s/include" % lib)
        if os.path.exists(os.path.join(emscripten_root, "tao", "thirdparty/%s/lib" % lib)):
            link_search_dir.append("thirdparty/%s/lib" % lib)
            precomipled_pure_link_flag = precomipled_pure_link_flag + " -l" + lib

    precomipled_flag = ""
    for header_dir in header_search_dir:
        dir_abs = os.path.abspath(os.path.join(emscripten_root, "tao", header_dir))
        precomipled_flag = precomipled_flag + " -I" + dir_abs

    for link_dir in link_search_dir:
        dir_abs = os.path.abspath(os.path.join(emscripten_root, "tao", link_dir))
        precomipled_flag = precomipled_flag + " -L" + dir_abs

    precomipled_flag = precomipled_flag + precomipled_pure_link_flag
    return precomipled_flag


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='taobao webassembly make tool')
    parser.add_argument('CMakeListsDir', action='store', help='dir contains CMakeLists.txt or the path of xcodeproject')
    parser.add_argument('-o', dest='outputDir', action='store', help='cmake output dir')
    parser.add_argument('--box2d', dest='used_box2d', action='store_true', help='add this flag if used box2d')
    parser.add_argument('--chipmunk', dest='used_chipmunk', action='store_true', help='add this flag if used chipmunk')
    parser.add_argument('--freetype', dest='used_freetype', action='store_true', help='add this flag is used freetype2')
    parser.add_argument('-g', dest='keep_symbols', action='store_true', help='wasm keep symbols')
    parser.add_argument('-d', dest='debug_mode', action='store_true', help='do not use ir opt (-Os)')
    parser.add_argument('--wat', dest="generate_wat", action='store_true', help='generate wat if found wasm2wat tool')
    parser.add_argument('--native_print', dest="use_native_print", action='store_true',
                        help='use native printf instead of wasm print. Not fully supported, Not support printf long long currently!');
    # parser.add_argument('rest', nargs=argparse.REMAINDER) # this is an alterative choice
    parser.add_argument('--no_use', dest="no_use", action="store", help='----separator: the option below is for xcodeproject -----------')
    ########### xcode project configs ##########
    parser.add_argument('--target', dest="xcode_target", action="store", help="export target. auto search default, if more than one target. need specify a target.")
    parser.add_argument('--config', dest="xcode_config", action="store", help="export xcode config(Debug/Release or self custom). Default Use xcodeproject default build configuration.")
    parser.add_argument('--override', dest="override_exist_cmakelists", action="store_true", help="override exist cmakelists.txt")
    parser.add_argument('--project_type', dest='project_type', action='store', help="cmake project type. Enable C CXX Default. Other Options Are CUDA, OBJC, OBJCXX, Fortran, HIP, ISPC, and ASM. Join there by -. ie(OBJC-OBJCXX)")
    parser.add_argument('--keep_objc', dest='keep_objc', action='store_true',
                        help=".m(m) does not pch, besides objc awalys use mac framework. if you want keep objc, make sure do not use pch, add custom framework path first (as emscripten will has it's own sysroot, so ${OS_SYSTEM_ROOT} is no long in xcode sdk dir)!")
    parser.add_argument('--generate_framework', dest='generate_framework', action='store_true', help='if generate find framework cmake code')
    parser.add_argument('--xcode_seperate', dest="export_seperate", action="store_true",
                        help="export seperate cmakelists.txt each xcode project")
    parser.add_argument('--clear', dest="clear_exported", action="store_true", help="clear exported result")

    args, custom_flag_arr = parser.parse_known_args()
    custom_flag = ' '.join(custom_flag_arr)

    # print(custom_flag)
    if args.CMakeListsDir is None:
        parser.print_help()
        exit(1)

    current_dir = os.getcwd()

    is_xcode_project = args.CMakeListsDir.endswith(".xcodeproj") or args.CMakeListsDir.endswith("project.pbxproj")
    if is_xcode_project:
        if int(platform.python_version_tuple()[0]) < 3:
            print("You need python3 or above to export xcode project. current version is %s" % platform.python_version())
            exit(1)
        try:
            import openstep_parser as osp
        except:
            os.system("pip3 install openstep_parser")

        try:
            import openstep_parser as osp
        except:
            exit(1)

        xcodeproject_to_cmakelists_tool = os.path.join(script_root, "mod-pbxproj", "taoxcode_exporter")
        xcodeproject_to_cmakelists_tool_dir = os.path.dirname(xcodeproject_to_cmakelists_tool)
        os.chdir(xcodeproject_to_cmakelists_tool_dir)

        xcode_project_location_dir = os.path.dirname(args.CMakeListsDir)
        cmake_output_dir = xcode_project_location_dir
        if args.outputDir:
            cmake_output_dir = args.outputDir

        # check if exist CMakeLists.txt
        if not args.override_exist_cmakelists:
            check_exist_path = os.path.join(cmake_output_dir, "CMakeLists.txt")
            if os.path.exists(check_exist_path):
                print("output path %s has CMakeLists.txt now!" % check_exist_path)
                print("--override to override it. or -o to change output dir")
                exit(1)

        merged_configs = ""
        if args.xcode_target:
            merged_configs = merged_configs + " --target " + args.xcode_target
        if args.xcode_config:
            merged_configs = merged_configs + " --config " + args.xcode_config
        if args.project_type:
            merged_configs = merged_configs + " --project_type " + args.project_type
        if args.keep_objc:
            merged_configs = merged_configs + " --keep_objc "
        if args.generate_framework:
            merged_configs = merged_configs + " --generate_framework"

        export_command = "%s %s -o %s %s" % (xcodeproject_to_cmakelists_tool, merged_configs, cmake_output_dir, args.CMakeListsDir)
        print("export xcode project: %s" % export_command)
        os.system(export_command)

        os.chdir(current_dir)

        args.CMakeListsDir = cmake_output_dir
        args.outputDir = os.path.join(args.CMakeListsDir, "build")

        if custom_flag is None:
            custom_flag = '-DJENOVA_ENV=ON'
        else:
            custom_flag = custom_flag + " -DJENOVA_ENV=ON"

    used_libraries = []
    if args.used_freetype:
        used_libraries.append("freetype")
    if args.used_box2d:
        used_libraries.append('box2d')
    if args.used_chipmunk:
        used_libraries.append('chipmunk')

    args.CMakeListsDir = os.path.abspath(args.CMakeListsDir)
    if args.outputDir is not None:
        args.outputDir = os.path.abspath(args.outputDir)

    script_root_base = os.path.basename(script_root)
    emscripten_root = None
    if script_root_base == "tool":
        emscripten_root = os.path.join(script_root, "..", "..", "emsdk", "upstream", "emscripten")
    else:
        emscripten_root = script_root

    cmake_file_dir = args.CMakeListsDir
    if os.path.isfile(cmake_file_dir):
        cmake_file_dir = os.path.abspath(os.path.dirname(cmake_file_dir))

    build_file_dir = os.path.abspath(os.path.join(cmake_file_dir, "build"))
    if args.outputDir:
        build_file_dir = args.outputDir
    if not check_build_dir_valid(cmake_file_dir, build_file_dir):
        print("${cmake output dir} can not contain or equal to ${CMakeFile_Dir}!")
        exit(1)

    if not os.path.exists(build_file_dir):
        os.makedirs(build_file_dir)

    compile_flag_pre = taoflag.get_emcc_compile_flag(args.debug_mode, args.keep_symbols)
    link_flag = taoflag.get_emcc_link_flag()

    printf_define = ""
    if args.use_native_print:
        printf_define = "-DJENOVA_USE_NATIVE_PRINT"
    compile_flag = "%s %s %s %s" % (
        compile_flag_pre, printf_define, custom_flag, get_precompiled_library_flag(emscripten_root, used_libraries))

    cpu_count = multiprocessing.cpu_count()

    emcmake_path = os.path.abspath(os.path.join(emscripten_root, "emcmake"))
    emmake_path = os.path.abspath(os.path.join(emscripten_root, "emmake"))

    os.chdir(build_file_dir)
    cmake_command = '%s cmake -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} %s" -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS} %s" ' \
                    '-DCMAKE_EXE_LINKER_FLAGS="%s" -DCMAKE_BUILD_TYPE=Release %s -S %s -B %s' % (emcmake_path,
                                                                                           compile_flag,
                                                                                           compile_flag, link_flag,
                                                                                           custom_flag,
                                                                                           cmake_file_dir,
                                                                                           build_file_dir)
    print(cmake_command)
    os.system(cmake_command)
    os.system("%s make -j%d" % (emmake_path, cpu_count))

    if args.generate_wat:
        generate_wat_if_avaiable(cmake_file_dir, build_file_dir)

    if is_xcode_project and args.clear_exported:
        gen_cmake_include_internal = os.path.join(args.CMakeListsDir, "gen_cmake_include_internal")
        if os.path.exists(gen_cmake_include_internal):
            shutil.rmtree(gen_cmake_include_internal)

    os.chdir(current_dir)
