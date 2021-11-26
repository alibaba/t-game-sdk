cmake_minimum_required(VERSION 3.13)
project(TBJGameFSTest)
# set(CMAKE_CXX_STANDARD 14)

include_directories(../WasmSimulator)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
include_directories(../../ThirdParty/pthread/include/windows)
link_directories(../../ThirdParty/pthread/lib/windows/x64)
endif()

set(SOURCE_FILES
	../TBJGameFSTest.cpp
	../TBJGameFS.h
	../TBJGameFS.cpp
)

add_executable(TBJGameFSTest ${SOURCE_FILES})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
target_link_libraries(TBJGameFSTest pthreadVC2.lib)
endif()