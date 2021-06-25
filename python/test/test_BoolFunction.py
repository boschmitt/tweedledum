# ------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# ------------------------------------------------------------------------------
from tweedledum.bool_function_compiler import BitVec, BoolFunction

def constant() -> BitVec(1):
    return BitVec(1, '1')

def constant_2bit() -> BitVec(2):
    return BitVec(2, '10')

def constant_3bit() -> BitVec(3):
    return BitVec('101')

def constant_4bit() -> BitVec(4):
    return BitVec(4)

def identity(a : BitVec(1)) -> BitVec(1):
    return a

def identity_2bit(a : BitVec(2)) -> BitVec(2):
    return a

def identity_not(a : BitVec(2)) -> (BitVec(2), BitVec(2)):
    return a, ~a

def bool_not(a : BitVec(1)) -> BitVec(1):
    return not a

def bit_not(a : BitVec(1)) -> BitVec(1):
    return ~a

def bit_not_2bit(a : BitVec(2)) -> BitVec(2):
    return ~a

def bool_and(a, b : BitVec(1)) -> BitVec(1):
    return a and b

def bit_and(a, b : BitVec(1)) -> BitVec(1):
    return a & b

def bit_and_2bit(a, b : BitVec(2)) -> BitVec(2):
    return a & b

def bool_or(a, b : BitVec(1)) -> BitVec(1):
    return a or b

def bit_or(a, b : BitVec(1)) -> BitVec(1):
    return a | b

def bit_or_2bit(a, b : BitVec(2)) -> BitVec(2):
    return a | b

def bit_xor(a, b : BitVec(1)) -> BitVec(1):
    return a ^ b

def bit_xor_2bit(a, b : BitVec(2)) -> BitVec(2):
    return a ^ b

def eq(a, b : BitVec(1)) -> BitVec(1):
    return a == b

def eq_2bit(a, b : BitVec(2)) -> BitVec(1):
    return a == b

def ne(a, b : BitVec(1)) -> BitVec(1):
    return a != b

def ne_2bit(a, b : BitVec(2)) -> BitVec(1):
    return a != b

