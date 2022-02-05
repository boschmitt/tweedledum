#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
from .bitvec import BitVec

class Integer(BitVec):
    """Class to represent an integer on top of a bit vector
    """

    def __init__(self, length, value = 0):
        """
        Parameters
        ----------
        length : int
            length of the vector 
        value : int or str (bit string), optional
            the initial value (default = 0)
        """
        super.__init__(length, value)

    def __len__(self):
        return self._length

    def __bool__(self):
        if self._length > 1:
            raise TypeError("[BitVec] A BitVec of length bigger than one "
                            "cannot be converted to a Boolean value")
        return bool(self._value)

    def __invert__ (self):
        return BitVec(self._length, ~self._value & (1 << self._length) - 1)

    def __and__(self, other):
        if isinstance(other, BitVec):
            if (self._length != other._length):
                raise TypeError("[BitVec] __and__ operation: length mismatch")
            return BitVec(self._length, self._value & other._value)
        raise TypeError("[BitVec] __and__ operation: type mismatch")

    def __or__(self, other):
        if isinstance(other, BitVec):
            if (self._length != other._length):
                raise TypeError("[BitVec] __or__ operation: length mismatch")
            return BitVec(self._length, self._value | other._value)
        raise TypeError("[BitVec] __or__ operation: type mismatch")

    def __xor__(self, other):
        if isinstance(other, BitVec):
            if (self._length != other._length):
                raise TypeError("[BitVec] __xor__ operation: length mismatch")
            return BitVec(self._length, self._value ^ other._value)
        raise TypeError("[BitVec] __xor__ operation: type mismatch")

    def __eq__(self, other):
        if isinstance(other, BitVec):
            return BitVec(1, self._length == other._length and self._value == other._value)
        raise TypeError("__eq__ operation: type mismatch")

    def __ne__(self, other):
        if isinstance(other, BitVec):
            return BitVec(1, self._length != other._length or self._value != other._value)
        raise TypeError("__ne__ operation: type mismatch")

    def __repr__(self):
        return f"BitVec({self._length}, '{str(self)}')"

    def __str__(self):
        return "{:0{}b}".format(self._value, self._length)
