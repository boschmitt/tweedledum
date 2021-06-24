/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <tweedledum/IR/Cbit.h>
#include <tweedledum/IR/Qubit.h>

void init_Wires(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

#define QUBIT_OR_CBIT_INSTANTIATION(T)                                        \
    py::class_<T> bit_##T(module, #T);                                        \
    py::enum_<T::Polarity>(bit_##T, "Polarity")                               \
      .value("negative", T::Polarity::negative)                               \
      .value("positive", T::Polarity::positive)                               \
      .export_values();                                                       \
                                                                              \
    bit_##T.def_static("invalid", &T::invalid)                                \
      .def(py::init<uint32_t, T::Polarity>(), py::arg("uid"),                 \
        py::arg("polarity") = T::Polarity::positive)                          \
      .def("polarity", &T::polarity)                                          \
      .def("uid", &T::uid)                                                    \
      .def(                                                                   \
        "__index__", [](T const& bit) { return static_cast<uint32_t>(bit); }) \
      .def("__invert__", [](T const& lhs) { return !lhs; })                   \
      .def(+py::self)                                                         \
      .def(-py::self)                                                         \
      .def(py::self == py::self)                                              \
      .def(py::self != py::self);

    QUBIT_OR_CBIT_INSTANTIATION(Cbit);
    QUBIT_OR_CBIT_INSTANTIATION(Qubit);
#undef QUBIT_OR_CBIT_INSTANTIATION
}
