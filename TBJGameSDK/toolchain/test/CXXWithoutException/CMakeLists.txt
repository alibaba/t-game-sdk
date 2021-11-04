cmake_policy(SET CMP0017 NEW)

cmake_minimum_required(VERSION 3.1)

set(APP_NAME SimpleTest)
project (${APP_NAME})

add_executable(${APP_NAME} 
    test.cpp
)