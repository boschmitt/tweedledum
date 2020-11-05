/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  
|
| I used to have my own conversion functions.  Now, they are pretty much what 
| can be found in: (I really liked the macro things :)
| https://github.com/pybind/pybind11_json/blob/master/include/pybind11_json/pybind11_json.hpp
|
| It does have small differences from the above (maybe not significant)!
|
| BSD 3-Clause License
| 
| Copyright (c) 2019,
| All rights reserved.
| 
| Redistribution and use in source and binary forms, with or without
| modification, are permitted provided that the following conditions are met:
| 
| * Redistributions of source code must retain the above copyright notice, this
|   list of conditions and the following disclaimer.
| 
| * Redistributions in binary form must reproduce the above copyright notice,
|   this list of conditions and the following disclaimer in the documentation
|   and/or other materials provided with the distribution.
| 
| * Neither the name of the copyright holder nor the names of its
|   contributors may be used to endorse or promote products derived from
|   this software without specific prior written permission.
| 
| THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
| AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
| IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
| DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
| FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
| DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
| SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
| CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
| OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
| OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
*-----------------------------------------------------------------------------*/
#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>
#include <stdexcept>

namespace tweedledum {

inline nlohmann::json to_json(pybind11::handle const& obj)
{
    if (obj.ptr() == nullptr || obj.is_none()) {
        return nullptr;
    }
    if (pybind11::isinstance<pybind11::bool_>(obj)) {
        return obj.cast<bool>();
    }
    if (pybind11::isinstance<pybind11::int_>(obj)) {
        return obj.cast<long>();
    }
    if (pybind11::isinstance<pybind11::float_>(obj)) {
        return obj.cast<double>();
    }
    if (pybind11::isinstance<pybind11::str>(obj)) {
        return obj.cast<std::string>();
    }
    if (pybind11::isinstance<pybind11::tuple>(obj) || pybind11::isinstance<pybind11::list>(obj)) {
        auto array = nlohmann::json::array();
        for (pybind11::handle const& value : obj) {
            array.push_back(to_json(value));
        }
        return array;
    }
    if (pybind11::isinstance<pybind11::dict>(obj)) {
        auto dict = nlohmann::json::object();
        for (pybind11::handle const& key : obj) {
            dict[pybind11::str(key).cast<std::string>()] = to_json(obj[key]);
        }
        return dict;
    }
    throw std::runtime_error("to_json not implemented for this type of object: "
                             + pybind11::repr(obj).cast<std::string>());
}

inline pybind11::object from_json(nlohmann::json const& json)
{
    if (json.is_array()) {
        pybind11::list obj;
        for (auto const& element : json) {
            obj.append(from_json(element));
        }
        return std::move(obj);
    }
    if (json.is_boolean()) {
        return pybind11::bool_(json.get<bool>());
    }
    if (json.is_null()) {
        return pybind11::none();
    }
    if (json.is_number_unsigned()) {
        return pybind11::int_(json.get<unsigned long>());
    }
    if (json.is_number_integer()) {
        return pybind11::int_(json.get<long>());
    }
    if (json.is_number_float()) {
        return pybind11::float_(json.get<double>());
    }
    if (json.is_string()) {
        return pybind11::str(json.get<std::string>());
    }
    // If none of the above, then create a dict 
    pybind11::dict obj;
    for (auto it = json.cbegin(); it != json.cend(); ++it) {
        obj[pybind11::str(it.key())] = from_json(it.value());
    }
    return std::move(obj);
}

}

namespace nlohmann {

#define JSON_SERIALIZER_DESERIALIZER(T)                \
template <>                                            \
struct adl_serializer<T>                               \
{                                                      \
    inline static void to_json(json& j, T const& obj)  \
    {                                                  \
        j = tweedledum::to_json(obj);                  \
    }                                                  \
                                                       \
    inline static T from_json(json const& j)           \
    {                                                  \
        return tweedledum::from_json(j);               \
    }                                                  \
}

JSON_SERIALIZER_DESERIALIZER(pybind11::bool_);
JSON_SERIALIZER_DESERIALIZER(pybind11::dict);
JSON_SERIALIZER_DESERIALIZER(pybind11::float_);
JSON_SERIALIZER_DESERIALIZER(pybind11::int_);
JSON_SERIALIZER_DESERIALIZER(pybind11::list);
JSON_SERIALIZER_DESERIALIZER(pybind11::object);
JSON_SERIALIZER_DESERIALIZER(pybind11::str);
JSON_SERIALIZER_DESERIALIZER(pybind11::tuple);
#undef JSON_SERIALIZER_DESERIALIZER

#define JSON_SERIALIZER(T)                             \
template <>                                            \
struct adl_serializer<T>                               \
{                                                      \
    inline static void to_json(json& j, T const& obj)  \
    {                                                  \
        j = tweedledum::to_json(obj);                  \
    }                                                  \
}

JSON_SERIALIZER(pybind11::handle);
JSON_SERIALIZER(pybind11::detail::item_accessor);
JSON_SERIALIZER(pybind11::detail::list_accessor);
JSON_SERIALIZER(pybind11::detail::obj_attr_accessor);
JSON_SERIALIZER(pybind11::detail::sequence_accessor);
JSON_SERIALIZER(pybind11::detail::str_attr_accessor);
JSON_SERIALIZER(pybind11::detail::tuple_accessor);
#undef JSON_SERIALIZER
}

namespace pybind11::detail {

template<>
struct type_caster<nlohmann::json> {
public:
    /**
     * This macro establishes the name 'json' in function signatures and
     * declares a local variable 'value' of type json
     */
    PYBIND11_TYPE_CASTER(nlohmann::json, _("json"));

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into a json instance
     * or return false upon failure.  The second argument indicates whether
     * implicit conversions should be applied.
     */
    bool load(handle src, bool)
    {
        try {
            value = tweedledum::to_json(src);
            return true;
        } catch (...) {
            return false;
        }
    }

     /**
     * Conversion part 2 (C++ -> Python): convert a json instance into a Python
     * object.  The second and third arguments are used to indicate the return
     * value policy and parent object
     * (for ``return_value_policy::reference_internal``) and are generally
     * ignored by implicit casters.
     */
    static handle cast(nlohmann::json src, return_value_policy, handle)
    {
        object obj = tweedledum::from_json(src);
        return obj.release();
    }
};

}
