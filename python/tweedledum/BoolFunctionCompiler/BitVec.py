#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
class BitVec(object):
    def __init__(self, size : int, value = 0):
        if not isinstance(size, int):
            raise TypeError("BitVec size must be an int")
        self.size_ = size
        if isinstance(value, int):
            # this -2 account for the leading '0b': 0b....
            # FIXME: feels a bit hack-y, because I suck at python :(
            required_size = len(bin(value)) - 2 
            if required_size > size:
                raise TypeError("BitVec value cannot fit in size")
            self.value_ = value
        elif isinstance(value, str):
            if (len(value) > size):
                raise TypeError("BitVec string bigger than the size")
            self.value_ = int(value, base = 2)
        else:
            raise TypeError("BitVec value must be either a int or a bit string")

    def __len__(self):
        return self.size_

    def __bool__(self):
        return bool(self.value_)

    def __and__(self, other):
        if isinstance(other, BitVec):
            if (self.size_ != other.size_):
                raise TypeError("BitVec __and__: size mismatch")
            return BitVec(self.size_, self.value_ & other.value_)
        raise TypeError("BitVec __and__: type mismatch")

    def __or__(self, other):
        if isinstance(other, BitVec):
            if (self.size_ != other.size_):
                raise TypeError("BitVec __or__: size mismatch")
            return BitVec(self.size_, self.value_ | other.value_)
        raise TypeError("BitVec __or__: type mismatch")

    def __xor__(self, other):
        if isinstance(other, BitVec):
            if (self.size_ != other.size_):
                raise TypeError("BitVec __xor__: size mismatch")
            return BitVec(self.size_, self.value_ ^ other.value_)
        raise TypeError("BitVec __xor__: type mismatch")

    def __eq__(self, other):
        if isinstance(other, BitVec):
            return self.size_ == other.size and self.value_ == other.value_
        return False

    def __ne__(self, other):
        if isinstance(other, BitVec):
            return self.size_ != other.size or self.value_ != other.value_
        return False

    def __str__(self):
        return "{:0{}b}".format(self.value_, self.size_)

    def __getitem__(self, index):
        if isinstance(index, slice):
            i, j = index.start, index.stop
            if j is None:
                j = 0
            if j < 0:
                raise ValueError("BitVec[i:j] requires j >= 0\n")
            if i is None:
                return BitVec(j, self.value_ >> j)
            if i <= j:
                raise ValueError("BitVec[i:j] requires i > j\n")
            return BitVec(i - j, (self.value_ & (1 << i) - 1) >> j)
        else:
            return BitVec(1, int((self.value_ >> index) & 0x1))

    def __setitem__(self, index, value):
        if isinstance(index, slice):
            if not isinstance(value, BitVec):
                raise ValueError("BitVec[i:j] = BitVec[i:j]\n")
            i, j = index.start, index.stop
            if j is None or j < 0:
                j = 0
            if i is None:
                i = self.size_
            if i <= j:
                raise ValueError("BitVec[i:j] assignment requires i > j\n")
            if (i - j) != value.size_:
                print((i - j), value.size_)
                raise ValueError("BitVec[i:j] must be assigned a BitVec of same size\n")
            limit = (1 << (i - j))
            mask = (limit - 1) << j
            self.value_ &= ~mask
            self.value_ |= (value.value_ << j)
        else:
            if isinstance(value, BitVec):
                if value.size_ != 1:
                    raise ValueError("BitVec[i] too big\n")
                v = value.value_
            elif isinstance(value, int):
                v = value
            else:
                raise ValueError("BitVec[i] assigned either a int of a BitVec of size one\n")
            if v == 1:
                self.value_ |= (1 << index)
            elif v == 0:
                self.value_ &= ~(1 << index)
            else:
                raise ValueError("BitVec[i] = (0 or 1)\n")
