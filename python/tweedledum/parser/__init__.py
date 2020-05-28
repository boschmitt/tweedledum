#-------------------------------------------------------------------------------
# Part of tweedledum.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import inspect
from .logic_network import LogicNetwork, ParseError

def func_to_ln(func):
    source = inspect.getsource(func).strip()
    return LogicNetwork(source)