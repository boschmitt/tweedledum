/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "Circuit.h"
#include "Instruction.h"
#include "../Operators/Operators.h"

#include <pybind11/pybind11.h>

PYBIND11_MODULE(libTweedledumIR, module)
{
    namespace py = pybind11;
    module.doc() = "tweedledum IR";

    init_Instruction(module);
    init_Circuit(module);
}