class TestBoolFunction:
    def test_constant_3bit(self):
        function = BoolFunction(constant_3bit)
        assert function._parameters_signature == []
        result = constant_3bit()
        assert result == BitVec(3, "101")

    def test_identity(self):
        function = BoolFunction(identity)
        assert function._parameters_signature == [(type(BitVec(1)), 1)]
        result = identity(BitVec(1, "0"))
        assert result == BitVec(1, "0")
        result = identity(BitVec(1, "1"))
        assert result == BitVec(1, "1")

    def test_identity_2bit(self):
        function = BoolFunction(identity_2bit)
        assert function._parameters_signature == [(type(BitVec(2)), 2)]
        for a in range(4):
            tmp = BitVec(2, a)
            result = identity(tmp)
            assert result == tmp

    def test_bool_not(self):
        function = BoolFunction(bool_not)
        assert function._parameters_signature == [(type(BitVec(1)), 1)]
        for a in range(2):
            tmp = BitVec(1, a)
            result = bool_not(tmp)
            assert result is not bool(tmp)

    def test_bit_not(self):
        function = BoolFunction(bit_not)
        assert function._parameters_signature == [(type(BitVec(1)), 1)]
        for a in range(2):
            tmp = BitVec(1, a)
            result = bit_not(tmp)
            assert result == ~tmp

    def test_bit_not_2bit(self):
        function = BoolFunction(bit_not_2bit)
        assert function._parameters_signature == [(type(BitVec(2)), 2)]
        for a in range(4):
            tmp = BitVec(2, a)
            result = bit_not_2bit(tmp)
            assert result == ~tmp

    def test_bool_and(self):
        function = BoolFunction(bool_and)
        assert function._parameters_signature == [
            (type(BitVec(1)), 1),
            (type(BitVec(1)), 1),
        ]
        for a in range(2):
            for b in range(2):
                result = bool_and(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) and BitVec(1, b)
                assert result == tmp

    def test_bit_and(self):
        function = BoolFunction(bit_and)
        assert function._parameters_signature == [
            (type(BitVec(1)), 1),
            (type(BitVec(1)), 1),
        ]
        for a in range(2):
            for b in range(2):
                result = bit_and(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) & BitVec(1, b)
                assert result == tmp

    def test_bit_and_2bit(self):
        function = BoolFunction(bit_and_2bit)
        assert function._parameters_signature == [
            (type(BitVec(2)), 2),
            (type(BitVec(2)), 2),
        ]
        for a in range(4):
            for b in range(4):
                result = bit_and_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) & BitVec(2, b)
                assert result == tmp

    def test_bool_or(self):
        function = BoolFunction(bool_or)
        assert function._parameters_signature == [
            (type(BitVec(1)), 1),
            (type(BitVec(1)), 1),
        ]
        for a in range(2):
            for b in range(2):
                result = bool_or(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) or BitVec(1, b)
                assert result == tmp

    def test_bit_or(self):
        function = BoolFunction(bit_or)
        assert function._parameters_signature == [
            (type(BitVec(1)), 1),
            (type(BitVec(1)), 1),
        ]
        for a in range(2):
            for b in range(2):
                result = bit_or(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) | BitVec(1, b)
                assert result == tmp

    def test_bit_or_2bit(self):
        function = BoolFunction(bit_or_2bit)
        assert function._parameters_signature == [
            (type(BitVec(2)), 2),
            (type(BitVec(2)), 2),
        ]
        for a in range(4):
            for b in range(4):
                result = bit_or_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) | BitVec(2, b)
                assert result == tmp

    def test_bit_xor(self):
        function = BoolFunction(bit_xor)
        assert function._parameters_signature == [
            (type(BitVec(1)), 1),
            (type(BitVec(1)), 1),
        ]
        for a in range(2):
            for b in range(2):
                result = bit_xor(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) ^ BitVec(1, b)
                assert result == tmp

    def test_bit_xor_2bit(self):
        function = BoolFunction(bit_xor_2bit)
        assert function._parameters_signature == [
            (type(BitVec(2)), 2),
            (type(BitVec(2)), 2),
        ]
        for a in range(4):
            for b in range(4):
                result = bit_xor_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) ^ BitVec(2, b)
                assert result == tmp

    def test_eq(self):
        function = BoolFunction(eq)
        assert function._parameters_signature == [
            (type(BitVec(1)), 1),
            (type(BitVec(1)), 1),
        ]
        for a in range(2):
            for b in range(2):
                result = eq(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) == BitVec(1, b)
                assert result == tmp

    def test_bit_eq_2bit(self):
        function = BoolFunction(eq_2bit)
        assert function._parameters_signature == [
            (type(BitVec(2)), 2),
            (type(BitVec(2)), 2),
        ]
        for a in range(4):
            for b in range(4):
                result = eq_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) == BitVec(2, b)
                assert result == tmp

    def test_ne(self):
        function = BoolFunction(ne)
        assert function._parameters_signature == [
            (type(BitVec(1)), 1),
            (type(BitVec(1)), 1),
        ]
        for a in range(2):
            for b in range(2):
                result = ne(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) != BitVec(1, b)
                assert result == tmp

    def test_bit_ne_2bit(self):
        function = BoolFunction(ne_2bit)
        assert function._parameters_signature == [
            (type(BitVec(2)), 2),
            (type(BitVec(2)), 2),
        ]
        for a in range(4):
            for b in range(4):
                result = ne_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) != BitVec(2, b)
                assert result == tmp


class TestBoolFunctionSimulation:
    def test_constant(self):
        function = BoolFunction(constant)
        result = function.simulate()
        assert result == BitVec(1, "1")

    def test_constant_2bit(self):
        function = BoolFunction(constant_2bit)
        result = function.simulate()
        assert result == BitVec("10")

    def test_constant_3bit(self):
        function = BoolFunction(constant_3bit)
        result = function.simulate()
        assert result == BitVec(3, 5)

    def test_constant_4bit(self):
        function = BoolFunction(constant_4bit)
        result = function.simulate()
        assert result == BitVec("0000")

    def test_identity(self):
        function = BoolFunction(identity)
        result = function.simulate(BitVec(1, "0"))
        assert result == BitVec(1, "0")
        result = function.simulate(BitVec(1, "1"))
        assert result == BitVec(1, "1")

    def test_identity_2bit(self):
        function = BoolFunction(identity_2bit)
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            assert result == tmp

    def test_identity_not(self):
        function = BoolFunction(identity_not)
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            assert result == (tmp, ~tmp)

    def test_bool_not(self):
        function = BoolFunction(bool_not)
        for a in range(2):
            tmp = BitVec(1, a)
            result = function.simulate(tmp)
            assert result == ~tmp

    def test_bit_not(self):
        function = BoolFunction(bit_not)
        for a in range(2):
            tmp = BitVec(1, a)
            result = function.simulate(tmp)
            assert result == ~tmp

    def test_bit_not_2bit(self):
        function = BoolFunction(bit_not_2bit)
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            assert result == ~tmp

    def test_bool_and(self):
        function = BoolFunction(bool_and)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) and BitVec(1, b)
                assert result == tmp

    def test_bit_and(self):
        function = BoolFunction(bit_and)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) & BitVec(1, b)
                assert result == tmp

    def test_bit_and_2bit(self):
        function = BoolFunction(bit_and_2bit)
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) & BitVec(2, b)
                assert result == tmp

    def test_bool_or(self):
        function = BoolFunction(bool_or)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) or BitVec(1, b)
                assert result == tmp

    def test_bit_or(self):
        function = BoolFunction(bit_or)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) | BitVec(1, b)
                assert result == tmp

    def test_bit_or_2bit(self):
        function = BoolFunction(bit_or_2bit)
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) | BitVec(2, b)
                assert result == tmp

    def test_bit_xor(self):
        function = BoolFunction(bit_xor)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) ^ BitVec(1, b)
                assert result == tmp

    def test_bit_xor_2bit(self):
        function = BoolFunction(bit_xor_2bit)
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) ^ BitVec(2, b)
                assert result == tmp

    def test_eq(self):
        function = BoolFunction(eq)
        assert function._parameters_signature == [
            (type(BitVec(1)), 1),
            (type(BitVec(1)), 1),
        ]
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) == BitVec(1, b)
                assert result == tmp

    def test_bit_eq_2bit(self):
        function = BoolFunction(eq_2bit)
        assert function._parameters_signature == [
            (type(BitVec(2)), 2),
            (type(BitVec(2)), 2),
        ]
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) == BitVec(2, b)
                assert result == tmp

    def test_ne(self):
        function = BoolFunction(ne)
        assert function._parameters_signature == [
            (type(BitVec(1)), 1),
            (type(BitVec(1)), 1),
        ]
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) != BitVec(1, b)
                assert result == tmp

    def test_bit_ne_2bit(self):
        function = BoolFunction(ne_2bit)
        assert function._parameters_signature == [
            (type(BitVec(2)), 2),
            (type(BitVec(2)), 2),
        ]
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) != BitVec(2, b)
                assert result == tmp


