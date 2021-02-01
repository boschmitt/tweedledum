#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
class BitVec(object):
    """Class to compactly represent a vector of bits

    The BitVec type is an unconstrained vector. During its declaration the
    length must be specified and cannot be modified later.

    The class implements an indexing interface that provides access to the bits
    of the integer representation. Hence, assignments to a BitVec can be done 
    using single element assignments or slices. Note that lower indices 
    correspond to less significant bits. (Little endian: bn .. b3 b2 b1 b0).
    Example:
        >>> x = BitVec(4, '0101')
        >>> x[0]
        1
        >>> x[1]
        0
        >>> x[2]
        1
        >>> x[3]
        0

    Unlike standard Python, slicing ranges are downward. Thus, the highest index
    bit corresponds to the leftmost item. As in standard Python, the slicing
    range is half-open: this highest index bit is not included.
    Example:
        >>> x = BitVec(4, '0101')
        >>> x[3:0]
        '101'
        >>> x[3:1]
        '10'
        >>> x[4:0]
        '0101'
    
    Both indices can be omitted from the slice. If the leftmost index is
    omitted, the meaning is to access “all” higher order bits.  If the rightmost
    index is omitted, it is 0 by default. 
    Example:
        >>> x = BitVec(4, '0101')
        >>> x[:1]
        '010'
        >>> x[2:]
        '01'

    Attributes
    ----------
    length_ : int
        length of the vector 
    value_ : int
        an integer used to compactly store the bits
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
        if isinstance(length, str) and value == 0:
            value = length
            length = len(value)
        if not isinstance(length, int):
            raise TypeError("[BitVec] The length must be an integer value")
        self.length_ = length
        if isinstance(value, int):
            # this -2 account for the leading '0b': 0b....
            # FIXME: feels a bit hack-y, because I suck at python :(
            required_length = len(bin(value)) - 2 
            if required_length > length:
                raise TypeError(f"[BitVec] Value requires a bit vector of "
                                f"length {required_length}, but BitVec has "
                                f"length {length}")
            self.value_ = value
        elif isinstance(value, str):
            if (len(value) > length):
                raise TypeError(f"[BitVec] String requires a bit vector of "
                                f"length {len(value)}, but BitVec has length " 
                                f"{length}")
            self.value_ = int(value, base = 2)
        else:
            raise TypeError("[BitVec] The value must be either an interger or "
                            "a bit string")

    def __len__(self):
        return self.length_

    def __bool__(self):
        if self.length_ > 1:
            raise TypeError("[BitVec] A BitVec of length bigger than one "
                            "cannot be converted to a Boolean value")
        return bool(self.value_)

    def __invert__ (self):
        return BitVec(self.length_, ~self.value_ & (1 << self.length_) - 1)

    def __and__(self, other):
        if isinstance(other, BitVec):
            if (self.length_ != other.length_):
                raise TypeError("[BitVec] __and__ operation: length mismatch")
            return BitVec(self.length_, self.value_ & other.value_)
        raise TypeError("[BitVec] __and__ operation: type mismatch")

    def __or__(self, other):
        if isinstance(other, BitVec):
            if (self.length_ != other.length_):
                raise TypeError("[BitVec] __or__ operation: length mismatch")
            return BitVec(self.length_, self.value_ | other.value_)
        raise TypeError("[BitVec] __or__ operation: type mismatch")

    def __xor__(self, other):
        if isinstance(other, BitVec):
            if (self.length_ != other.length_):
                raise TypeError("[BitVec] __xor__ operation: length mismatch")
            return BitVec(self.length_, self.value_ ^ other.value_)
        raise TypeError("[BitVec] __xor__ operation: type mismatch")

    def __eq__(self, other):
        if isinstance(other, BitVec):
            return self.length_ == other.length_ and self.value_ == other.value_
        return False

    def __ne__(self, other):
        if isinstance(other, BitVec):
            return self.length_ != other.length_ or self.value_ != other.value_
        return False

    def __repr__(self):
        return f'BitVec("{self.length_}","{self.value_}")'

    def __str__(self):
        return "{:0{}b}".format(self.value_, self.length_)

    def __getitem__(self, index):
        if isinstance(index, slice):
            i, j = index.start, index.stop
            if j is None:
                j = 0
            if j < 0:
                raise ValueError("[BitVec] Slice [i:j] requires j >= 0\n")
            if i is None:
                return BitVec(j, self.value_ >> j)
            if i <= j:
                raise ValueError("[BitVec] Slice [i:j] requires i > j\n")
            return BitVec(i - j, (self.value_ & (1 << i) - 1) >> j)
        else:
            return BitVec(1, int((self.value_ >> index) & 0x1))

    def __setitem__(self, index, value):
        if isinstance(index, slice):
            if not isinstance(value, BitVec):
                raise TypeError("[BitVec] Assignment operation: type mismatch")
            i, j = index.start, index.stop
            if j is None or j < 0:
                j = 0
            if i is None:
                i = self.length_
            if i <= j:
                raise ValueError("[BitVec] Slice [i:j] requires i > j\n")
            if (i - j) != value.length_:
                raise ValueError("[BitVec] Assignment requires BitVec of with "
                                 "the same length\n")
            limit = (1 << (i - j))
            mask = (limit - 1) << j
            self.value_ &= ~mask
            self.value_ |= (value.value_ << j)
        else:
            if isinstance(value, BitVec):
                if value.length_ != 1:
                    raise ValueError("[BitVec] Single element assignment "
                                     "requires an interger value (0 or 1) or a "
                                     "BitVec of length one")
                v = value.value_
            elif isinstance(value, int):
                v = value
            else:
                raise ValueError("[BitVec] Single element assignment requires "
                                 "an interger value (0 or 1) or a "
                                 "BitVec of length one")
            if v == 1:
                self.value_ |= (1 << index)
            elif v == 0:
                self.value_ &= ~(1 << index)
            else:
                raise ValueError("[BitVec] Single element assignment requires "
                                 "an interger value (0 or 1) or a "
                                 "BitVec of length one")
