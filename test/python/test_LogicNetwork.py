#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import unittest

from tweedledum import Classical as Classical

class TestLogicNetwork(unittest.TestCase):
    def test_constant(self):
        logic_network_ = Classical.LogicNetwork()
        constant = logic_network_.get_constant(True)
        logic_network_.create_po(constant)
