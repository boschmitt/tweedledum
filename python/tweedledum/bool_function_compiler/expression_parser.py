# -------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
# -------------------------------------------------------------------------------
import ast

from .bitvec import BitVec
from .function_parser import FunctionParser, ParseError


class ExpressionParser(FunctionParser):
    def __init__(self, source, var_order: list = None):
        self.var_order = var_order
        super().__init__(source)

    def visit_Module(self, node):
        """The full snippet should contain a single function"""
        if len(node.body) != 1 or not isinstance(node.body[0], ast.Expr):
            raise ParseError("Not an expression to parse")
        if self.var_order:
            self._create_ordered_vars(node)

        self._return_signature = [(self.types["BitVec"], 1)]
        expr_type, expr_signal = self.visit(node.body[0])
        if expr_type != self._return_signature[0]:
            raise ParseError("Expressions can only evaluate to a BitVec(1)")
        self._logic_network.create_po(expr_signal[0])

    def visit_Expr(self, node):
        return super().visit(node.value)

    def visit_Name(self, node):
        return self._create_name(node.id)

    def _create_name(self, name: str):
        if name not in self._symbol_table:
            self._parameters_signature.append((self.types["BitVec"], 1))
            self._symbol_table[name] = (
                (self.types["BitVec"], 1),
                [self._logic_network.create_pi()],
            )
        return self._symbol_table[name]

    def _create_ordered_vars(self, module):
        vars = set()
        for node in ast.walk(module):
            if isinstance(node, ast.Name):
                vars.add(node.id)
        if vars != set(self.var_order):
            raise ParseError(
                f"Missing variables in order list {vars - set(self.var_order)}"
            )
        for var in self.var_order:
            self._create_name(var)
