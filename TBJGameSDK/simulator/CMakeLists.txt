cmake_minimum_required(VERSION 3.11)

include(${CMAKE_CURRENT_SOURCE_DIR}/ios.toolchain.cmake)
set(MACOSX_DEPLOYMENT_TARGET ios)

project(TBJGameFramework)
set(ProjectName TBJGameFramework)
set(CMAKE_CXX_STANDARD 11)

option(TEST_EXE OFF)

macro(source_group_by_dir source_files)
	set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR}/..)

    foreach(sgbd_file ${${source_files}})
		file(RELATIVE_PATH sgbd_fpath ${sgbd_cur_dir} ${sgbd_file})
        string(REGEX REPLACE "(.*)[/\\].*" \\1 sgbd_group_name ${sgbd_fpath})
        string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
        string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
        if(sgbd_nogroup)
            set(sgbd_group_name "\\")
        endif(sgbd_nogroup)
        source_group(${sgbd_group_name} FILES ${sgbd_file})
    endforeach(sgbd_file)
endmacro(source_group_by_dir)

macro(add_framework appname fwname)
    find_library(FRAMEWORK_${fwname}
        NAMES ${fwname}
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library
        PATH_SUFFIXES Frameworks
        NO_DEFAULT_PATH)
    if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        TARGET_LINK_LIBRARIES(${appname} ${FRAMEWORK_${fwname}})
        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro(add_framework)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../TBJGameSDK/TBJGameEnviroment.h")
    set(SIMULATOR_ENV "Dev")
elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../include/TBJGameEnviroment.h")
    set(SIMULATOR_ENV "Pub")
else()
    set(SIMULATOR_ENV "Unknown")
endif()

if (${SIMULATOR_ENV} STREQUAL "Unknown")
    MESSAGE(FATAL_ERROR "Unknown cmake build target")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (${SIMULATOR_ENV} STREQUAL "Dev")
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../TBJGameSDK)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../taoemsdk/wasmruntime_simulator/thirdparty/libpng/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../taoemsdk/wasmruntime_simulator/thirdparty/libjpg/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../taoemsdk/wasmruntime_simulator/thirdparty/libz/include)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../taoemsdk/wasmruntime_simulator/thirdparty/libpng/lib)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../taoemsdk/wasmruntime_simulator/thirdparty/libjpg/lib)
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libpng/include/png.h")
            execute_process(
                COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/get_simulator_thirdparty.py
            )
            if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libpng/include/png.h")
                message(WARN "Auto load thirdparty failed! Run run the following command first!")
                message(WARN "python3 ${CMAKE_CURRENT_SOURCE_DIR}/get_simulator_thirdparty.py")
            endif()
    endif()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libpng/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libjpg/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libz/include)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libpng/lib)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libjpg/lib)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS NO)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_definitions(-DARCH_ARM64)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mcpu=native")

file(GLOB_RECURSE SIMULATOR_OBJC "${CMAKE_CURRENT_SOURCE_DIR}/*.mm" "${CMAKE_CURRENT_SOURCE_DIR}/.m" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
if (${SIMULATOR_ENV} STREQUAL "Dev")
    file(GLOB_RECURSE TBJ_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/../TBJGameSDK/*.h")
else()
    file(GLOB_RECURSE TBJ_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/../include/*.h")
endif()

set(TBJGameFramework_src 
    ${SIMULATOR_OBJC} 
    ${TBJ_HEADER}
    ${CMAKE_CURRENT_SOURCE_DIR}/TBJEnvContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TBJStorageContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TBJGameInstance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TBJGameImage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TBJGameImageInternal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TBJGameFS.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TBJGameFS.cpp
        )

if (TEST_EXE)
    list(APPEND TBJGameFramework_src
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    )
endif()

source_group_by_dir(TBJGameFramework_src)

if (TEST_EXE)
    add_executable(${ProjectName} ${TBJGameFramework_src})
    set_target_properties(${ProjectName} PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.alibaba.jenova
        VERSION 16.4.0
        SOVERSION 1.0.0
        PUBLIC_HEADER "${TBJGameFramework_header}"
        # XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    )
else()
    add_library(${ProjectName} SHARED ${TBJGameFramework_src})
    set_target_properties(${ProjectName} PROPERTIES
        FRAMEWORK TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.alibaba.jenova
        VERSION 16.4.0
        SOVERSION 1.0.0
        PUBLIC_HEADER "${TBJ_HEADER}"
        # XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
    )
endif()

target_link_libraries(${ProjectName} z sqlite3 png jpeg)
add_framework(TBJGameFramework UIKit)
add_framework(TBJGameFramework QuartzCore)
add_framework(TBJGameFramework OpenGLES)
