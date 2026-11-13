find_program(UV_EXECUTABLE NAMES uv)

if(UV_EXECUTABLE)
    message(STATUS "uv found: ${UV_EXECUTABLE}")
    message(STATUS "Installing Python ${PYTHON_VERSION} using uv...")

    execute_process(
        COMMAND "${UV_EXECUTABLE}" python list ${PYTHON_VERSION}
        OUTPUT_VARIABLE UV_PYTHON_LIST
        OUTPUT_STRIP_TRAILING_WHITESPACE
        COMMAND_ERROR_IS_FATAL ANY
    )

    string(REGEX MATCH "^[^ \t\r\n]+" UV_PYTHON_TARGET "${UV_PYTHON_LIST}")

    if("${UV_PYTHON_TARGET}" STREQUAL "")
        message(FATAL_ERROR "uv python list returned no Python targets")
    endif()

    message(STATUS "Selected Python target: ${UV_PYTHON_TARGET}")

    set(Python_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/python")
    set(Python_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/python/${UV_PYTHON_TARGET}"
        CACHE PATH "Python installation directory" FORCE
    )
        
    execute_process(
        COMMAND "${UV_EXECUTABLE}" python install ${UV_PYTHON_TARGET} 
            -i "${Python_INSTALL_DIR}" -f
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND_ERROR_IS_FATAL ANY
    )
    message(STATUS "Python is installed on ${Python_INSTALL_DIR}")
else()
    message(FATAL_ERROR "uv not found!!!\n"
        "Please install uv from https://docs.astral.sh/uv/getting-started/installation/"
    )
endif()