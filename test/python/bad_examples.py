#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
"""These examples should be handle by the classicalfunction compiler"""

from tweedledum.bool_function_compiler import BitVec 

def constant() -> BitVec(1):
    return '1'

def constant_2bit() -> BitVec(2):
    return '10'

def identity(a : BitVec(1)) -> BitVec(2):
    return a

def identity_2bit(a : BitVec(1)) -> BitVec(2):
    return a

#-------------------------------------------------------------------------------

# Reason: Boolean NOT operator cannot be applied to a variable with multiple 
# bits
def bool_not_2bit(a : BitVec(2)) -> BitVec(2):
    return not a

#-------------------------------------------------------------------------------