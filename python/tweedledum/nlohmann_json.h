/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <nanobind/nanobind.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace tweedledum {

inline nlohmann::json to_json(nanobind::handle const& obj)
{
    namespace nb = nanobind;

    if (obj.ptr() == nullptr || obj.is_none()) {
        return nullptr;
    }
    if (PyBool_Check(obj.ptr())) {
        return obj.ptr() == Py_True;
    }
    if (PyLong_Check(obj.ptr())) {
        return nb::cast<long>(obj);
    }
    if (PyFloat_Check(obj.ptr())) {
        return PyFloat_AsDouble(obj.ptr());
    }
    if (PyUnicode_Check(obj.ptr())) {
        Py_ssize_t size = 0;
        const char* data = PyUnicode_AsUTF8AndSize(obj.ptr(), &size);
        return std::string(data, static_cast<size_t>(size));
    }
    if (PyTuple_Check(obj.ptr()) || PyList_Check(obj.ptr())) {
        auto array = nlohmann::json::array();
        for (nb::handle const& value : obj) {
            array.push_back(to_json(value));
        }
        return array;
    }
    if (PyDict_Check(obj.ptr())) {
        auto dict = nlohmann::json::object();
        PyObject* key = nullptr;
        PyObject* value = nullptr;
        Py_ssize_t pos = 0;
        while (PyDict_Next(obj.ptr(), &pos, &key, &value)) {
            nb::str key_str = nb::str(nb::handle(key));
            Py_ssize_t size = 0;
            const char* data = PyUnicode_AsUTF8AndSize(key_str.ptr(), &size);
            dict[std::string(data, static_cast<size_t>(size))]
                = to_json(nb::handle(value));
        }
        return dict;
    }
    throw std::runtime_error("to_json not implemented for this type of object: "
                             + nb::cast<std::string>(nb::repr(obj)));
}

inline nanobind::object from_json(nlohmann::json const& json)
{
    namespace nb = nanobind;

    if (json.is_array()) {
        nb::list obj;
        for (auto const& element : json) {
            obj.append(from_json(element));
        }
        return obj;
    }
    if (json.is_boolean()) {
        return nb::cast(json.get<bool>());
    }
    if (json.is_null()) {
        return nb::none();
    }
    if (json.is_number_unsigned()) {
        return nb::cast(json.get<unsigned long>());
    }
    if (json.is_number_integer()) {
        return nb::cast(json.get<long>());
    }
    if (json.is_number_float()) {
        return nb::cast(json.get<double>());
    }
    if (json.is_string()) {
        return nb::cast(json.get<std::string>());
    }
    // If none of the above, then create a dict
    nb::dict obj;
    for (auto it = json.cbegin(); it != json.cend(); ++it) {
        obj[nb::cast(it.key())] = from_json(it.value());
    }
    return obj;
}

} // namespace tweedledum

namespace nanobind::detail {

template <>
struct type_caster<nlohmann::json> {
    NB_TYPE_CASTER(nlohmann::json, const_name("json"))

    bool from_python(handle src, uint8_t, cleanup_list*) noexcept
    {
        try {
            value = tweedledum::to_json(src);
            return true;
        } catch (...) {
            return false;
        }
    }

    static handle from_cpp(nlohmann::json const& src, rv_policy, cleanup_list*) noexcept
    {
        try {
            return tweedledum::from_json(src).release();
        } catch (...) {
            return handle();
        }
    }
};

} // namespace nanobind::detail
