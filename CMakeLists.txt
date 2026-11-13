cmake_minimum_required(VERSION 3.19)

project(pybind11_example_cpp_calls_python)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYTHON_VERSION 3.12 CACHE STRING
    "Python version to install"
)
include(install_python.cmake)

find_package(Python ${PYTHON_VERSION} REQUIRED EXACT COMPONENTS Interpreter Development)
add_subdirectory(extern/pybind11)

pybind11_add_module(example example_module.cc)

add_executable(interpreter_embedding_examples interpreter_embedding_examples.cc)
target_link_libraries(interpreter_embedding_examples PRIVATE pybind11::embed)
target_compile_definitions(interpreter_embedding_examples
    PRIVATE PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)