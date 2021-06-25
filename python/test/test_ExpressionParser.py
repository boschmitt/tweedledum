# ------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# ------------------------------------------------------------------------------
import pytest

from tweedledum.bool_function_compiler.function_parser import ParseError
from tweedledum.bool_function_compiler.expression_parser import ExpressionParser


def _get_node(parser, symbol):
    _, signals_ = parser._symbol_table[symbol]
    return parser._logic_network.get_node(signals_[0])


def test_ordered():
    parser = ExpressionParser(
        "((A & C) | (B & D)) & ~(C & D)", var_order=["A", "B", "C", "D"]
    )
    assert _get_node(parser, "A") == parser._logic_network.pi_at(0)
    assert _get_node(parser, "B") == parser._logic_network.pi_at(1)
    assert _get_node(parser, "C") == parser._logic_network.pi_at(2)
    assert _get_node(parser, "D") == parser._logic_network.pi_at(3)


def test_fail_ordered():
    with pytest.raises(ParseError, match=r".*\{'C'\}*"):
        ExpressionParser("((A & C) | (B & D)) & ~(C & D)", var_order=["A", "B", "D"])


def test_unordered():
    parser = ExpressionParser("((A & C) | (B & D)) & ~(C & D)")
    assert _get_node(parser, "A") == parser._logic_network.pi_at(0)
    assert _get_node(parser, "B") == parser._logic_network.pi_at(2)
    assert _get_node(parser, "C") == parser._logic_network.pi_at(1)
    assert _get_node(parser, "D") == parser._logic_network.pi_at(3)
