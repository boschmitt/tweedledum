/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "Operators.h"

#include <pybind11/pybind11.h>

PYBIND11_MODULE(libTweedledumOp, module)
{
    namespace py = pybind11;
    module.doc() = "tweedledum Operators";

    init_ext_operators(module);
    init_ising_operators(module);
    init_meta_operators(module);
    init_std_operators(module);
}
