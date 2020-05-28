#-------------------------------------------------------------------------------
# Part of tweedledum.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import unittest
import sys
from tweedledum import core, parser
from . import examples

class TestTypeAndSimulation(unittest.TestCase):
    def test_id(self):
        network = parser.func_to_ln(examples.id)
        truth_table = network.simulate()
        self.assertEqual(network.types, [{'Bit': 'type', 'a': 'Bit', 'return': 'Bit'}])
        self.assertEqual(truth_table, '10')

    def test_bit_not(self):
        network = parser.func_to_ln(examples.bit_not)
        truth_table = network.simulate()
        self.assertEqual(network.types, [{'Bit': 'type', 'a': 'Bit', 'return': 'Bit'}])
        self.assertEqual(truth_table, '01')

    def test_bool_not(self):
        network = parser.func_to_ln(examples.bool_not)
        truth_table = network.simulate()
        self.assertEqual(network.types, [{'Bit': 'type', 'a': 'Bit', 'return': 'Bit'}])
        self.assertEqual(truth_table, '01')

    def test_id_assign(self):
        network = parser.func_to_ln(examples.id_assing)
        truth_table = network.simulate()
        self.assertEqual(network.types, [{'Bit': 'type', 'a': 'Bit', 'b': 'Bit', 'return': 'Bit'}])
        self.assertEqual(truth_table, '10')

    def test_bit_and(self):
        network = parser.func_to_ln(examples.bit_and)
        truth_table = network.simulate()
        self.assertEqual(network.types, [{'Bit': 'type', 'a': 'Bit', 'b': 'Bit', 'return': 'Bit'}])
        self.assertEqual(truth_table, '1000')

    def test_bit_or(self):
        network = parser.func_to_ln(examples.bit_or)
        truth_table = network.simulate()
        self.assertEqual(network.types, [{'Bit': 'type', 'a': 'Bit', 'b': 'Bit', 'return': 'Bit'}])
        self.assertEqual(truth_table, '1110')

    def test_bool_or(self):
        network = parser.func_to_ln(examples.bool_or)
        truth_table = network.simulate()
        self.assertEqual(network.types, [{'Bit': 'type', 'a': 'Bit', 'b': 'Bit', 'return': 'Bit'}])
        self.assertEqual(truth_table, '1110')

    def test_multiple_binop(self):
        network = parser.func_to_ln(examples.multiple_binop)
        truth_table = network.simulate()
        self.assertEqual(network.types, [{'Bit': 'type', 'a': 'Bit', 'b': 'Bit', 'return': 'Bit'}])
        self.assertEqual(truth_table, '1000')

    def test_grover_oracle(self):
        network = parser.func_to_ln(examples.grover_oracle)
        truth_table = network.simulate()
        self.assertEqual(network.types, [{'return': 'Bit', 'Bit': 'type', 'a': 'Bit', 'b': 'Bit',
                                          'c': 'Bit', 'd': 'Bit'}])
        self.assertEqual(truth_table, '0100010001000100')

    def test_example1(self):
        network = parser.func_to_ln(examples.example1)
        truth_table = network.simulate()
        self.assertEqual(network.types, [
            {'Bit': 'type', 'a': 'Bit', 'b': 'Bit', 'c': 'Bit', 'd': 'Bit', 'return': 'Bit'}])
        self.assertEqual(truth_table, '1110')


class TestFail(unittest.TestCase):
    def assertExceptionMessage(self, context, message):
        self.assertTrue(message in context.exception.args[0])

    def test_id_bad_return(self):
        with self.assertRaises(parser.ParseError) as context:
            parser.func_to_ln(examples.id_bad_return)
        self.assertExceptionMessage(context, 'return type error')

    def test_id_no_type_arg(self):
        with self.assertRaises(parser.ParseError) as context:
            parser.func_to_ln(examples.id_no_type_arg)
        self.assertExceptionMessage(context, 'argument type is needed')

    def test_id_no_type_return(self):
        with self.assertRaises(parser.ParseError) as context:
            parser.func_to_ln(examples.id_no_type_return)
        self.assertExceptionMessage(context, 'return type is needed')

    def test_out_of_scope(self):
        with self.assertRaises(parser.ParseError) as context:
            parser.func_to_ln(examples.out_of_scope)
        self.assertExceptionMessage(context, 'out of scope: c')

if __name__ == '__main__':
    unittest.main()