class TestBoolFunctionFullSimulation:
    def test_identity(self):
        function = BoolFunction(identity)
        function.simulate_all()
        assert function.num_inputs() == 1
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 1
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "10"

        result = function.simulate(BitVec(1, "0"))
        assert result == BitVec(1, "0")
        result = function.simulate(BitVec(1, "1"))
        assert result == BitVec(1, "1")

    def test_identity_2bit(self):
        function = BoolFunction(identity_2bit)
        function.simulate_all()
        assert function.num_inputs() == 1
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 2
        assert str(function.truth_table(output_bit=0)) == "1010"
        assert str(function.truth_table(output_bit=1)) == "1100"
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            assert result == tmp

    def test_not(self):
        function = BoolFunction(bool_not)
        function.simulate_all()
        assert function.num_inputs() == 1
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 1
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "01"
        for a in range(2):
            tmp = BitVec(1, a)
            result = function.simulate(tmp)
            assert result == ~tmp

    def test_not_2bit(self):
        function = BoolFunction(bit_not_2bit)
        function.simulate_all()
        assert function.num_inputs() == 1
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 2
        assert str(function.truth_table(output_bit=0)) == "0101"
        assert str(function.truth_table(output_bit=1)) == "0011"
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            assert result == ~tmp

    def test_and(self):
        function = BoolFunction(bool_and)
        function.simulate_all()
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "1000"
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) and BitVec(1, b)
                assert result == tmp

    def test_and_2bit(self):
        function = BoolFunction(bit_and_2bit)
        function.simulate_all()
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 4
        assert function.num_output_bits() == 2
        output0 = BitVec(16, 0xAAAA) & BitVec(16, 0xF0F0)
        output1 = BitVec(16, 0xCCCC) & BitVec(16, 0xFF00)
        assert str(function.truth_table(output_bit=0)) == str(output0)
        assert str(function.truth_table(output_bit=1)) == str(output1)
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) & BitVec(2, b)
                assert result == tmp

    def test_or(self):
        function = BoolFunction(bool_or)
        function.simulate_all()
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "1110"
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) or BitVec(1, b)
                assert result == tmp

    def test_or_2bit(self):
        function = BoolFunction(bit_or_2bit)
        function.simulate_all()
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 4
        assert function.num_output_bits() == 2
        output0 = BitVec(16, 0xAAAA) | BitVec(16, 0xF0F0)
        output1 = BitVec(16, 0xCCCC) | BitVec(16, 0xFF00)
        assert str(function.truth_table(output_bit=0)) == str(output0)
        assert str(function.truth_table(output_bit=1)) == str(output1)
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) | BitVec(2, b)
                assert result == tmp

    def test_xor_2bit(self):
        function = BoolFunction(bit_xor_2bit)
        function.simulate_all()
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 4
        assert function.num_output_bits() == 2
        output0 = BitVec(16, 0xAAAA) ^ BitVec(16, 0xF0F0)
        output1 = BitVec(16, 0xCCCC) ^ BitVec(16, 0xFF00)
        assert str(function.truth_table(output_bit=0)) == str(output0)
        assert str(function.truth_table(output_bit=1)) == str(output1)
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) ^ BitVec(2, b)
                assert result == tmp


