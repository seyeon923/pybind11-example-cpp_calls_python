// examples from:
// https://pybind11.readthedocs.io/en/latest/advanced/embedding.html
#include <iostream>
#include <string>
#include <future>

#include <pybind11/embed.h>
#include <pybind11/subinterpreter.h>

namespace py = pybind11;
using namespace pybind11::literals;

// Embedded module `fast_cal`
PYBIND11_EMBEDDED_MODULE(fast_cal, m,
                         py::multiple_interpreters::per_interpreter_gil()) {
    m.def("add", [](int i, int j) { return i + j; });
}

PYBIND11_EMBEDDED_MODULE(cpp_module, m,
                         py::multiple_interpreters::per_interpreter_gil()) {
    m.attr("a") = 1;
}

PYBIND11_EMBEDDED_MODULE(printer, m,
                         py::multiple_interpreters::per_interpreter_gil()) {
    m.def("which", [](const std::string& when) {
        std::cout << when << "; Current Interpreter is "
                  << py::subinterpreter::current().id() << std::endl;
    });
}

int main() {
    {
        py::scoped_interpreter guard{};

        py::print("Hello from embedded Python!", "flush"_a = true);

        // using exec to run Python code directly from C++
        py::exec(R"(
            kwargs = dict(name="World", number=42)
            message = "Hello, {name}! The answer is {number}".format(**kwargs)
            print(message, flush=True)
        )");

        // using pybind11's API
        auto kwargs = py::dict("name"_a = "World2", "number"_a = 43);
        auto message =
            "Hello, {name}! The answer is {number}"_s.format(**kwargs);
        py::print(message, "flush"_a = true);
        py::print(py::globals(), "flush"_a = true);

        // using pybind11's API + exec
        auto locals = py::dict("name"_a = "World3", "number"_a = 44);
        py::exec(R"(
            print("name:", name, flush=True) # name: World3
            print("number:", number, flush=True) # number: 44
            message = "Hello, {name}! The answer is {number}".format(**locals()) 
            print(message, flush=True)
        )",
                 py::globals(), locals);

        auto message2 = locals["message"].cast<std::string>();
        std::cout << "message2: " << message2 << std::endl;

        // importing modules
        py::module_ sys = py::module_::import("sys");
        py::print("Python version:", sys.attr("version"), "flush"_a = true);
        py::print("Python executable:", sys.attr("executable"),
                  "flush"_a = true);
        py::print("Python path:", sys.attr("path"), "flush"_a = true);

        sys.attr("path").cast<py::list>().append("python");
        py::print("Updated Python path:", sys.attr("path"), "flush"_a = true);
        py::module_ algorithm = py::module_::import("algorithm");
        auto result = algorithm.attr("add")(2, 3).cast<int>();
        std::cout << "Result of python.algorithm.add(2, 3): " << result
                  << std::endl;

        // adding embedded modules
        auto fast_cal = py::module_::import("fast_cal");
        auto fast_result = fast_cal.attr("add")(5, 6).cast<int>();
        std::cout << "Result of fast_cal.add(5, 6): " << fast_result
                  << std::endl;

        auto py_module = py::module_::import("py_module");
        auto a = py_module.attr("a").cast<int>();
        auto b = py_module.attr("b").cast<int>();
        std::cout << "py_module.a: " << a << std::endl;
        std::cout << "py_module.b: " << b << std::endl;

        locals =
            py::dict("fmt"_a = "{} + {} = {}", **py_module.attr("__dict__"));
        py::exec(R"(
            c = a + b
            message = fmt.format(a, b, c)
            print(message, flush=True)
        )",
                 py::globals(), locals);

        auto message3 = locals["message"].cast<std::string>();
        std::cout << "message3: " << message3 << std::endl;
    }

    py::initialize_interpreter();
    // Main GIL is held
    py::print("\nMain interpreter ID:", py::subinterpreter::main().id(),
              "flush"_a = true);
    py::print("Current interpreter ID:", py::subinterpreter::current().id(),
              "flush"_a = true);
    py::module_::import("fast_cal");
    py::print("imported `fast_cal`", "flush"_a = true);
    py::exec(R"(
        import sys
        print('"fast_cal" in sys.modules:', "fast_cal" in sys.modules, flush=True)
        print('"cpp_module" in sys.modules:', "cpp_module" in sys.modules, flush=True)
    )");

    {
        py::subinterpreter sub = py::subinterpreter::create();
        // Main interpreter is still active, main GIL re-acquired
        py::print(
            "\nSub-interpreter created, but main interpreter still active, "
            "main "
            "GIL held",
            "flush"_a = true);
        py::print("Sub-interpreter ID:", sub.id(), "flush"_a = true);
        py::print("Current interpreter ID:", py::subinterpreter::current().id(),
                  "flush"_a = true);
        {
            py::subinterpreter_scoped_activate sub_guard(sub);
            // Sub-interpreter is now active, thread holds sub's GIL
            py::print(
                "\nHello from the sub-interpreter! (sub-interpreter is now "
                "activated)",
                "flush"_a = true);
            py::print("Current interpreter ID:",
                      py::subinterpreter::current().id(), "flush"_a = true);
            py::module_::import("cpp_module");
            py::print("imported `cpp_module`", "flush"_a = true);
            py::exec(R"(
                import sys

                print('"fast_cal" in sys.modules:', "fast_cal" in sys.modules, flush=True)
                print('"cpp_module" in sys.modules:', "cpp_module" in sys.modules, flush=True)
            )");
            {
                py::subinterpreter_scoped_activate main_guard(
                    py::subinterpreter::main());
                // Sub's GIL was automatically released
                // Main interpreter active, thread holds main's GIL
                py::print(
                    "\nHello from the main interpreter! (activate main by "
                    "subinterpreter_scoped_activate)",
                    "flush"_a = true);
                py::print("Current interpreter ID:",
                          py::subinterpreter::current().id(), "flush"_a = true);
                py::exec(R"(
                    import sys

                    print('"fast_cal" in sys.modules:', "fast_cal" in sys.modules, flush=True)
                    print('"cpp_module" in sys.modules:', "cpp_module" in sys.modules, flush=True)
                )");
            }

            // Back to sub-interpreter, thread holds sub's GIL again
            py::print(
                "\nBack to sub-interpreter (sub-interpreter is re-activated)",
                "flush"_a = true);
            py::print("Current interpreter ID:",
                      py::subinterpreter::current().id(), "flush"_a = true);
            py::exec(R"(
                import sys

                print('"fast_cal" in sys.modules:', "fast_cal" in sys.modules, flush=True)
                print('"cpp_module" in sys.modules:', "cpp_module" in sys.modules, flush=True)
            )");
        }

        // Back to main-interpreter, thread holds main's GIL again
        py::print(
            "\nBack to main-interpreter (main-interpreter is re-activated)",
            "flush"_a = true);
        py::print("Current interpreter ID:", py::subinterpreter::current().id(),
                  "flush"_a = true);
    }  // destroy sub-interpreter

    {
        auto sub2 = py::subinterpreter::create();
        py::print("\nSub-interpreter2 ID: ", sub2.id(), "flush"_a = true);

        {
            py::subinterpreter_scoped_activate sub2_guard(sub2);
            py::print("\nSub-interpreter2 is activated", "flush"_a = true);
            py::print("Current interpreter ID:",
                      py::subinterpreter::current().id(), "flush"_a = true);
            py::module_::import("cpp_module");
            py::print("imported `cpp_module`", "flush"_a = true);
            py::exec(R"(
            import sys
            print('"cpp_module" in sys.modules:', "cpp_module" in sys.modules, flush=True)
        )");
        }

        {
            py::subinterpreter_scoped_activate sub2_guard(sub2);
            py::print("\nSub-interpreter2 is re-activated", "flush"_a = true);
            py::print("Current interpreter ID:",
                      py::subinterpreter::current().id(), "flush"_a = true);
            py::exec(R"(
            import sys
            print('"cpp_module" in sys.modules:', "cpp_module" in sys.modules, flush=True)
        )");
        }
    }

    py::module_::import("printer").attr("which")("\nFirst init");

    {
        py::subinterpreter sub = py::subinterpreter::create();

        py::module_::import("printer").attr("which")("Created sub");

        {
            py::subinterpreter_scoped_activate guard(sub);
            try {
                py::module_::import("printer").attr("which")("Activated sub");
            } catch (py::error_already_set& e) {
                std::cerr << "EXCEPTION " << e.what() << std::endl;
                return 1;
            }
        }

        py::module_::import("printer").attr("which")("Deactivated sub");

        {
            py::gil_scoped_release nogil;
            {
                py::subinterpreter_scoped_activate guard(sub);
                try {
                    {
                        py::subinterpreter_scoped_activate main_guard(
                            py::subinterpreter::main());
                        try {
                            py::module_::import("printer").attr("which")(
                                "Main within sub");
                        } catch (py::error_already_set& e) {
                            std::cerr << "EXCEPTION " << e.what() << std::endl;
                            return 1;
                        }
                    }
                    py::module_::import("printer").attr("which")(
                        "After Main, still within sub");
                } catch (py::error_already_set& e) {
                    std::cerr << "EXCEPTION " << e.what() << std::endl;
                    return 1;
                }
            }
        }
    }

    py::module_::import("printer").attr("which")("At end");

    std::cout << std::endl;
    {
        py::subinterpreter sub = py::subinterpreter::create();

        py::module_::import("printer").attr("which")("Before async");
        py::exec(R"(
            import threading
            print("Main thread ID:", threading.get_ident(), flush=True)
        )");

        auto ftr = std::async([&sub]() {
            py::subinterpreter_scoped_activate guard(sub);
            py::module_::import("printer").attr("which")(
                "Activated sub in async(another thread)");
            py::exec(R"(
                import threading
                print("Async thread ID:", threading.get_ident(), flush=True)
            )");
        });

        py::module_::import("printer").attr("which")("After async");

        ftr.get();
    }

    py::finalize_interpreter();

    return 0;
}