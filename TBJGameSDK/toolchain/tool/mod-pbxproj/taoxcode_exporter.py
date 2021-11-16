#!/usr/bin/python

# Copyright (C) 2021-2014 Alibaba Group Holding Limited

import platform
import os
import argparse
import shutil
import subprocess
import hashlib

from pbxproj import XcodeProject

TARGET_APPLICATION = 0
TARGET_STATIC_LIBRARY = 1
TARGET_SHARED_LIBRARY = 2

__jenova_prefined_libraries = ["OpenGL", "OpenGLES", "freetype", "box2d", "libbox2d", "chipmunk", "libchipmunk",
                               "libpng",
                               "libjpeg", "libfreetype"]


def is_predefined_library(name):
    return name in __jenova_prefined_libraries


def stripped_name(name):
    name = name.strip()
    name = name.replace(" ", "_")
    return name.replace("\t", "_")


def src_attr_name(target):
    return "%s_SRCS" % stripped_name(target.name)


def auxiliary_src_attr_name(target):
    return "%s_auxiliary_SRCS" % stripped_name(target.name)


def get_file_md5(filepath):
    if not os.path.exists(filepath) or not os.path.isfile(filepath):
        return None
    md5_hash = hashlib.md5()
    with open(filepath, 'rb') as f:
        while True:
            b = f.read(8192)
            if not b:
                break
            md5_hash.update(b)
    return md5_hash.hexdigest()


def get_xcode_target_type(target):
    if target.productType == "com.apple.product-type.application":
        return TARGET_APPLICATION
    elif target.productType == "com.apple.product-type.library.static":
        return TARGET_STATIC_LIBRARY
    else:
        return TARGET_SHARED_LIBRARY


def remove_objc_flag(flags_expand):
    for i in range(len(flags_expand) - 1, -1, -1):
        if flags_expand[i].lower() == "-objc":
            del flags_expand[i]
    return flags_expand


class ExportError(Exception):
    def __init__(self, expression, message):
        self.expression = expression
        self.message = message


class XCodeProjectRootObject(object):
    def __init__(self, xcode_project_dir, root_object):
        self._root_object = root_object

        self._xcode_project_location_dir = os.path.abspath(os.path.dirname(xcode_project_dir))
        self._xcode_project_dir = os.path.abspath(
            os.path.join(self._xcode_project_location_dir, self._root_object["projectDirPath"]))
        self._xcode_project_root = os.path.abspath(
            os.path.join(self._xcode_project_location_dir, self._root_object["projectRoot"]))
        self._xcode_pods_root = os.path.abspath(os.path.join(self._xcode_project_location_dir, "Pods"))

    def get_xcode_project_location_dir(self):
        return self._xcode_project_location_dir

    def get_xcode_project_dir(self):
        return self._xcode_project_dir

    def get_xcode_project_root(self):
        return self._xcode_project_root

    def get_pods_root(self):
        return self._xcode_pods_root


