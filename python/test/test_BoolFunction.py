#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import unittest

from tweedledum.bool_function_compiler import BitVec, BoolFunction
from python.test import examples

class TestBoolFunction(unittest.TestCase):
    def test_constant_3bit(self):
        function = BoolFunction(examples.constant_3bit)
        self.assertEqual(function._parameters_signature, [])
        result = examples.constant_3bit()
        self.assertEqual(result, BitVec(3, '101'))

    def test_identity(self):
        function = BoolFunction(examples.identity)
        self.assertEqual(function._parameters_signature, [(type(BitVec(1)), 1)])
        result = examples.identity(BitVec(1, '0'))
        self.assertEqual(result, BitVec(1, '0'))
        result = examples.identity(BitVec(1, '1'))
        self.assertEqual(result, BitVec(1, '1'))

    def test_identity_2bit(self):
        function = BoolFunction(examples.identity_2bit)
        self.assertEqual(function._parameters_signature, [(type(BitVec(2)), 2)])
        for a in range(4):
            tmp = BitVec(2, a)
            result = examples.identity(tmp)
            self.assertEqual(result, tmp)

    def test_bool_not(self):
        function = BoolFunction(examples.bool_not)
        self.assertEqual(function._parameters_signature, [(type(BitVec(1)), 1)])
        for a in range(2):
            tmp = BitVec(1, a)
            result = examples.bool_not(tmp)
            self.assertEqual(result, not bool(tmp))

    def test_bit_not(self):
        function = BoolFunction(examples.bit_not)
        self.assertEqual(function._parameters_signature, [(type(BitVec(1)), 1)])
        for a in range(2):
            tmp = BitVec(1, a)
            result = examples.bit_not(tmp)
            self.assertEqual(result, ~tmp)

    def test_bit_not_2bit(self):
        function = BoolFunction(examples.bit_not_2bit)
        self.assertEqual(function._parameters_signature, [(type(BitVec(2)), 2)])
        for a in range(4):
            tmp = BitVec(2, a)
            result = examples.bit_not_2bit(tmp)
            self.assertEqual(result, ~tmp)

    def test_bool_and(self):
        function = BoolFunction(examples.bool_and)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1), (type(BitVec(1)), 1)])
        for a in range(2):
            for b in range(2):
                result = examples.bool_and(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) and BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_and(self):
        function = BoolFunction(examples.bit_and)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1), (type(BitVec(1)), 1)])
        for a in range(2):
            for b in range(2):
                result = examples.bit_and(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) & BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_and_2bit(self):
        function = BoolFunction(examples.bit_and_2bit)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(2)), 2), (type(BitVec(2)), 2)])
        for a in range(4):
            for b in range(4):
                result = examples.bit_and_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) & BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_bool_or(self):
        function = BoolFunction(examples.bool_or)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1), (type(BitVec(1)), 1)])
        for a in range(2):
            for b in range(2):
                result = examples.bool_or(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) or BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_or(self):
        function = BoolFunction(examples.bit_or)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1), (type(BitVec(1)), 1)])
        for a in range(2):
            for b in range(2):
                result = examples.bit_or(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) | BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_or_2bit(self):
        function = BoolFunction(examples.bit_or_2bit)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(2)), 2), (type(BitVec(2)), 2)])
        for a in range(4):
            for b in range(4):
                result = examples.bit_or_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) | BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_bit_xor(self):
        function = BoolFunction(examples.bit_xor)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1), (type(BitVec(1)), 1)])
        for a in range(2):
            for b in range(2):
                result = examples.bit_xor(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) ^ BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_xor_2bit(self):
        function = BoolFunction(examples.bit_xor_2bit)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(2)), 2), (type(BitVec(2)), 2)])
        for a in range(4):
            for b in range(4):
                result = examples.bit_xor_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) ^ BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_eq(self):
        function = BoolFunction(examples.eq)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1), (type(BitVec(1)), 1)])
        for a in range(2):
            for b in range(2):
                result = examples.eq(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) == BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_eq_2bit(self):
        function = BoolFunction(examples.eq_2bit)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(2)), 2), (type(BitVec(2)), 2)])
        for a in range(4):
            for b in range(4):
                result = examples.eq_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) == BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_ne(self):
        function = BoolFunction(examples.ne)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1), (type(BitVec(1)), 1)])
        for a in range(2):
            for b in range(2):
                result = examples.ne(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) != BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_ne_2bit(self):
        function = BoolFunction(examples.ne_2bit)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(2)), 2), (type(BitVec(2)), 2)])
        for a in range(4):
            for b in range(4):
                result = examples.ne_2bit(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) != BitVec(2, b)
                self.assertEqual(result, tmp)

