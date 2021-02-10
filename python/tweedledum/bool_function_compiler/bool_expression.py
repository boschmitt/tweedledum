#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
from .bitvec import BitVec
from .bool_function import BoolFunction
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

    @classmethod
    def from_dimacs_file(cls, filename: str, name=None):
        """Create a BooleanExpression from the string in the DIMACS format.
        Args:
            dimacs: The string in the DIMACS format.

        Returns:
            A quantum circuit (BooleanExpression) for the input string
        """
        from tweedledum.classical import read_dimacs
        bool_exp_instance = cls.__new__(cls)
        bool_exp_instance._logic_network = read_dimacs(filename)
        bool_exp_instance._parameters_signature = []
        for i in range(bool_exp_instance._logic_network.num_pis()):
            bool_exp_instance._parameters_signature.append((type(BitVec(1)), 1))
        bool_exp_instance._return_signature = [(type(BitVec(1)), 1)]
        return bool_exp_instance