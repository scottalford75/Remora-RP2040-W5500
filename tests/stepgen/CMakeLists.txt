cmake_minimum_required(VERSION 3.12)
project(stepgen_tests)
set(CMAKE_CXX_STANDARD 17)

add_executable(test_stepgen
    test_stepgen.cpp
    ../../modules/module.cpp
)

target_include_directories(test_stepgen PRIVATE
    ../../port
)
