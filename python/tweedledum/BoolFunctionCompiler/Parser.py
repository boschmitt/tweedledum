#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import _ast
import ast

from .BitVec import BitVec
from ..libPyTweedledum.Classical import LogicNetwork

class ParseError(Exception):
    pass

class Parser(ast.NodeVisitor):
    bit_ops = {
        _ast.BitAnd: 'create_and',
        _ast.BitOr: 'create_or',
        _ast.BitXor: 'create_xor',
    }

    bool_ops = {
        _ast.And: 'create_and',
        _ast.Or: 'create_or',
    }

    # This feels quite hack-y
    types = {
        'BitVec' : type(BitVec(1))
    }

    def __init__(self, source):
        self.symbol_table_ = []
        self.signature_ = []
        self.logic_network_ = LogicNetwork()

        node = ast.parse(source)
        self.visit(node)
        super().__init__()

    def visit_args(self, node):
        cache = list()
        for arg in node.args:
            cache.append(arg.arg)
            if arg.annotation is None:
                continue
            if not isinstance(arg.annotation, _ast.Call):
                raise ParseError("BitVec type with size is needed")
            for var in cache:
                pis = list()
                size = int(arg.annotation.args[0].value)
                for i in range(size):
                    pis.append(self.logic_network_.create_pi("{}_{}".format(var, i)))
                self.symbol_table_[-1][var] = (arg.annotation.func.id, pis)
                self.signature_.append([self.types[arg.annotation.func.id], size])
            cache.clear()
        if len(cache) != 0:
             raise ParseError("Argument type is needed for %s" % cache)

    def visit_Assign(self, node):
        """When assign, the scope needs to be updated with the right type"""
        value_type, value_signals = self.visit(node.value)
        for target in node.targets:
            self.symbol_table_[-1][target.id] = [value_type, value_signals]
        return [value_type, value_signals]

    def visit_BinOp(self, node):
        """Handles ``&``, ``^``, and ``|``."""
        op = Parser.bit_ops.get(type(node.op))
        if not op:
            raise ParseError("Unknown binop.op %s" % op)
        left_type, left_signals = self.visit(node.left)
        right_type, right_signals = self.visit(node.right)
        if len(left_signals) != len(right_signals):
            raise ParseError("Different length binop.op %s" % op)
        result = list()
        for l, r in zip(left_signals, right_signals):
            result.append(getattr(self.logic_network_, op)(l, r))
        return right_type, result

    def visit_BoolOp(self, node):
        result_type, result_signal = self.visit(node.values[0])
        op = Parser.bool_ops.get(type(node.op))
        for value in node.values[1:]:
            value_type, value_signal = self.visit(value)
            result_signal[0] = getattr(self.logic_network_, op)(result_signal[0], value_signal[0])
        return 'BitVec', result_signal

    def visit_Call(self, node):
        """The return type should match the return type hint."""
        if node.func.id == 'BitVec':
            if len(node.args) == 1:
                return self.visit(node.args[0])
            elif len(node.args) == 2:
                return self.visit(node.args[1])
            else:
                raise ParseError("Invalid number of arguments")

    def visit_Compare(self, node):
        left_type, left_signals = self.visit(node.left)
        partial_results = list()
        for op, right in zip(node.ops, node.comparators):
            right_type, right_signals = self.visit(right)
            if len(left_signals) != len(right_signals):
                raise ParseError("Different length")
            if isinstance(op, ast.Eq):
                new_signals = [self.logic_network_.create_xnor(i, j) for i, j in zip(left_signals, right_signals)]
                partial_results.append(self.logic_network_.create_nary_and(new_signals))
            elif isinstance(op, ast.NotEq):
                new_signals = [self.logic_network_.create_xor(i, j) for i, j in zip(left_signals, right_signals)]
                partial_results.append(self.logic_network_.create_nary_or(new_signals))
            else:
               raise ParseError("Unsupported comparator") 

        result = self.logic_network_.create_nary_and(partial_results)
        return 'BitVec', [result]

    def visit_Constant(self, node):
        if isinstance(node.value, str):
            return 'BitVec', [self.logic_network_.get_constant(i == '1') for i in node.value[::-1]]
        return node.value

    def visit_FunctionDef(self, node):
        if node.returns is None:
            raise ParseError("Return type is needed")
        if not isinstance(node.returns, _ast.Call):
            raise ParseError("Return type is needed")

        return_type = node.returns.func.id
        self.symbol_table_.append({'return': (return_type, None)})
        self.visit_args(node.args)
        return super().generic_visit(node)

    def visit_Name(self, node):
        if node.id not in self.symbol_table_[-1]:
            raise ParseError('out of scope: %s' % node.id)
        return self.symbol_table_[-1][node.id][0], self.symbol_table_[-1][node.id][1]

    def visit_Return(self, node):
        """The return type should match the return type hint."""
        _, signals = self.visit(node.value)
        for s in signals:
            self.logic_network_.create_po(s)

    def visit_Slice(self, node):
        return slice(self.visit(node.lower), self.visit(node.upper))

    def visit_Subscript(self, node):
        v_type, v_signals = self.visit(node.value)
        slice_ = self.visit(node.slice)
        if isinstance(node.slice, ast.Constant):
            return v_type, [v_signals[slice_]]
        print(v_signals)
        print(v_signals[slice_])
        return v_type, v_signals[slice_]

    def visit_UnaryOp(self, node):
        result_type, result_signal = self.visit(node.operand)
        if isinstance(node.op, _ast.Not):
            if len(result_signal) != 1:
                raise ParseError("Boolean NOT doesn't work on multibit values")
            return 'BitVec', [self.logic_network_.create_not(result_signal[0])]
        elif isinstance(node.op, ast.Invert):
            result = list()
            for s in  result_signal:
                result.append(self.logic_network_.create_not(s))
            return 'BitVec', result
        raise ParseError("Unsupported unary operator")
