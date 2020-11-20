#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import unittest

from tweedledum.CFuncCompiler import BitVec, Parser, classical_function

def id(a: BitVec(1)) -> BitVec(1):
    return a

class TestParser(unittest.TestCase):
        def test_id(self):
            function = classical_function(id)
            # truth_table = network.simulate()
            # self.assertEqual(network.types, [{'Bit': 'type', 'a': 'Bit', 'return': 'Bit'}])
            # self.assertEqual(truth_table, '10')

if __name__ == '__main__':
    unittest.main()
