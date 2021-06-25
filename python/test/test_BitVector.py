# ------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# ------------------------------------------------------------------------------
import operator
import pytest
import random
from random import randrange

from tweedledum.bool_function_compiler import BitVec

random.seed(1337)


@pytest.fixture
def create_string_bitvectors():
    bvs = list()
    for i in range(1, 12):
        value = randrange((1 << i), (1 << (i + 1)))
        string = "{0:b}".format(value)
        bvs.append((BitVec((1 << i), value), string))
    return bvs


def test_one_bit_str():
    zero_str = BitVec(1, "0")
    assert zero_str._length == 1
    assert zero_str._value == 0


def test_one_bit_str():
    zero_str = BitVec(1, "0")
    assert zero_str._length == 1
    assert zero_str._value == 0

    one_str = BitVec(1, "1")
    assert one_str._length == 1
    assert one_str._value == 1


def test_one_bit_value():
    zero = BitVec(1, 0)
    assert zero._length == 1
    assert zero._value == 0

    one = BitVec(1, 1)
    assert one._length == 1
    assert one._value == 1


def test_fail_value_type():
    with pytest.raises(TypeError, match=r".*interger or a bit string.*"):
        BitVec(1, 1.0)


def test_fail_size_str():
    with pytest.raises(TypeError, match=r".*length 2.*length 1.*"):
        BitVec(1, "00")


def test_fail_size_value():
    with pytest.raises(TypeError, match=r".*length 4.*length 1.*"):
        BitVec(1, 10)


def test_n_bit_strings(create_string_bitvectors):
    for _, string in create_string_bitvectors:
        bv = BitVec(len(string), string)
        assert bv._length == len(string)
        assert bv._value == int(string, 2)

        bv = BitVec(string)
        assert bv._length == len(string)
        assert bv._value == int(string, 2)


def test_getitem(create_string_bitvectors):
    for bv, _ in create_string_bitvectors:
        string = str(bv)
        last = len(string) - 1
        for i in range(len(string)):
            value = bv[i]._value
            assert bv[i]._length == 1
            # Remember that the string is little-endian!
            assert value == int(string[last - i])


def test_setitem(create_string_bitvectors):
    for bv, _ in create_string_bitvectors:
        string = str(bv)
        result = BitVec(bv._length, 0)
        last = len(string) - 1
        for i in range(len(string)):
            result[i] = int(string[last - i])
        assert bv._length == result._length
        assert bv._value == result._value


def test_getslice(create_string_bitvectors):
    # Limit the length
    for bv, string in create_string_bitvectors[0:32]:
        for i in range(1, len(string) + 1):
            for j in range(len(string)):
                if i <= j:
                    with pytest.raises(ValueError, match=r".*i > j.*"):
                        bv[i:j]
                    continue
                result = bv[i:j]
                expected = string[len(string) - i : len(string) - j]
                assert result._length == len(expected)
                assert result._value == int(expected, 2)


def test_setslice(create_string_bitvectors):
    for bv, _ in create_string_bitvectors[3:]:
        string = str(bv)
        result = BitVec(bv._length, (1 << bv._length - 1) - 1)
        n = int(bv._length / 4)
        for i in range(0, len(string), n):
            part = string[i : i + n]
            result[len(string) - i : len(string) - (i + n)] = BitVec(len(part), part)
        assert result._length == bv._length
        assert result._value == bv._value


@pytest.fixture
def create_bv_operand_pairs():
    operand_pairs = list()
    for i in range(1, 12):
        left = randrange((1 << i), (1 << (i + 1)))
        right = randrange((1 << i), (1 << (i + 1)))
        operand_pairs.append((BitVec((1 << i), left), BitVec((1 << i), right)))
    return operand_pairs


def test_binops(create_bv_operand_pairs):
    for op in [operator.and_, operator.or_, operator.xor]:
        for left, right in create_bv_operand_pairs:
            result = op(left, right)
            iresult = op(right, left)
            assert result._length == left._length
            assert result._value == op(left._value, right._value)
            assert iresult._value == op(left._value, right._value)
