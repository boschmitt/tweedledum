/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <tweedledum/IR/Cbit.h>
#include <tweedledum/IR/Qubit.h>

void init_Wires(nanobind::module_& module)
{
    using namespace tweedledum;
    namespace nb = nanobind;

#define QUBIT_OR_CBIT_INSTANTIATION(T)                                        \
    nb::class_<T> bit_##T(module, #T);                                        \
    nb::enum_<T::Polarity>(bit_##T, "Polarity")                               \
      .value("negative", T::Polarity::negative)                               \
      .value("positive", T::Polarity::positive)                               \
      .export_values();                                                       \
                                                                              \
    bit_##T.def_static("invalid", &T::invalid)                                \
      .def(nb::init<uint32_t, T::Polarity>(), nb::arg("uid"),                 \
        nb::arg("polarity") = T::Polarity::positive)                          \
      .def("polarity", &T::polarity)                                          \
      .def("uid", &T::uid)                                                    \
      .def(                                                                   \
        "__index__", [](T const& bit) { return static_cast<uint32_t>(bit); }) \
      .def("__invert__", [](T const& lhs) { return !lhs; })                   \
      .def(+nb::self)                                                         \
      .def(-nb::self)                                                         \
      .def(nb::self == nb::self)                                              \
      .def(nb::self != nb::self);

    QUBIT_OR_CBIT_INSTANTIATION(Cbit);
    QUBIT_OR_CBIT_INSTANTIATION(Qubit);
#undef QUBIT_OR_CBIT_INSTANTIATION
}
