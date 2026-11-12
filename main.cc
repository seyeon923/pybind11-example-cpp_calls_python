#include <iostream>
#include <string>

#include <pybind11/embed.h>

namespace py = pybind11;
using namespace pybind11::literals;

// Embedded module `fast_cal`
PYBIND11_EMBEDDED_MODULE(fast_cal, m) {
    m.def("add", [](int i, int j) { return i + j; });
}

PYBIND11_EMBEDDED_MODULE(cpp_module, m) { m.attr("a") = 1; }

int main() {
    py::scoped_interpreter guard{};

    py::print("Hello from embedded Python!");

    // using exec to run Python code directly from C++
    py::exec(R"(
        kwargs = dict(name="World", number=42)
        message = "Hello, {name}! The answer is {number}".format(**kwargs)
        print(message)
    )");

    // using pybind11's API
    auto kwargs = py::dict("name"_a = "World2", "number"_a = 43);
    auto message = "Hello, {name}! The answer is {number}"_s.format(**kwargs);
    py::print(message);
    py::print(py::globals());

    // using pybind11's API + exec
    auto locals = py::dict("name"_a = "World3", "number"_a = 44);
    py::exec(R"(
        print("name:", name) # name: World3
        print("number:", number) # number: 44
        message = "Hello, {name}! The answer is {number}".format(**locals()) 
        print(message)
    )",
             py::globals(), locals);

    auto message2 = locals["message"].cast<std::string>();
    std::cout << "message2: " << message2 << std::endl;

    // importing modules
    py::module_ sys = py::module_::import("sys");
    py::print("Python version:", sys.attr("version"));
    py::print("Python executable:", sys.attr("executable"));
    py::print("Python path:", sys.attr("path"));

    sys.attr("path").cast<py::list>().append("python");
    py::print("Updated Python path:", sys.attr("path"));
    py::module_ algorithm = py::module_::import("algorithm");
    auto result = algorithm.attr("add")(2, 3).cast<int>();
    std::cout << "Result of python.algorithm.add(2, 3): " << result
              << std::endl;

    // adding embedded modules
    auto fast_cal = py::module_::import("fast_cal");
    auto fast_result = fast_cal.attr("add")(5, 6).cast<int>();
    std::cout << "Result of fast_cal.add(5, 6): " << fast_result << std::endl;

    auto py_module = py::module_::import("py_module");
    auto a = py_module.attr("a").cast<int>();
    auto b = py_module.attr("b").cast<int>();
    std::cout << "py_module.a: " << a << std::endl;
    std::cout << "py_module.b: " << b << std::endl;

    locals = py::dict("fmt"_a = "{} + {} = {}", **py_module.attr("__dict__"));
    py::exec(R"(
        c = a + b
        message = fmt.format(a, b, c)
        print(message)
    )",
             py::globals(), locals);

    auto message3 = locals["message"].cast<std::string>();
    std::cout << "message3: " << message3 << std::endl;

    return 0;
}