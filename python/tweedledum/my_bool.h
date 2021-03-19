/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <stdexcept>
#include <tweedledum/Utils/Matrix.h>

// pybind11 wizardry
// Based on the discussion and solution provided in:
//      https://github.com/pybind/pybind11/issues/1776
namespace pybind11::detail {

template <>
struct npy_format_descriptor<tweedledum::MyBool> {
    static constexpr auto name = _("MyBool");

    static pybind11::dtype dtype()
    {
        handle ptr = npy_api::get().PyArray_DescrFromType_(npy_api::NPY_BOOL_);
        return reinterpret_borrow<pybind11::dtype>(ptr);
    }
};

}