class XCodeSettingResolver(object):
    def __init__(self, root_obj, target_configuration, project_configuration):
        self._root_obj = root_obj
        self._target_configuration = target_configuration
        self._project_configuration = project_configuration

        if self._target_configuration is not None:
            self._target_setting = self._target_configuration.buildSettings
        else:
            self._target_setting = None

        if self._project_configuration is not None:
            self._project_setting = self._project_configuration.buildSettings
        else:
            self._project_setting = None

    def get_target_setting(self):
        return self._target_setting

    def get_project_setting(self):
        return self._project_setting

    @classmethod
    def _strip_macro_str(cls, macro):
        macro = macro.strip()
        while (macro.startswith('"') and macro.endswith('"')) or (
                macro.startswith("'") and macro.endswith("'")) and len(macro) > 1:
            macro = macro[1:len(macro) - 1]

        while (macro.startswith('\\"') and macro.endswith('\\"')) and len(macro) > 3:
            macro = macro[2:len(macro) - 2]
        return macro

    @classmethod
    def _strip_macro_list(cls, macros):
        if macros is None:
            return []
        for i in range(0, len(macros)):
            macros[i] = XCodeSettingResolver._strip_macro_str(macros[i])
        return macros

    @classmethod
    def _strip_mac_and_change_to_list(cls, macro_str_or_list):
        macro_list = []
        if isinstance(macro_str_or_list, str):
            macro_str_or_list = macro_str_or_list.strip()
            macro_list = macro_str_or_list.split(" ")
        elif isinstance(macro_str_or_list, list):
            macro_list = macro_str_or_list

        macro_list = XCodeSettingResolver._strip_macro_list(macro_list)
        return macro_list

    def _resolve_in_project(self, label):
        if self._project_setting is None or self._project_setting[label] is None:
            return []

        resolved_list = []
        macro_list = XCodeSettingResolver._strip_mac_and_change_to_list(self._project_setting[label])

        for i in range(0, len(macro_list)):
            macro = macro_list[i]
            if macro.startswith('$(') or macro.startswith('${'):
                if macro.lower() == "$(inherited)" or macro.lower() == "${inherited}":
                    print("$(inherited) in project! this should not happen! ingore it!")
                elif macro.lower().startswith("$(srcroot)") or macro.lower().startswith("${srcroot}"):
                    # can not use join, as macro[len("$(srcroot)"):] may startswith /, and join will get an un
                    # expected path.
                    resolved_list.append(self._root_obj.get_xcode_project_root() + macro[len("$(srcroot)"):])
                elif macro.lower().startswith("$(project_dir)") or macro.lower().startswith("${project_dir}"):
                    resolved_list.append(self._root_obj.get_xcode_project_dir() + macro[len("$(project_dir)"):])
                elif macro.lower().startswith("$(pods_root)") or macro.lower().startswith("${pods_root}"):
                    resolved_list.append(self._root_obj.get_pods_root() + macro[len("${pods_root}"):])
                else:
                    print("ingore macro: %s" % macro)
            elif macro is not None and len(macro.strip()) > 0:
                resolved_list.append(macro)
        return resolved_list

    def resolve_macro(self, label):
        if self._target_setting[label] is None:
            return []

        resolved_list = []

        macro_list = XCodeSettingResolver._strip_mac_and_change_to_list(self._target_setting[label])
        for i in range(0, len(macro_list)):
            macro = macro_list[i]
            if macro.startswith('$(') or macro.startswith('${'):
                if macro.lower() == "$(inherited)" or macro.lower() == "${inherited}":
                    resolved_list_project = self._resolve_in_project(label)
                    resolved_list = [*resolved_list, *resolved_list_project]
                elif macro.lower().startswith("$(srcroot)") or macro.lower().startswith("${srcroot}"):
                    # can not use join, as macro[len("$(srcroot)"):] may startswith /, and join will get an un
                    # expected path.
                    resolved_list.append(self._root_obj.get_xcode_project_root() + macro[len("$(srcroot)"):])
                elif macro.lower().startswith("$(project_dir)") or macro.lower().startswith("${project_dir}"):
                    resolved_list.append(self._root_obj.get_xcode_project_dir() + macro[len("$(project_dir)"):])
                elif macro.lower().startswith("$(pods_root)") or macro.lower().startswith("${pods_root}"):
                    resolved_list.append(self._root_obj.get_pods_root() + macro[len("${pods_root}"):])
                else:
                    print("ingore macro: %s" % macro)
            elif macro is not None and len(macro.strip()) > 0:
                resolved_list.append(macro)
        return resolved_list

    def resolve_pure_setting(self, label):
        if self._target_setting[label] is not None:
            macro = self._strip_macro_str(self._target_setting[label])
            if macro.lower() == "$(inherited)" or macro.lower() == "${inherited}":
                return self._strip_macro_str(self._project_setting[label])
            else:
                return self._strip_macro_str(self._target_setting[label])
        elif self._project_setting[label] is not None:
            return self._strip_macro_str(self._project_setting[label])
        return None

    def resolve_pure_setting_bool(self, label):
        v = self.resolve_pure_setting(label)
        return v is not None and v.upper() == "YES"


class XcodePathResolver(object):
    def __init__(self, root_object, project):
        self._root_obj = root_object
        self._project = project
        self._hashid_to_parentgroup_mapping = {}
        groups = self._project.objects.get_objects_in_section('PBXGroup')
        for group in groups:
            for child in group.children:
                self._hashid_to_parentgroup_mapping[child] = group

    def resolve_path(self, path_info):
        if path_info.sourceTree == "<group>":
            group_path = ""
            ref_info = path_info

            is_ancestor_group_SOURCE_TREE = False
            is_ancestor_group_DEVELOPER_DIR = False
            while ref_info.get_id() in self._hashid_to_parentgroup_mapping:
                parent_group = self._hashid_to_parentgroup_mapping[ref_info.get_id()]

                if parent_group.sourceTree == "SOURCE_TREE":
                    is_ancestor_group_SOURCE_TREE = True
                    group_path = os.path.abspath(os.path.join(self._root_obj.get_xcode_project_dir(),
                                                              parent_group.path if parent_group.path is not None else "",
                                                              group_path))
                    break
                elif parent_group.sourceTree == "DEVELOPER_DIR":
                    is_ancestor_group_DEVELOPER_DIR = True
                    group_path = os.path.join(parent_group.path if parent_group.path is not None else "", group_path)
                    break
                group_relpath = parent_group.get_actual_relpath()
                if group_relpath is not None and len(group_relpath) > 0:
                    group_path = group_relpath + "/" + group_path
                ref_info = parent_group

            if is_ancestor_group_SOURCE_TREE:
                return os.path.abspath(os.path.join(group_path, path_info.path))
            elif is_ancestor_group_DEVELOPER_DIR:
                return os.path.join(group_path, path_info.path)
            else:
                return os.path.abspath(
                    os.path.join(self._root_obj.get_xcode_project_root(), group_path, path_info.path))

        elif path_info.sourceTree == "SOURCE_TREE":
            return os.path.abspath(os.path.join(self._root_obj.get_xcode_project_dir(), path_info.path))
        elif path_info.sourceTree == "DEVELOPER_DIR":
            return path_info.path
        else:
            return os.path.abspath(os.path.join(self._root_obj.get_xcode_project_location_dir(), path_info.path))


