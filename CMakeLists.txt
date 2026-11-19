cmake_minimum_required(VERSION 3.19)

project(pybind11_example_cpp_calls_python)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(msvc_c "$<COMPILE_LANG_AND_ID:C,MSVC>")

# compiler warning options
add_compile_options(
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:/W4>>")

# MSVC /permissive- compile option
add_compile_options(
    "$<${msvc_cxx}:$<BUILD_INTERFACE:/permissive->>")

# MSVC utf-8 option
add_compile_options(
    "$<${msvc_cxx}:$<BUILD_INTERFACE:/utf-8>>"
    "$<${msvc_c}:$<BUILD_INTERFACE:/utf-8>>"
)

set(PYTHON_VERSION 3.12 CACHE STRING
    "Python version to install"
)
include(install_python.cmake)

find_package(OpenCV REQUIRED)
find_package(Python ${PYTHON_VERSION} REQUIRED EXACT COMPONENTS Interpreter Development)
add_subdirectory(extern/pybind11)

pybind11_add_module(example src/cpp/example_module.cc)

add_executable(interpreter_embedding_examples 
    src/cpp/interpreter_embedding_examples.cc
)
target_link_libraries(interpreter_embedding_examples PRIVATE pybind11::embed)
target_compile_definitions(interpreter_embedding_examples
    PRIVATE PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(type_conversion_examples src/cpp/type_conversion_examples.cc)
target_include_directories(type_conversion_examples PRIVATE extern/eigen)
target_link_libraries(type_conversion_examples PRIVATE pybind11::embed)

add_executable(check_array_pass src/cpp/check_array_pass.cc)
target_compile_definitions(check_array_pass
    PRIVATE PROJECT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(check_array_pass 
    PRIVATE ${OpenCV_LIBS}
    PRIVATE pybind11::embed
)