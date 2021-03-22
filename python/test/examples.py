#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
"""These examples should be handle by the classicalfunction compiler"""

from tweedledum.bool_function_compiler import BitVec 

def constant() -> BitVec(1):
    return BitVec(1, '1')

def constant_2bit() -> BitVec(2):
    return BitVec(2, '10')

def constant_3bit() -> BitVec(3):
    return BitVec('101')

def constant_4bit() -> BitVec(4):
    return BitVec(4)

#-------------------------------------------------------------------------------

def identity(a : BitVec(1)) -> BitVec(1):
    return a

def identity_2bit(a : BitVec(2)) -> BitVec(2):
    return a

def identity_not(a : BitVec(2)) -> (BitVec(2), BitVec(2)):
    return a, ~a

#-------------------------------------------------------------------------------

def bool_not(a : BitVec(1)) -> BitVec(1):
    return not a

def bit_not(a : BitVec(1)) -> BitVec(1):
    return ~a

def bit_not_2bit(a : BitVec(2)) -> BitVec(2):
    return ~a

#-------------------------------------------------------------------------------

def bool_and(a, b : BitVec(1)) -> BitVec(1):
    return a and b

def bit_and(a, b : BitVec(1)) -> BitVec(1):
    return a & b

def bit_and_2bit(a, b : BitVec(2)) -> BitVec(2):
    return a & b

#-------------------------------------------------------------------------------

def bool_or(a, b : BitVec(1)) -> BitVec(1):
    return a or b

def bit_or(a, b : BitVec(1)) -> BitVec(1):
    return a | b

def bit_or_2bit(a, b : BitVec(2)) -> BitVec(2):
    return a | b

#-------------------------------------------------------------------------------

def bit_xor(a, b : BitVec(1)) -> BitVec(1):
    return a ^ b

def bit_xor_2bit(a, b : BitVec(2)) -> BitVec(2):
    return a ^ b

#-------------------------------------------------------------------------------

def eq(a, b : BitVec(1)) -> BitVec(1):
    return a == b

def eq_2bit(a, b : BitVec(2)) -> BitVec(1):
    return a == b

#-------------------------------------------------------------------------------

def ne(a, b : BitVec(1)) -> BitVec(1):
    return a != b

def ne_2bit(a, b : BitVec(2)) -> BitVec(1):
    return a != b