class TBJXCodeExporter(object):
    def __init__(self, xcode_project_path):
        # normalize path
        if xcode_project_path.endswith("/"):
            xcode_project_path = xcode_project_path[:len(xcode_project_path) - 1]
        base_name = os.path.basename(xcode_project_path)
        if base_name.endswith(".xcodeproj"):
            self._xcode_project_dir = os.path.abspath(xcode_project_path)
        elif base_name.endswith("project.pbxproj"):
            self._xcode_project_dir = os.path.abspath(os.path.dirname(xcode_project_path))
        else:
            raise ExportError(True, 'invalid project path')

        self._target_copy_include_dir = None
        self._xcode_project_file = os.path.join(self._xcode_project_dir, "project.pbxproj")

        self._project = XcodeProject.load(self._xcode_project_file)
        self._project_root = self._project.objects[self._project.rootObject]

        self._xcode_root_object = XCodeProjectRootObject(self._xcode_project_dir, self._project_root)
        self._path_resolver = XcodePathResolver(self._xcode_root_object, self._project)
        self._setting_resolver = None

        self._export_config = "Release"

        # xcode product is generated in ${self._build_configuration.buildSetting.SYMROOT}
        # tb wasm output dir is input by taomake.py
        self._project_type = None
        self._cmake_output_dir = None
        self._cmake_list_file = None
        self._export_target_name = None
        self._export_target = None

        # the clang emscripten used do not support pch in .m(m), so in pch case, move .m(m) to another library, and add dependency to that library
        self._keep_objc = None
        self._export_target_pch_additional_name = None
        self._os_sdk = None
        self._generate_framework_flag = None
        self.set_cmake_output_dir(self._xcode_root_object.get_xcode_project_location_dir())

        ######## cmake information caches ########
        self.__cmake_dependency = {}
        self.__ingore_build_phase_links = []

    def copy_global_configuration(self, other):
        self._project_type = other._project_type
        self.set_cmake_output_dir(other._cmake_output_dir)
        self.set_sdk(other._os_sdk)
        self.set_objc_auxiliary_setting(other._keep_objc)
        self._generate_framework_flag = other._generate_framework_flag

    def set_project_type(self, type):
        self._project_type = type

    def set_sdk(self, sdk):
        self._os_sdk = sdk
        if self._os_sdk is not None:
            self._os_sdk = self._os_sdk.strip()

    def set_objc_auxiliary_setting(self, s):
        self._keep_objc = s

    def set_generate_framework_flag(self, f):
        self._generate_framework_flag = f

    def get_export_target_name(self):
        return self._export_target_name

    def get_export_target(self):
        return self._export_target

    def get_auxiliary_export_target_name(self):
        return self._export_target_pch_additional_name

    def set_cmake_output_dir(self, d):
        self._cmake_output_dir = d
        if self._cmake_output_dir is not None and self._cmake_output_dir.endswith("/"):
            self._cmake_output_dir = self._cmake_output_dir[:len(self._cmake_output_dir - 1)]
        if self._cmake_output_dir is not None:
            self._cmake_list_file = os.path.join(self._cmake_output_dir, "CMakeLists.txt")

    def get_xcode_project_file(self):
        return self._xcode_project_file

    def get_xcode_project_location_dir(self):
        return self._xcode_project_file

    def get_xcode_project_dir(self):
        return self._xcode_project_dir

    def set_target_name(self, target_name):
        if self._export_target_name is None and target_name is None:
            targets = self._project.objects.get_targets(self._export_target_name)
            if targets and len(targets) > 0:
                target_name = targets[0].name

        self._export_target_name = target_name
        if self._export_target_name is not None:
            self._export_target = self._project.get_target_by_name(self._export_target_name)
        else:
            self._export_target = None

    def set_target_by_hashcode(self, hash_code):
        self._export_target = self._project.get_object(hash_code)
        self.set_target_name(self._export_target.name)

    def search_config(self, config):
        if self._export_target is None:
            print("export target has not setted yet! ingore build configuration set!")
            return

        build_configurations = self._project.get_object(self._export_target.buildConfigurationList)
        self._export_config = build_configurations.defaultConfigurationName
        if config is not None:
            for build_config_hascode in build_configurations.buildConfigurations:
                if config == self._project.get_object(build_config_hascode).name:
                    self._export_config = config
                    break

        target_build_configuration = None
        project_build_configuration = None

        # target build configuration. inhert from project buildconfiguration
        for build_config_hascode in build_configurations.buildConfigurations:
            if self._export_config == self._project.get_object(build_config_hascode).name:
                target_build_configuration = self._project.get_object(build_config_hascode)
                break

        # project build configuration
        project_build_configurations = self._project.get_object(self._project_root.buildConfigurationList)
        for build_config_hascode in project_build_configurations.buildConfigurations:
            if self._export_config == self._project.get_object(build_config_hascode).name:
                project_build_configuration = self._project.get_object(build_config_hascode)
                break

        self._setting_resolver = XCodeSettingResolver(self._xcode_root_object, target_build_configuration,
                                                      project_build_configuration)

        self._target_copy_include_dir = os.path.abspath(
            os.path.join(self._cmake_output_dir, "gen_cmake_include_internal", stripped_name(self._export_target_name)))
        ### rm include dir every time will make all files rebuild every time. so, keep it
        # if os.path.exists(self._target_copy_include_dir):
        #     if os.path.isdir(self._target_copy_include_dir):
        #         shutil.rmtree(self._target_copy_include_dir)
        #     else:
        #         os.remove(self._target_copy_include_dir)
        # os.makedirs(self._target_copy_include_dir)
        if os.path.exists(self._target_copy_include_dir) and not os.path.isdir(self._target_copy_include_dir):
            os.remove(self._target_copy_include_dir)
        if not os.path.exists(self._target_copy_include_dir):
            os.makedirs(self._target_copy_include_dir)
        return self._export_config

    def get_dependency_project_by_id(self, hashcode):
        dep = self._project.get_object(hashcode)
        dep_proxy_hashcode = dep["targetProxy"]
        dep_name = dep["name"]
        dep_proxy = self._project.get_object(dep_proxy_hashcode)
        dep_brief_info_key = dep_proxy["containerPortal"]
        remote_target = dep_proxy["remoteGlobalIDString"]
        dep_brief_info = self._project.get_object(dep_brief_info_key)
        dep_project_path = self._path_resolver.resolve_path(dep_brief_info)

        dep_exporter = TBJXCodeExporter(dep_project_path)
        dep_exporter.copy_global_configuration(self)
        dep_exporter.set_target_by_hashcode(remote_target)
        if dep_exporter.get_export_target() is not None:
            name = dep_exporter.get_export_target().name
            self.__ingore_build_phase_links.append(name)
        dep_config = dep_exporter.search_config(self._export_config)
        if dep_config is None:
            print("dependency project %s does not contain any config" % dep_name)
            exit(1)
        return dep_exporter

    def resolve_path_by_hashcode(self, hashcode):
        PBXBuildFile_obj = self._project.get_object(hashcode)
        PBXBuildFile_ref_hascode = PBXBuildFile_obj.fileRef
        brief_info = self._project.get_object(PBXBuildFile_ref_hascode)
        return self._path_resolver.resolve_path(brief_info), brief_info

    def _export_buildphase_source(self, f, target, buildPhase):
        src_name = src_attr_name(target)

        has_pch = self._target_has_pch()
        auxiliary_library_srcs = []

        f.write("set(%s\n" % src_name)
        if buildPhase.files:
            for file_hashcode in buildPhase.files:
                filepath, _ = self.resolve_path_by_hashcode(file_hashcode)

                if (filepath.lower().endswith(".mm") or filepath.lower().endswith(".m")) and not self._keep_objc:
                    continue

                if has_pch and (filepath.lower().endswith(".mm") or filepath.lower().endswith(".m")):
                    if self._keep_objc:
                        print(".m(m) does not support pch, move to auxiliary library: %s" % filepath)
                        auxiliary_library_srcs.append(filepath)
                    else:
                        print(".m(m) does not support pch, ingore it: %s" % filepath)
                    continue
                rel_filepath = os.path.relpath(filepath, self._cmake_output_dir)
                cmake_write_path = "${CMAKE_CURRENT_SOURCE_DIR}/%s" % rel_filepath
                f.write("\t%s\n" % cmake_write_path)
        f.write(")\n\n")

        if len(auxiliary_library_srcs) > 0:
            auxiliary_src_name = auxiliary_src_attr_name(target)
            self._export_target_pch_additional_name = stripped_name(target.name) + "_auxiliary"
            f.write("set(%s\n" % auxiliary_src_name)
            for filepath in auxiliary_library_srcs:
                rel_filepath = os.path.relpath(filepath, self._cmake_output_dir)
                cmake_write_path = "${CMAKE_CURRENT_SOURCE_DIR}/%s" % rel_filepath
                f.write("\t%s\n" % cmake_write_path)
            f.write(")\n\n")
            f.write("add_library(%s STATIC ${%s})\n" % (self._export_target_pch_additional_name, auxiliary_src_name))
            f.write("target_compile_options(%s PRIVATE -g)\n" % self._export_target_pch_additional_name)

        cmake_target_name = target.name
        target_type = get_xcode_target_type(target)
        if target_type == TARGET_APPLICATION:
            f.write("add_executable(%s ${%s})\n" % (stripped_name(cmake_target_name), src_name))
            f.write(
                'set_target_properties(%s PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")\n' % stripped_name(
                    cmake_target_name))
        elif target_type == TARGET_STATIC_LIBRARY:
            f.write("add_library(%s STATIC ${%s})\n" % (stripped_name(target.name), src_name))
            f.write("target_compile_options(%s PRIVATE -g)\n" % stripped_name(target.name))
        else:
            # jenova wasm does not support side_module yet! SHARED change to STATIC
            f.write("add_library(%s STATIC ${%s})\n" % (stripped_name(target.name), src_name))
            f.write("target_compile_options(%s PRIVATE -g)\n" % stripped_name(target.name))

    def _export_buildphase_header(self, f, target, buildPhase):
        has_copyed_header = buildPhase is not None and buildPhase.files is not None and len(buildPhase.files) > 0

        if has_copyed_header:
            for file_hashcode in buildPhase.files:
                filepath, _ = self.resolve_path_by_hashcode(file_hashcode)
                if os.path.exists(filepath):
                    to_file = os.path.join(self._target_copy_include_dir, os.path.basename(filepath))

                    if not os.path.exists(to_file) or get_file_md5(to_file) != get_file_md5(filepath):
                        shutil.copyfile(filepath, to_file)

    def _export_buildphase_resource(self, f, target, buildPhase):
        if buildPhase.files and len(buildPhase.files) > 0:
            for file_hashcode in buildPhase.files:
                filepath, brief_info = self.resolve_path_by_hashcode(file_hashcode)
                if os.path.exists(filepath):
                    rel_filepath = os.path.relpath(filepath, self._cmake_output_dir)

                    f.write("add_custom_command(TARGET %s POST_BUILD\n" % stripped_name(target.name))
                    if os.path.isdir(filepath):
                        f.write("\tCOMMAND ${CMAKE_COMMAND} -E copy_directory")
                    else:
                        f.write("\tCOMMAND ${CMAKE_COMMAND} -E copy\n")
                    f.write("\t${CMAKE_CURRENT_SOURCE_DIR}/%s\n" % rel_filepath)
                    f.write("\t${CMAKE_BINARY_DIR}/bin/%s\n" % brief_info.path)
                    f.write(")\n")

    def _export_buildphase_framework(self, f, target, buildPhase):
        if buildPhase.files and len(buildPhase.files) > 0:
            for file_hashcode in buildPhase.files:
                filepath, brief_info = self.resolve_path_by_hashcode(file_hashcode)

                system_library_full_name = os.path.basename(brief_info.get_name())
                filename, file_extension = os.path.splitext(system_library_full_name)

                is_predefined = is_predefined_library(filename)
                is_framework = file_extension is not None and (
                        file_extension.lower() == "framework" or file_extension.lower() == ".framework")

                if is_framework and not self._generate_framework_flag:
                    continue

                prefix_space = ""
                if is_predefined:
                    prefix_space = '\t'
                    f.write('if (NOT JENOVA_ENV)\n')

                if brief_info.sourceTree == "DEVELOPER_DIR" or is_framework:  ## search from system sdk. use find_library
                    if is_framework:
                        f.write('find_library(FRAMEWORK_%s\n' % filename)
                        f.write('\tNAMES %s\n' % filename)
                        f.write('\tPATHS ${CMAKE_OSX_SYSROOT}/System/Library\n')
                        f.write('\tPATH_SUFFIXES Frameworks\n')
                        f.write('\tNO_DEFAULT_PATH)\n')
                        f.write('if (${FRAMEWORK_%s} STREQUAL FRAMEWORK_%s-NOTFOUND)\n' % (filename, filename))
                        f.write('\tmessage(WARN ": Framework %s not found")\n' % filename)
                        f.write('else()\n')
                        f.write('\tmessage(STATUS "Framework %s found at ${FRAMEWORK_%s}")\n' % (filename, filename))
                        f.write('\tif (JENOVA_ENV)\n')
                        if self._export_target_pch_additional_name is not None:
                            f.write('\t\ttarget_include_directories(%s PUBLIC ${FRAMEWORK_%s})\n' % (
                                self._export_target_pch_additional_name, filename))
                        f.write('\t\ttarget_include_directories(%s PUBLIC ${FRAMEWORK_%s})\n' % (
                            stripped_name(target.name), filename))
                        f.write('\telse()\n')
                        if self._export_target_pch_additional_name is not None:
                            f.write('\t\ttarget_include_directories(%s PUBLIC ${FRAMEWORK_%s})\n' % (
                                self._export_target_pch_additional_name, filename))
                        f.write(
                            '\t\ttarget_link_libraries(%s ${FRAMEWORK_%s})\n' % (stripped_name(target.name), filename))
                        f.write('\tendif()\n')
                        f.write('endif()\n')
                    else:
                        f.write('find_library(LIBRARY_%s\n' % filename)
                        f.write('\tNAMES %s\n' % filename)
                        f.write('\tPATHS ${CMAKE_OSX_SYSROOT}/System/Library\n')
                        f.write('\tNO_DEFAULT_PATH)\n')
                        f.write('if (${LIBRARY_%s} STREQUAL LIBRARY_%s-NOTFOUND)\n' % (filename, filename))
                        f.write('\tmessage(WARN ": library %s not found")\n' % filename)
                        f.write('else()\n')
                        f.write('\tmessage(STATUS "library %s found at ${LIBRARY_%s}")\n' % (filename, filename))
                        f.write('\tif (NOT JENOVA_ENV)\n')
                        f.write(
                            '\t\ttarget_link_libraries(%s ${LIBRARY_%s})\n' % (stripped_name(target.name), filename))
                        f.write('\tendif()\n')
                        f.write('endif()\n')
                else:
                    if filename not in self.__ingore_build_phase_links:
                        f.write(
                            '%starget_link_libraries(%s %s)\n' % (prefix_space, stripped_name(target.name), filename))

                if is_predefined:
                    f.write('endif()\n\n')

    def _target_has_pch(self):
        use_prefix_header = self._setting_resolver.resolve_pure_setting_bool("GCC_PRECOMPILE_PREFIX_HEADER")
        prefix_header_expand = self._setting_resolver.resolve_macro(
            "GCC_PREFIX_HEADER") if use_prefix_header else None
        has_pch = prefix_header_expand is not None and len(prefix_header_expand) > 0
        return has_pch

    def _export_public_compile_preprocessor(self, f, target):
        c_version = self._setting_resolver.resolve_pure_setting("GCC_C_LANGUAGE_STANDARD")
        if c_version is not None:
            f.write('set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=%s")\n' % c_version)
        cxx_version = self._setting_resolver.resolve_pure_setting("CLANG_CXX_LANGUAGE_STANDARD")
        if cxx_version is not None:
            if cxx_version == "c++0x":
                cxx_version = "c++11"
            f.write('set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=%s")\n' % cxx_version)

        clang_cxx_library = self._setting_resolver.resolve_pure_setting("CLANG_CXX_LIBRARY")
        if len(clang_cxx_library) > 0:
            f.write('set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=%s")\n' % clang_cxx_library)

        other_c_flag_expand = self._setting_resolver.resolve_macro("OTHER_CFLAGS")
        if len(other_c_flag_expand) > 0:
            other_c_flag = " ".join(other_c_flag_expand)
            f.write('set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} %s")\n' % other_c_flag)

        other_cxx_flag_expand = self._setting_resolver.resolve_macro("OTHER_CPLUSPLUSFLAGS")
        if len(other_cxx_flag_expand) > 0:
            other_cxx_flag = " ".join(other_cxx_flag_expand)
            f.write('set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} %s")\n' % other_cxx_flag)

        other_ld_flag_expand = self._setting_resolver.resolve_macro("OTHER_LDFLAGS")
        other_ld_flag_expand = remove_objc_flag(other_ld_flag_expand)
        if len(other_ld_flag_expand) > 0:
            other_ld_flag = " ".join(other_ld_flag_expand)
            f.write('set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} %s")\n' % other_ld_flag)

    def _export_private_compile_preprocessor(self, f, target):
        preprocessor_expand = self._setting_resolver.resolve_macro("GCC_PREPROCESSOR_DEFINITIONS")
        if len(preprocessor_expand) > 0:
            preprocessor_defines = " ".join(preprocessor_expand)
            if self._export_target_pch_additional_name is not None:
                f.write('target_compile_definitions(%s PUBLIC %s)\n' % (
                self._export_target_pch_additional_name, preprocessor_defines))

            f.write('target_compile_definitions(%s PUBLIC %s)\n' % (stripped_name(target.name), preprocessor_defines))

        use_prefix_header = self._setting_resolver.resolve_pure_setting_bool("GCC_PRECOMPILE_PREFIX_HEADER")
        prefix_header_expand = self._setting_resolver.resolve_macro(
            "GCC_PREFIX_HEADER") if use_prefix_header else None

        has_pch = prefix_header_expand is not None and len(prefix_header_expand) > 0
        if has_pch:
            f.write('target_precompile_headers(%s\n' % stripped_name(target.name))

            resolved_prefix_header = ''.join(prefix_header_expand)
            pch_location = None
            if os.path.isabs(resolved_prefix_header):
                pch_location = resolved_prefix_header
            else:
                pch_location = os.path.abspath(
                    os.path.join(self._xcode_root_object.get_xcode_project_root(), resolved_prefix_header))
            rel_filepath = os.path.relpath(pch_location, self._cmake_output_dir)
            cmake_write_path = "${CMAKE_CURRENT_SOURCE_DIR}/%s" % rel_filepath
            f.write('\tPUBLIC %s\n' % cmake_write_path)
            f.write(')\n')

        header_search_expand = self._setting_resolver.resolve_macro("HEADER_SEARCH_PATHS")
        framework_search_expand = self._setting_resolver.resolve_macro("FRAMEWORK_SEARCH_PATHS")
        merged_search_expand = [*header_search_expand, *framework_search_expand]
        if self._setting_resolver.resolve_pure_setting_bool("ALWAYS_SEARCH_USER_PATHS"):
            user_header_search_expand = self._setting_resolver.resolve_macro("USER_HEADER_SEARCH_PATHS")
            merged_search_expand = [*merged_search_expand, *user_header_search_expand]

        merged_search_expand.insert(0, self._target_copy_include_dir)
        if len(merged_search_expand) > 0:
            if self._export_target_pch_additional_name is not None:
                f.write('target_include_directories(%s PUBLIC\n' % self._export_target_pch_additional_name)
                for solved_path in merged_search_expand:
                    if not os.path.isabs(solved_path):
                        solved_path = os.path.join(self._xcode_root_object.get_xcode_project_root(), solved_path)
                    rel_filepath = os.path.relpath(solved_path, self._cmake_output_dir)
                    cmake_write_path = "${CMAKE_CURRENT_SOURCE_DIR}/%s" % rel_filepath
                    f.write('\t%s\n' % cmake_write_path)
                f.write(')\n\n')

            f.write('target_include_directories(%s PUBLIC\n' % stripped_name(target.name))
            for solved_path in merged_search_expand:
                if not os.path.isabs(solved_path):
                    solved_path = os.path.join(self._xcode_root_object.get_xcode_project_root(), solved_path)
                rel_filepath = os.path.relpath(solved_path, self._cmake_output_dir)
                cmake_write_path = "${CMAKE_CURRENT_SOURCE_DIR}/%s" % rel_filepath
                f.write('\t%s\n' % cmake_write_path)
            f.write(')\n\n')

        libraries_search_expand = self._setting_resolver.resolve_macro("LIBRARY_SEARCH_PATHS")
        if len(libraries_search_expand) > 0:
            f.write('target_link_directories(%s PUBLIC\n' % stripped_name(target.name))
            for solved_path in libraries_search_expand:
                if not os.path.isabs(solved_path):
                    solved_path = os.path.join(self._xcode_root_object.get_xcode_project_root(), solved_path)
                rel_filepath = os.path.relpath(solved_path, self._cmake_output_dir)
                cmake_write_path = "${CMAKE_CURRENT_SOURCE_DIR}/%s" % rel_filepath
                f.write('\t%s\n' % cmake_write_path)
            f.write(')\n\n')

    def _export_one_target(self, f, target):
        deps = target.dependencies
        if deps is not None and len(deps) > 0:
            for dep_hashcode in deps:
                dep = self.get_dependency_project_by_id(dep_hashcode)
                if dep is None:
                    continue

                if target.name not in self.__cmake_dependency:
                    self.__cmake_dependency[target.name] = []
                self.__cmake_dependency[target.name].append(dep)
                dep._export_one_target(f, dep._export_target)
                f.write("\n")

        buildPhases = target.buildPhases
        if buildPhases is None or len(buildPhases) == 0:
            return

        self._export_public_compile_preprocessor(f, target)

        ## source -> framework -> resource
        for buildPhase_hashcode in buildPhases:
            buildPhase = self._project.get_object(buildPhase_hashcode)
            if buildPhase.isa == "PBXSourcesBuildPhase":
                self._export_buildphase_source(f, target, buildPhase)

        for buildPhase_hashcode in buildPhases:
            buildPhase = self._project.get_object(buildPhase_hashcode)
            if buildPhase.isa == "PBXHeadersBuildPhase":
                self._export_buildphase_header(f, target, buildPhase)
                break

        self._export_private_compile_preprocessor(f, target)

        for buildPhase_hashcode in buildPhases:
            buildPhase = self._project.get_object(buildPhase_hashcode)
            if buildPhase.isa == "PBXFrameworksBuildPhase":
                self._export_buildphase_framework(f, target, buildPhase)

        for buildPhase_hashcode in buildPhases:
            buildPhase = self._project.get_object(buildPhase_hashcode)
            if buildPhase.isa == "PBXResourcesBuildPhase":
                self._export_buildphase_resource(f, target, buildPhase)

    def export_cmake_dependencies(self, f):
        if self._export_target_pch_additional_name is not None:
            f.write('\nadd_dependencies(%s %s)\n' % (
            stripped_name(self._export_target_name), self._export_target_pch_additional_name))
            f.write('target_link_libraries(%s %s)\n' % (
            stripped_name(self._export_target_name), self._export_target_pch_additional_name))

        for target_name, deps in self.__cmake_dependency.items():
            f.write('\nadd_dependencies(%s' % stripped_name(target_name))
            for dep in deps:
                f.write(' %s' % stripped_name(dep.get_export_target_name()))
                if dep.get_auxiliary_export_target_name() is not None:
                    f.write(' %s' % stripped_name(dep.get_auxiliary_export_target_name()))
            f.write(")\n")

        for target_name, deps in self.__cmake_dependency.items():
            f.write('target_link_libraries(%s' % stripped_name(target_name))
            for dep in deps:
                f.write(' %s' % stripped_name(dep.get_export_target_name()))
                if dep.get_auxiliary_export_target_name() is not None:
                    f.write(' %s' % stripped_name(dep.get_auxiliary_export_target_name()))
            f.write(")\n")

    def export(self):
        f = open(self._cmake_list_file, "w")
        TBJXCodeExporter.export_cmake_header(f)
        self.export_cmake_project_name_and_defaultoption(f, self._export_target_name)
        self._export_one_target(f, self._export_target)
        self.export_cmake_dependencies(f)
        f.flush()
        f.close()

    @staticmethod
    def export_cmake_header(f):
        f.write('# NOTICE: This file is auto-generated by xcode project. Any modifications here may be covered!\n\n')
        f.write('cmake_policy(SET CMP0017 NEW)\n')
        f.write('cmake_minimum_required (VERSION 3.4)\n\n')

    def export_cmake_project_name_and_defaultoption(self, f, project_name):
        if self._project_type is not None:
            p_types = self._project_type.split("-")
            p_types.insert(0, "CXX")
            p_types.insert(0, "C")
            p_types_str = " ".join(p_types)
            f.write('project(%s %s)\n\n' % (stripped_name(project_name), p_types_str))
        else:
            f.write('project(%s)\n\n' % stripped_name(project_name))
        f.write('option(JENOVA_ENV OFF)\n')

    def has_unique_target(self, target_name):
        targets = self._project.objects.get_targets(target_name)
        if targets is not None and len(targets) == 1:
            return 0
        if targets is None or len(targets) == 0:
            return 1
        else:
            return 2