class TestBoolFunctionSimulation(unittest.TestCase):
    def test_constant(self):
        function = BoolFunction(examples.constant)
        result = function.simulate()
        self.assertEqual(result, BitVec(1, '1'))

    def test_constant_2bit(self):
        function = BoolFunction(examples.constant_2bit)
        result = function.simulate()
        self.assertEqual(result, BitVec('10'))

    def test_constant_3bit(self):
        function = BoolFunction(examples.constant_3bit)
        result = function.simulate()
        self.assertEqual(result, BitVec(3, 5))

    def test_constant_4bit(self):
        function = BoolFunction(examples.constant_4bit)
        result = function.simulate()
        self.assertEqual(result, BitVec('0000'))

    def test_identity(self):
        function = BoolFunction(examples.identity)
        result = function.simulate(BitVec(1, '0'))
        self.assertEqual(result, BitVec(1, '0'))
        result = function.simulate(BitVec(1, '1'))
        self.assertEqual(result, BitVec(1, '1'))

    def test_identity_2bit(self):
        function = BoolFunction(examples.identity_2bit)
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            self.assertEqual(result, tmp)
    
    def test_identity_not(self):
        function = BoolFunction(examples.identity_not)
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            self.assertEqual(result, (tmp, ~tmp))

    def test_bool_not(self):
        function = BoolFunction(examples.bool_not)
        for a in range(2):
            tmp = BitVec(1, a)
            result = function.simulate(tmp)
            self.assertEqual(result, ~tmp)

    def test_bit_not(self):
        function = BoolFunction(examples.bit_not)
        for a in range(2):
            tmp = BitVec(1, a)
            result = function.simulate(tmp)
            self.assertEqual(result, ~tmp)

    def test_bit_not_2bit(self):
        function = BoolFunction(examples.bit_not_2bit)
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            self.assertEqual(result, ~tmp)

    def test_bool_and(self):
        function = BoolFunction(examples.bool_and)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) and BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_and(self):
        function = BoolFunction(examples.bit_and)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) & BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_and_2bit(self):
        function = BoolFunction(examples.bit_and_2bit)
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) & BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_bool_or(self):
        function = BoolFunction(examples.bool_or)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) or BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_or(self):
        function = BoolFunction(examples.bit_or)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) | BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_or_2bit(self):
        function = BoolFunction(examples.bit_or_2bit)
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) | BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_bit_xor(self):
        function = BoolFunction(examples.bit_xor)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) ^ BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_xor_2bit(self):
        function = BoolFunction(examples.bit_xor_2bit)
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) ^ BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_eq(self):
        function = BoolFunction(examples.eq)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1), (type(BitVec(1)), 1)])
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) == BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_eq_2bit(self):
        function = BoolFunction(examples.eq_2bit)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(2)), 2), (type(BitVec(2)), 2)])
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) == BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_ne(self):
        function = BoolFunction(examples.ne)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1), (type(BitVec(1)), 1)])
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) != BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_bit_ne_2bit(self):
        function = BoolFunction(examples.ne_2bit)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(2)), 2), (type(BitVec(2)), 2)])
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) != BitVec(2, b)
                self.assertEqual(result, tmp)

