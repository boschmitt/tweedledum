#-------------------------------------------------------------------------------
# Part of tweedledum.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import ast
import _ast
import sys
from .. import core

class ParseError(Exception):
    pass

class LogicNetwork(ast.NodeVisitor):
    bitops = {_ast.BitAnd: 'create_and',
              _ast.BitOr: 'create_or',
              _ast.BitXor: 'create_xor',
              _ast.And: 'create_and',
              _ast.Or: 'create_or',
              _ast.Not: 'create_not',
              _ast.Invert: 'create_not'
              }

    def __init__(self, source):
        self.source = source
        node = ast.parse(source)
        self.scopes = []
        self._network = None
        self.visit(node)
        super().__init__()

    @property
    def types(self):
        """returns vars ands types in the scope"""
        ret = []
        for scope in self.scopes:
            ret.append({k: v[0] for k, v in scope.items()})
        return ret

    def simulate(self):
        return core.simulate(self._network)

    def synthesize(self) -> str:
        return core.synthesize_xag(self._network)

    def visit_Module(self, node):
        if len(node.body) != 1 and not isinstance(node.body[0], ast.FunctionDef):
            raise ParseError("just functions, sorry!")
        self.visit(node.body[0])

    def visit_FunctionDef(self, node):
        if node.returns is None:
                raise ParseError("return type is needed")
        self.scopes.append({'return': (node.returns.id, None),
                            node.returns.id: ('type', None)})
        self._network = core.xag_network()
        self.extend_scope(node.args)
        return super().generic_visit(node)

    def visit_Return(self, node):
        _type, signal = self.visit(node.value)
        if _type != self.scopes[-1]['return'][0]:
            raise ParseError("return type error")
        self._network.create_po(signal)
        return

    def visit_Assign(self, node):
        type_value, signal_value = self.visit(node.value)
        for target in node.targets:
            self.scopes[-1][target.id] = (type_value, signal_value)
            # _, _ = self.visit(target)
        return (type_value, signal_value)

    def bit_binop(self, left, right, op):
        left_type, left_signal = left
        right_type, right_signal = right
        if left_type != 'Bit' or right_type != 'Bit':
            raise ParseError("binop type error")
        bitop = LogicNetwork.bitops.get(type(op))
        if bitop:
            return 'Bit', getattr(self._network, bitop)(left_signal, right_signal)
        else:
            raise ParseError("Unknown binop.op %s" % op)

    def visit_BoolOp(self, node):
        """ node.left=Bit and node.right=Bit return Bit """
        return self.bit_binop(self.visit(node.values[0]), self.visit(node.values[1]), node.op)

    def visit_BinOp(self, node):
        """ node.left=Bit and node.right=Bit return Bit """
        return self.bit_binop(self.visit(node.left), self.visit(node.right), node.op)

    def visit_UnaryOp(self, node):
        operand_type, operand_signal = self.visit(node.operand)
        bitop = LogicNetwork.bitops.get(type(node.op))
        if bitop:
            return 'Bit', getattr(self._network, bitop)(operand_signal)
        else:
            raise ParseError("Unknown UntaryOp.op %s" % node.op)

    def visit_Name(self, node):
        if node.id not in self.scopes[-1]:
            raise ParseError('out of scope: %s' % node.id)
        return self.scopes[-1][node.id]

    def generic_visit(self, node):
        if isinstance(node, (_ast.arguments, _ast.arg, _ast.Load, _ast.BitAnd,
                             _ast.BitOr, _ast.BitXor, _ast.BoolOp, _ast.Or)):
            return super().generic_visit(node)
        raise ParseError("Unknown node: %s" % type(node))

    def extend_scope(self, args_node: _ast.arguments) -> None:
        for arg in args_node.args:
            if arg.annotation is None:
                raise ParseError("argument type is needed")
            self.scopes[-1][arg.annotation.id] = ('type', None)
            self.scopes[-1][arg.arg] = (arg.annotation.id, self._network.create_pi())
