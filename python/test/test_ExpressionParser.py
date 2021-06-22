# ------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# ------------------------------------------------------------------------------
import unittest

from tweedledum.bool_function_compiler.expression_parser import ExpressionParser


class TestExpressionParser(unittest.TestCase):
    def _get_node(self, parser, symbol):
        _, signals_ = parser._symbol_table[symbol]
        return parser._logic_network.get_node(signals_[0])

    def test_ordered(self):
        parser = ExpressionParser(
            "((A & C) | (B & D)) & ~(C & D)", var_order=["A", "B", "C", "D"]
        )
        # Remember that index 0 is the constant!
        self.assertEqual(self._get_node(parser, "A"), parser._logic_network.pi_at(0))
        self.assertEqual(self._get_node(parser, "B"), parser._logic_network.pi_at(1))
        self.assertEqual(self._get_node(parser, "C"), parser._logic_network.pi_at(2))
        self.assertEqual(self._get_node(parser, "D"), parser._logic_network.pi_at(3))

    def test_unordered(self):
        parser = ExpressionParser("((A & C) | (B & D)) & ~(C & D)")
        # Remember that index 0 is the constant!
        self.assertEqual(self._get_node(parser, "A"), parser._logic_network.pi_at(0))
        self.assertEqual(self._get_node(parser, "B"), parser._logic_network.pi_at(2))
        self.assertEqual(self._get_node(parser, "C"), parser._logic_network.pi_at(1))
        self.assertEqual(self._get_node(parser, "D"), parser._logic_network.pi_at(3))
