# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
import unittest

from tweedledum.bool_function_compiler import BitVec, BoolExpression


class TestBoolExpression(unittest.TestCase):
    def test_id(self):
        expression = 'x'
        function = BoolExpression(expression)
        self.assertEqual(function._parameters_signature,
                         [(type(BitVec(1)), 1)])
        result = eval(expression, {'x': BitVec(1, '0')})
        self.assertEqual(result, BitVec(1, '0'))


class TestBoolExpressionFullSimulation(unittest.TestCase):
    ''' Simulate full truth table. '''

    def test_id_str(self):
        function = BoolExpression("x")
        truth_table = function.simulate_all()
        self.assertEqual(len(truth_table), 1)
        self.assertEqual(str(truth_table[0]), '10')

    def test_not_str(self):
        function = BoolExpression("~x")
        truth_table = function.simulate_all()
        self.assertEqual(len(truth_table), 1)
        self.assertEqual(str(truth_table[0]), '01')

    def test_and_str(self):
        function = BoolExpression("x & b")
        truth_table = function.simulate_all()
        self.assertEqual(len(truth_table), 1)
        self.assertEqual(str(truth_table[0]), '1000')

    def test_or_str(self):
        function = BoolExpression("x | b")
        truth_table = function.simulate_all()
        self.assertEqual(len(truth_table), 1)
        self.assertEqual(str(truth_table[0]), '1110')

    def test_xor_str(self):
        function = BoolExpression("x ^ b")
        truth_table = function.simulate_all()
        self.assertEqual(len(truth_table), 1)
        self.assertEqual(str(truth_table[0]), '0110')
