#include <iostream>
#include <string>

#include <pybind11/embed.h>

#ifndef PROJECT_SOURCE_DIR
#    define PROJECT_SOURCE_DIR ""
#endif

namespace py = pybind11;

int main() {
    py::scoped_interpreter guard{};

    try {
        py::module_::import("sys").attr("path").attr("insert")(
            1, PROJECT_SOURCE_DIR);
        py::module_ algorithm = py::module_::import("src.python.algorithm");
        auto result = algorithm.attr("check_3rd_parties")().cast<bool>();

        std::cout << "Result: " << std::boolalpha << result << std::endl;
    } catch (const py::error_already_set& e) {
        std::cerr << "Python error: " << e.what() << std::endl;
    }

    return 0;
}