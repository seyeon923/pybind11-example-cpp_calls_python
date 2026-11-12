cmake_minimum_required(VERSION 3.19)

project(pybind11_example_cpp_calls_python)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/pybind11)

pybind11_add_module(example example.cc)

add_executable(main main.cc)