class TestBoolFunctionFullSimulation(unittest.TestCase):
    def test_identity(self):
        function = BoolFunction(examples.identity)
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 1)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 1)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '10')
        
        result = function.simulate(BitVec(1, '0'))
        self.assertEqual(result, BitVec(1, '0'))
        result = function.simulate(BitVec(1, '1'))
        self.assertEqual(result, BitVec(1, '1'))

    def test_identity_2bit(self):
        function = BoolFunction(examples.identity_2bit)
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 1)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 2)
        self.assertEqual(str(function.truth_table(output_bit=0)), '1010')
        self.assertEqual(str(function.truth_table(output_bit=1)), '1100')
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            self.assertEqual(result, tmp)

    def test_not(self):
        function = BoolFunction(examples.bool_not)
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 1)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 1)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '01')
        for a in range(2):
            tmp = BitVec(1, a)
            result = function.simulate(tmp)
            self.assertEqual(result, ~tmp)

    def test_not_2bit(self):
        function = BoolFunction(examples.bit_not_2bit)
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 1)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 2)
        self.assertEqual(str(function.truth_table(output_bit=0)), '0101')
        self.assertEqual(str(function.truth_table(output_bit=1)), '0011')
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp)
            self.assertEqual(result, ~tmp)

    def test_and(self):
        function = BoolFunction(examples.bool_and)
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '1000')
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) and BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_and_2bit(self):
        function = BoolFunction(examples.bit_and_2bit)
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 4)
        self.assertEqual(function.num_output_bits(), 2)
        output0 = BitVec(16, 0xaaaa) & BitVec(16, 0xf0f0)
        output1 = BitVec(16, 0xcccc) & BitVec(16, 0xff00)
        self.assertEqual(str(function.truth_table(output_bit=0)), str(output0))
        self.assertEqual(str(function.truth_table(output_bit=1)), str(output1))
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) & BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_or(self):
        function = BoolFunction(examples.bool_or)
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '1110')
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) or BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_or_2bit(self):
        function = BoolFunction(examples.bit_or_2bit)
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 4)
        self.assertEqual(function.num_output_bits(), 2)
        output0 = BitVec(16, 0xaaaa) | BitVec(16, 0xf0f0)
        output1 = BitVec(16, 0xcccc) | BitVec(16, 0xff00)
        self.assertEqual(str(function.truth_table(output_bit=0)), str(output0))
        self.assertEqual(str(function.truth_table(output_bit=1)), str(output1))
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) | BitVec(2, b)
                self.assertEqual(result, tmp)

    def test_xor_2bit(self):
        function = BoolFunction(examples.bit_xor_2bit)
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 4)
        self.assertEqual(function.num_output_bits(), 2)
        output0 = BitVec(16, 0xaaaa) ^ BitVec(16, 0xf0f0)
        output1 = BitVec(16, 0xcccc) ^ BitVec(16, 0xff00)
        self.assertEqual(str(function.truth_table(output_bit=0)), str(output0))
        self.assertEqual(str(function.truth_table(output_bit=1)), str(output1))
        for a in range(4):
            for b in range(4):
                result = function.simulate(BitVec(2, a), BitVec(2, b))
                tmp = BitVec(2, a) ^ BitVec(2, b)
                self.assertEqual(result, tmp)

class TestBoolFunctionExpressionConstructor(unittest.TestCase):
    def test_identity(self):
        function = BoolFunction.from_expression("x")
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 1)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 1)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '10')

    def test_not(self):
        function = BoolFunction.from_expression("~x")
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 1)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 1)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '01')

    def test_and(self):
        function = BoolFunction.from_expression("x & b")
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '1000')

    def test_or(self):
        function = BoolFunction.from_expression("x | b")
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '1110')

    def test_xor(self):
        function = BoolFunction.from_expression("x ^ b")
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '0110')

    def test_de_morgan(self):
        function = BoolFunction.from_expression("~(~(x0 | x1) ^ (~x0 & ~x1))")
        function.simulate_all()
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 1)
        self.assertEqual(str(function.truth_table(output_bit=0)), '1111')

