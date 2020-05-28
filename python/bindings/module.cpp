/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <pybind11/pybind11.h>

void init_mockturtle(pybind11::module &);
void init_tweedledum(pybind11::module &);

PYBIND11_MODULE(core, m)
{
	namespace py = pybind11;
	m.doc() = "tweedledum core";
	init_mockturtle(m);
	init_tweedledum(m);
}