if __name__ == '__main__':
    if int(platform.python_version_tuple()[0]) < 3:
        print("only support python3 or above. current version is %s" % platform.python_version())
        exit(1)

    parser = argparse.ArgumentParser(
        description='taobao xcode project exporter based on thirdparty tool ('
                    'https://github.com/kronenthaler/mod-pbxproj).')
    parser.add_argument('pbxproj', action='store', help='xcode project path')
    parser.add_argument('-o', dest='outputDir', action='store', help='cmake output dir')
    parser.add_argument('--target', dest="exportTarget", action='store',
                        help='export target. auto search default, if more than one target. need specify a target.')
    parser.add_argument('--config', dest="config", action='store', help='export configuration, if no specify, use '
                                                                        'xcode default buildconfiguration')
    parser.add_argument('--project_type', dest='project_type', action='store',
                        help="cmake project type. Enable C CXX Default. Other Options Are CUDA, OBJC, OBJCXX, Fortran, HIP, ISPC, and ASM. Join there by -. ie(OBJC-OBJCXX)")
    parser.add_argument('--generate_framework', dest='generate_framework', action='store_true',
                        help='if generate find framework cmake code')
    parser.add_argument('--sdk', dest="sdk", action='store', help="macos sdk. only for header search and debug!")
    parser.add_argument('--keep_objc', dest='keep_objc', action='store_true',
                        help=".m(m) does not pch, besides objc awalys use mac framework. if you want keep objc, make sure do not use pch, add custom framework path first (as emscripten will has it's own sysroot, so ${OS_SYSTEM_ROOT} is no long in xcode sdk dir)!")
    args, custom_flag_arr = parser.parse_known_args()

    if args.pbxproj is None:
        parser.print_help()
        exit(1)

    exporter = TBJXCodeExporter(args.pbxproj)
    if args.outputDir:
        exporter.set_cmake_output_dir(args.outputDir)
    if not args.sdk and platform.system() == "Darwin":
        args.sdk = \
            subprocess.Popen(["xcrun", "--sdk", "macosx", "--show-sdk-path"], stdout=subprocess.PIPE).communicate()[
                0].decode()
    exporter.set_generate_framework_flag(args.generate_framework)
    exporter.set_project_type(args.project_type)
    exporter.set_sdk(args.sdk)
    exporter.set_objc_auxiliary_setting(args.keep_objc)

    check_ret = exporter.has_unique_target(args.exportTarget)
    if check_ret != 0:
        if check_ret == 1:
            if args.exportTarget is not None:
                print("the project does not contain target %s", args.exportTarget)
            else:
                print("the project does not contain any target")
        elif check_ret == 2:
            print("Find multi targets in the project, we don't know what target you want to export! use --target to "
                  "specify one!")
        else:
            print("unknown exception!")
        exit(1)

    exporter.set_target_name(args.exportTarget)
    search_result = exporter.search_config(args.config)
    if search_result is None:
        if args.config is None:
            print("target does not contain any configuration")
        else:
            print("target does not contain config: %s" % args.config)
        exit(1)
    print("export config: %s" % search_result)

    exporter.export()
