#include <iostream>

#include <opencv2/opencv.hpp>

#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#ifndef PROJECT_SOURCE_DIR
#    define PROJECT_SOURCE_DIR ""
#endif

namespace py = pybind11;
using namespace pybind11::literals;

int main() {
    double data[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    cv::Mat1d mat(2, 3, data);

    std::cout << "mat:\n" << mat << std::endl;

    py::scoped_interpreter guard{};

    try {
        py::module_ sys = py::module_::import("sys");
        sys.attr("path").attr("insert")(0, PROJECT_SOURCE_DIR);
        py::module_ algorithm = py::module_::import("src.python.algorithm");

        py::array_t<double> arr(
            {mat.rows, mat.cols}, mat.ptr<double>(),
            py::none());  // Pass `py::none` to handle for shallow copy
        data[0] = 10.0;
        std::cout << "mat after data[0] modification:\n" << mat << std::endl;
        py::print("arr after data[0] modification:\n", arr, "flush"_a = true);

        algorithm.attr("pass_array")(arr);

        std::cout << "mat after passing to Python:\n"
                  << mat << '\n'
                  << std::endl;
        py::print("arr after passing to Python:\n", arr, "flush"_a = true);
    } catch (const py::error_already_set& e) {
        std::cerr << "Python exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}