class TestBoolFunctionExpressionConstructor:
    def test_identity(self):
        function = BoolFunction.from_expression("x")
        function.simulate_all()
        assert function.num_inputs() == 1
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 1
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "10"

    def test_not(self):
        function = BoolFunction.from_expression("~x")
        function.simulate_all()
        assert function.num_inputs() == 1
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 1
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "01"

    def test_and(self):
        function = BoolFunction.from_expression("x & b")
        function.simulate_all()
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "1000"

    def test_or(self):
        function = BoolFunction.from_expression("x | b")
        function.simulate_all()
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "1110"

    def test_xor(self):
        function = BoolFunction.from_expression("x ^ b")
        function.simulate_all()
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "0110"

    def test_de_morgan(self):
        function = BoolFunction.from_expression("~(~(x0 | x1) ^ (~x0 & ~x1))")
        function.simulate_all()
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 1
        assert str(function.truth_table(output_bit=0)) == "1111"


class TestBoolFunctionTTConstructor:
    def test_identity(self):
        function = BoolFunction.from_truth_table("10")
        assert function.num_inputs() == 1
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 1
        assert function.num_output_bits() == 1
        result = function.simulate(BitVec(1, "0"))
        assert result == BitVec(1, "0")
        result = function.simulate(BitVec(1, "1"))
        assert result == BitVec(1, "1")

    def test_identity_2bit(self):
        function = BoolFunction.from_truth_table(["1010", "1100"])
        assert function.num_inputs() == 2
        assert function.num_outputs() == 2
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 2
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp[0], tmp[1])
            assert result == (tmp[0], tmp[1])

    def test_not(self):
        function = BoolFunction.from_truth_table("01")
        assert function.num_inputs() == 1
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 1
        assert function.num_output_bits() == 1
        for a in range(2):
            tmp = BitVec(1, a)
            result = function.simulate(tmp)
            assert result == ~tmp

    def test_not_2bit(self):
        function = BoolFunction.from_truth_table(["0101", "0011"])
        assert function.num_inputs() == 2
        assert function.num_outputs() == 2
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 2
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp[0], tmp[1])
            assert result == (~tmp[0], ~tmp[1])

    def test_and(self):
        function = BoolFunction.from_truth_table("1000")
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 1
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) and BitVec(1, b)
                assert result == tmp

    def test_and_2bit(self):
        function = BoolFunction.from_truth_table(
            ["1010000010100000", "1100110000000000"]
        )
        assert function.num_inputs() == 4
        assert function.num_outputs() == 2
        assert function.num_input_bits() == 4
        assert function.num_output_bits() == 2
        for a in range(4):
            for b in range(4):
                a_bv = BitVec(2, a)
                b_bv = BitVec(2, b)
                result = function.simulate(a_bv[0], a_bv[1], b_bv[0], b_bv[1])
                tmp = a_bv & b_bv
                assert result == (tmp[0], tmp[1])

    def test_or(self):
        function = BoolFunction(bool_and)
        function = BoolFunction.from_truth_table("1110")
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 1
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) or BitVec(1, b)
                assert result == tmp

    def test_or_2bit(self):
        function = BoolFunction.from_truth_table(
            ["1111101011111010", "1111111111001100"]
        )
        assert function.num_inputs() == 4
        assert function.num_outputs() == 2
        assert function.num_input_bits() == 4
        assert function.num_output_bits() == 2
        for a in range(4):
            for b in range(4):
                a_bv = BitVec(2, a)
                b_bv = BitVec(2, b)
                result = function.simulate(a_bv[0], a_bv[1], b_bv[0], b_bv[1])
                tmp = a_bv | b_bv
                assert result == (tmp[0], tmp[1])

    def test_xor(self):
        function = BoolFunction(bool_and)
        function = BoolFunction.from_truth_table("0110")
        assert function.num_inputs() == 2
        assert function.num_outputs() == 1
        assert function.num_input_bits() == 2
        assert function.num_output_bits() == 1
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) ^ BitVec(1, b)
                assert result == tmp

    def test_xor_2bit(self):
        function = BoolFunction.from_truth_table(
            ["0101101001011010", "0011001111001100"]
        )
        assert function.num_inputs() == 4
        assert function.num_outputs() == 2
        assert function.num_input_bits() == 4
        assert function.num_output_bits() == 2
        for a in range(4):
            for b in range(4):
                a_bv = BitVec(2, a)
                b_bv = BitVec(2, b)
                result = function.simulate(a_bv[0], a_bv[1], b_bv[0], b_bv[1])
                tmp = a_bv ^ b_bv
                assert result == (tmp[0], tmp[1])
