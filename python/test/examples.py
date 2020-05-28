#-------------------------------------------------------------------------------
# Part of tweedledum.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
from tweedledum.parser.types import Bit, int2

def id(a: Bit) -> Bit:
    return a

def id_no_type_arg(a) -> Bit:
    return a

def id_no_type_return(a: Bit):
    return a

def id_bad_return(a: Bit) -> int2:
    return a

def bit_and(a: Bit, b: Bit) -> Bit:
    return a & b

def bit_or(a: Bit, b: Bit) -> Bit:
    return a | b

def bool_or(a: Bit, b: Bit) -> Bit:
    return a or b

def bit_not(a: Bit) -> Bit:
    return ~a

def bool_not(a: Bit) -> Bit:
    return not a

def multiple_binop(a: Bit, b: Bit) -> Bit:
    return (a or b) | (b & a) and (a & b)

def out_of_scope(a: Bit) -> Bit:
    return a & c

def id_assing(a: Bit) -> Bit:
    b = a
    return b

def example1(a: Bit, b: Bit) -> Bit:
    c = a & b
    d = b | a
    return c ^ a | d

def grover_oracle(a: Bit, b: Bit, c: Bit, d: Bit) -> Bit:
    return (not a and b and not c and d)
