#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import ast

from .bitvec import BitVec
from .function_parser import FunctionParser, ParseError

class ExpressionParser(FunctionParser):

    def visit_Module(self, node):
        """The full snippet should contain a single function"""
        if len(node.body) != 1 or not isinstance(node.body[0], ast.Expr):
            raise ParseError("Not an expression to parse")

        self._return_signature = [(self.types['BitVec'], 1)]
        expr_type, expr_signal = self.visit(node.body[0])
        if expr_type != self._return_signature[0]:
            raise ParseError("Expressions can only evaluate to a BitVec(1)")
        self._logic_network.create_po(expr_signal[0])

    def visit_Expr(self, node):
        return super().visit(node.value)

    def visit_Name(self, node):
        if node.id not in self._symbol_table:
            self._parameters_signature.append((self.types['BitVec'], 1))
            self._symbol_table[node.id] = ((self.types['BitVec'], 1),
                                           [self._logic_network.create_pi()])
        return self._symbol_table[node.id]
