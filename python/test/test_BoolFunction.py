#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import unittest

from tweedledum.BoolFunctionCompiler import BitVec, BoolFunction

def id(a: BitVec(1)) -> BitVec(1):
    return a

class TestBoolFunction(unittest.TestCase):
        def test_id(self):
            function = BoolFunction(id)
            self.assertEqual(function.signature_, [['BitVec', 1]])

class TestBoolFunctionSimulation(unittest.TestCase):
        def test_id(self):
            function = BoolFunction(id)
            truth_table = function.simulate('0')
            # Simulate full truth table
            truth_table = function.simulate()
            self.assertEqual(len(truth_table), 1)
            self.assertEqual(str(truth_table[0]), '10')

if __name__ == '__main__':
    unittest.main()
