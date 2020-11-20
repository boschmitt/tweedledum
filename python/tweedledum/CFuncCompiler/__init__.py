#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
from .types import *
from .Parser import *
from .CFunc import *

def classical_function(func, comp_type = CompilationType.COMPUTATIONAL_BASIS):
    import inspect
    source = inspect.getsource(func).strip()
    return CFunc(comp_type, source, func.__name__)

def phase_oracle(func):
    return classical_function(func, CompilationType.PHASE)

def computational_basis_oracle(func):
    return classical_function(func, CompilationType.COMPUTATIONAL_BASIS)
