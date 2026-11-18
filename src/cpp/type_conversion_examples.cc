#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/eigen.h>

namespace py = pybind11;

// Wrapper in C++(`py::list`), native type in Python(`list`)
void print_list(py::list my_list) {
    for (auto item : my_list) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

// Converting(copying all elements) between native C++(`std::vector`) and Python
// types(`list`)
void print_vector(const std::vector<int>& v) {
    for (auto item : v) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

void print_bytes(py::bytes b) { std::cout << "Bytes: " << b << std::endl; }

char utf8_chars[1024] = {static_cast<char>(0xF0),
                         static_cast<char>(0x9F),
                         static_cast<char>(0x8E),
                         static_cast<char>(0x82),  // 🎂
                         static_cast<char>(0xF0),
                         static_cast<char>(0x9F),
                         static_cast<char>(0x8D),
                         static_cast<char>(0x95),  // 🍕
                         0x00};

PYBIND11_EMBEDDED_MODULE(type_conversion_examples, m) {
    m.def("print_list", &print_list);
    m.def("print_vector", &print_vector);
}

PYBIND11_EMBEDDED_MODULE(string_conv_examples, m) {
    m.def("utf8_test", [](const std::string& s) {
        std::cout << "utf-8 is icing on the cake: " << s << std::endl;
    });
    m.def("utf8_charptr", [](const char* s) {
        std::cout << "My favorite string: " << s << std::endl;
    });
    m.def("print_bytes", &print_bytes);

    m.def("std_string_return", []() {
        return std::string{"This string needs to be UTF-8 encoded😉"};
    });
    m.def("char_ptr_return", []() -> char* { return utf8_chars; });

    // This uses the Python C API to convert Latin-1 to Unicode
    m.def("str_output", []() {
        std::string s = "Send your r\xe9sum\xe9 to Alice in HR";  // Latin-1
        py::handle py_s = PyUnicode_DecodeLatin1(s.data(), s.length(), nullptr);
        if (!py_s) {
            throw py::error_already_set();
        }
        return py::reinterpret_steal<py::str>(py_s);
    });

    m.def("return_bytes", []() {
        std::string s("\xba\xd0\xba\xd0");  // Not valid UTF-8
        return py::bytes(s);  // Return the data without transcoding
    });

    m.def("asymmetry",
          [](std::string s) {  // Accepts str or bytes from Python
              return s;        // Looks harmless, but implicitly converts to str
          });

    m.def("pass_char", [](char c) { return c; });
    m.def("pass_wchar", [](wchar_t c) { return c; });
}

void TestStringModule() {
    py::exec(R"(
        from string_conv_examples import (
            utf8_test,
            utf8_charptr,
            print_bytes,
            std_string_return,
            char_ptr_return,
            str_output,
            return_bytes,
            asymmetry,
            pass_char,
            pass_wchar
        )
        
        utf8_test("🎂");
        utf8_test(b"\xF0\x9F\x8E\x82");
        utf8_charptr("🍕");
        utf8_charptr(b"\xF0\x9F\x8D\x95");
        print_bytes(b"\xF0\x9F\x8D\x95");
        print("-----\n")
        
        x = std_string_return()
        print(f"type(std_string_return()): {type(x)}")
        print(f"std_string_return(): {x}")
        
        y = char_ptr_return()
        print(f"type(char_ptr_return()): {type(y)}")
        print(f"char_ptr_return(): {y}")
        print("-----\n")
        
        print(f"str_output(): {str_output()}")
        print("-----\n")

        print(f"return_bytes(): {return_bytes()}")
        print("-----\n")
        
        try:
            x = asymmetry(b'test')
            print(f"asymmetry(b'test'): {type(x)} {x}")

            y = asymmetry(b'\xba\xd0\xba\xd0')
            print(f"asymmetry(b'\xba\xd0\xba\xd0'): {type(y)} {y}")
        except Exception as e:
            print(f"Error: {e}")
        print("-----\n")

        print(f"pass_char('a'): {pass_char('a')}")

        try:
            print(f"pass_char('abcdef'): {pass_char('abcdef')}")
        except Exception as e: # 문서에선 첫번째 문자만 넘어간댔는데 에러남; 
            print(f"Error at pass_char('abcdef'): {e}")

        try:
            print(f"pass_wchar('ab'): {pass_wchar('ab')}")
        except Exception as e: # 문서에선 첫번째 문자만 넘어간댔는데 에러남;
            print(f"Error at pass_wchar('ab'): {e}")

        print(f"pass_wchar('あ'): {pass_wchar('あ')}")

        try:
            print(f"pass_char(''): {pass_char('')}")
        except Exception as e: # Empty str 전달 시도 에러
            print(f"Error at pass_char(''): {e}")

        try:
            print(f"pass_char(0x65): {pass_char(0x65)}")
        except Exception as e: # int 전달 시도 에러
            print(f"Error at pass_char(0x65): {e}")

        print(f"pass_char(chr(0x65)): {pass_char(chr(0x65))}")

        print(f"pass_wchar('é'): {pass_wchar('é')}")
        combining_e_acute = "e" + "\u0301"
        print(f"combining_e_acute: {combining_e_acute}")
        print(f"combining_e_acuate == 'é': {combining_e_acute == 'é'}")
        try:
            print(f"pass_wchar(combining_e_acute): {pass_wchar(combining_e_acute)}")
        except Exception as e: # Combining character 전달 시도 에러
            print(f"Error at pass_wchar(combining_e_acute): {e}")
        import unicodedata
        print(f"normalized combining_e_acute: {pass_wchar(unicodedata.normalize('NFC', combining_e_acute))}")

        print("-----\n")
    )");
}

PYBIND11_EMBEDDED_MODULE(stl_conv_examples, m) {
    // Copy and modify a C++ vector from Python(not affected to the original
    // python's `list`)
    // In order to avoid this situation, use macro `PYBIND11_MAKE_OPAQUE`
    m.def("append_1", [](std::vector<int>& v) { v.push_back(1); });
}

void TestStlModule() {
    py::exec(R"(
        from stl_conv_examples import (
            append_1
        )
        
        v = [5, 6]
        print(f"v before append_1: {v}")
        append_1(v)
        print(f"v after append_1: {v}")
        print("-----\n")
    )");
}

int func_arg(const std::function<int(int)>& f) { return f(10); }

std::function<int(int)> func_ret(const std::function<int(int)>& f) {
    return [f](int i) { return f(i) + 1; };
}

py::cpp_function func_cpp() {
    return py::cpp_function([](int i) { return i + 1; }, py::arg("number"));
}

PYBIND11_EMBEDDED_MODULE(func_conv_examples, m) {
    m.def("func_arg", &func_arg);
    m.def("func_ret", &func_ret);
    m.def("func_cpp", &func_cpp);
}

void TestFuncModule() {
    py::exec(R"(
        from func_conv_examples import (
            func_arg,
            func_ret,
            func_cpp
        )

        def square(i):
            return i * i

        print(f"func_arg(square): {func_arg(square)}") # square(10) = 100
        square_plus_1 = func_ret(square) # square_plus_1 := square(i) + 1
        print(f"square_plus_1(4): {square_plus_1(4)}") # square(4) + 1 = 17
        plus_1 = func_cpp()
        print(f"plus_1(number=43): {plus_1(number=43)}") # 43 + 1 = 44
        print("-----\n")
    )");
}

class MyClass {
public:
    Eigen::MatrixXd& GetMatrix() { return big_mat_; }
    const Eigen::MatrixXd& ViewMatrix() const { return big_mat_; }

private:
    Eigen::MatrixXd big_mat_ = Eigen::MatrixXd::Zero(10000, 10000);
};

PYBIND11_EMBEDDED_MODULE(eigen_examples, m) {
    using namespace pybind11::literals;  // for "_a" literal used in arg()

    py::class_<MyClass>(m, "MyClass")
        .def(py::init<>())
        .def("CopyMatrix", &MyClass::GetMatrix)  // Makes a copy
        .def("GetMatrix", &MyClass::GetMatrix,
             py::return_value_policy::reference_internal)
        .def("ViewMatrix", &MyClass::ViewMatrix,
             py::return_value_policy::reference_internal);

    // Use `py::EigenDRef` to accept Eigen matrices from Python without
    // copying(considering storage order)
    m.def("scale", [](py::EigenDRef<Eigen::MatrixXd> m, double c) { m *= c; });

    // or using row major Eigen::MatrixXd
    using RowMajorMatrixXd =
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
    m.def("scale_row_major",
          [](Eigen::Ref<RowMajorMatrixXd> m, double c) { m *= c; });

    // or pass col major numpy array(order='F')
    m.def("scale_col_major",
          [](Eigen::Ref<Eigen::MatrixXd> m, double c) { m *= c; });

    // binding `Eigen::Ref<const MatrixType>` makes copy if needed,
    // use `py::arg()::noconvert()` to avoid copying
    m.def(
        "some_function",
        [](const Eigen::Ref<const Eigen::MatrixXd>& big,
           const Eigen::Ref<const Eigen::MatrixXd>& small) {
            return big.sum() + small.sum();
        },
        "big"_a.noconvert(),  // <- Don't allow copying for this arg
        "small"_a             // <- This one can be copied if needed
    );
}

void TestEigenModule() {
    py::exec(R"!(
        import numpy as np
        from eigen_examples import (
            MyClass, 
            scale, 
            scale_row_major, 
            scale_col_major,
            some_function
        )

        a = MyClass()
        m = a.GetMatrix()  # flags.writeable = True,  flags.owndata = False
        v = a.ViewMatrix()  # flags.writeable = False, flags.owndata = False
        c = a.CopyMatrix()  # flags.writeable = True,  flags.owndata = True
        # m[5,6] and v[5,6] refer to the same element, c[5,6] does not.

        print(f"m: {type(m)}, flags.writeable = {m.flags.writeable}, flags.owndata = {m.flags.owndata}")
        print(f"v: {type(v)}, flags.writeable = {v.flags.writeable}, flags.owndata = {v.flags.owndata}")
        print(f"c: {type(c)}, flags.writeable = {c.flags.writeable}, flags.owndata = {c.flags.owndata}")

        m[5,6] = 42
        print(f"After modifying m[5,6] = 42")
        print(f"m[5,6] = {m[5,6]}")
        print(f"v[5,6] = {v[5,6]}")
        print(f"c[5,6] = {c[5,6]}")

        x = np.ones((6,9))
        print(f"x:\n{x}")
        scale(x[0::2, 2:9:3], 2)
        print(f"x after scale(x[0::2, 2:9:3], 2) with slicing:\n{x}")

        scale_row_major(x, 3)
        print(f"x after scale_row_major(x, 3):\n{x}")

        y = np.array(np.arange(9).reshape(3,3), dtype=np.float64, order='F')
        print(f"y:\n{y}")
        scale_col_major(y, 4)
        print(f"y after scale_col_major(y, 4):\n{y}")

        y = y.transpose()
        print(f"y after transpose:\n{y}")
        try:
            scale_col_major(y, 1/4) # This will likely raise an error 
                                    # because y is no longer column-major 
                                    # after transpose
            print(f"y after scale_col_major(y, 1/4):\n{y}")
        except Exception as e:
            print("Error occurred when calling scale_col_major(y, 1/4)",
             f"with non-column-major y: {e}")

        col_major = np.ones((2000, 2000), dtype=np.float64, order='F')
        row_major = np.ones((2,2), dtype=np.float64, order='C')
        print("some_function(big=col_major, small=row_major):", 
            some_function(big=col_major, small=row_major))
        try:
            print("some_function(big=row_major, small=row_major):", 
                some_function(big=row_major, small=row_major))
        except Exception as e:
            print("Error occurred when calling",
                "some_function(big=row_major, small=row_major)",
                f"with mismatched order: {e}")

        print("-----\n")
    )!");
}

int main() {
    py::scoped_interpreter guard{};

    try {
        // Overview
        py::exec(R"(
            from type_conversion_examples import print_list, print_vector

            print_list([1,2,3]) # Python `list` is wrapped to C++ `py::list`
            print_vector([4,5,6]) # Python `list` is converted to C++ `std::vector`
            print("-----\n")
        )");

        TestStringModule();
        TestStlModule();
        TestFuncModule();
        TestEigenModule();
    } catch (const py::error_already_set& e) {
        std::cerr << "Python error: " << e.what() << std::endl;
    }

    return 0;
}