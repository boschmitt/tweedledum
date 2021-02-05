#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
from .BoolFunction import BoolFunction
from .expression_parser import ExpressionParser


class BoolExpression(BoolFunction):
    """Class to represent a Boolean Expression"""

    def __init__(self, expression):
        if not isinstance(expression, str):
            raise TypeError("[BoolExpression] Constructor requires a string")
        parsed_expression = ExpressionParser(expression)
        self._parameters_signature = parsed_expression._parameters_signature
        self._return_signature = parsed_expression._return_signature
        self._symbol_table = parsed_expression._symbol_table
        self._logic_network = parsed_expression._logic_network
        self._truth_table = None
