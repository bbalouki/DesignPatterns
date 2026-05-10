cmake_minimum_required(VERSION 3.25)
project(DesignPatterns LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings 
if(MSVC)
    add_compile_options(/W4 /WX /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Targets 
add_executable(creational_patterns  src/creational_patterns.cpp)
add_executable(structural_patterns  src/structural_patterns.cpp)
add_executable(behavioral_patterns  src/behavioral_patterns.cpp)
