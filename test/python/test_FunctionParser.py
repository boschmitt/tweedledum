#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import unittest

from tweedledum.bool_function_compiler import BitVec, FunctionParser, BoolFunction

def id(a: BitVec(1)) -> BitVec(1):
    return a

class TestParser(unittest.TestCase):
        def test_id(self):
            pass