class TestBoolFunctionTTConstructor(unittest.TestCase):
    def test_identity(self):
        function = BoolFunction.from_truth_table('10')
        self.assertEqual(function.num_inputs(), 1)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 1)
        self.assertEqual(function.num_output_bits(), 1)
        result = function.simulate(BitVec(1, '0'))
        self.assertEqual(result, BitVec(1, '0'))
        result = function.simulate(BitVec(1, '1'))
        self.assertEqual(result, BitVec(1, '1'))

    def test_identity_2bit(self):
        function = BoolFunction.from_truth_table(['1010', '1100'])
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 2)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 2)
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp[0], tmp[1])
            self.assertEqual(result, (tmp[0], tmp[1]))

    def test_not(self):
        function = BoolFunction.from_truth_table('01')
        self.assertEqual(function.num_inputs(), 1)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 1)
        self.assertEqual(function.num_output_bits(), 1)
        for a in range(2):
            tmp = BitVec(1, a)
            result = function.simulate(tmp)
            self.assertEqual(result, ~tmp)

    def test_not_2bit(self):
        function = BoolFunction.from_truth_table(['0101', '0011'])
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 2)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 2)
        for a in range(4):
            tmp = BitVec(2, a)
            result = function.simulate(tmp[0], tmp[1])
            self.assertEqual(result, (~tmp[0], ~tmp[1]))

    def test_and(self):
        function = BoolFunction.from_truth_table('1000')
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 1)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) and BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_and_2bit(self):
        function = BoolFunction.from_truth_table(['1010000010100000', 
                                                  '1100110000000000'])
        self.assertEqual(function.num_inputs(), 4)
        self.assertEqual(function.num_outputs(), 2)
        self.assertEqual(function.num_input_bits(), 4)
        self.assertEqual(function.num_output_bits(), 2)
        for a in range(4):
            for b in range(4):
                a_bv = BitVec(2, a)
                b_bv = BitVec(2, b)
                result = function.simulate(a_bv[0], a_bv[1], b_bv[0], b_bv[1])
                tmp = a_bv & b_bv
                self.assertEqual(result, (tmp[0], tmp[1]))

    def test_or(self):
        function = BoolFunction(examples.bool_and)
        function = BoolFunction.from_truth_table('1110')
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 1)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) or BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_or_2bit(self):
        function = BoolFunction.from_truth_table(['1111101011111010', 
                                                  '1111111111001100'])
        self.assertEqual(function.num_inputs(), 4)
        self.assertEqual(function.num_outputs(), 2)
        self.assertEqual(function.num_input_bits(), 4)
        self.assertEqual(function.num_output_bits(), 2)
        for a in range(4):
            for b in range(4):
                a_bv = BitVec(2, a)
                b_bv = BitVec(2, b)
                result = function.simulate(a_bv[0], a_bv[1], b_bv[0], b_bv[1])
                tmp = a_bv | b_bv
                self.assertEqual(result, (tmp[0], tmp[1]))

    def test_xor(self):
        function = BoolFunction(examples.bool_and)
        function = BoolFunction.from_truth_table('0110')
        self.assertEqual(function.num_inputs(), 2)
        self.assertEqual(function.num_outputs(), 1)
        self.assertEqual(function.num_input_bits(), 2)
        self.assertEqual(function.num_output_bits(), 1)
        for a in range(2):
            for b in range(2):
                result = function.simulate(BitVec(1, a), BitVec(1, b))
                tmp = BitVec(1, a) ^ BitVec(1, b)
                self.assertEqual(result, tmp)

    def test_xor_2bit(self):
        function = BoolFunction.from_truth_table(['0101101001011010', 
                                                  '0011001111001100'])
        self.assertEqual(function.num_inputs(), 4)
        self.assertEqual(function.num_outputs(), 2)
        self.assertEqual(function.num_input_bits(), 4)
        self.assertEqual(function.num_output_bits(), 2)
        for a in range(4):
            for b in range(4):
                a_bv = BitVec(2, a)
                b_bv = BitVec(2, b)
                result = function.simulate(a_bv[0], a_bv[1], b_bv[0], b_bv[1])
                tmp = a_bv ^ b_bv
                self.assertEqual(result, (tmp[0], tmp[1]))
