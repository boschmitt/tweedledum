#-------------------------------------------------------------------------------
# Part of tweedledum.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
from typing import NewType
from bitstring import BitArray

def BAtype(length):
    return type('int%s' % length, (BitArray, ), {'length': length})

Bit = NewType('Bit', bool)
int2 = BAtype(2)
