#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import functools 
import inspect
import math
import re
import string
import types

from .bitvec import BitVec
from .function_parser import FunctionParser
from .._tweedledum import classical

class BoolFunction(object):
    """Class to represent a Boolean function
    """
    def __init__(self, f):
        if not isinstance(f, types.FunctionType):
            raise TypeError("[BoolFunction] Constructor requires a function")
        parsed_function = FunctionParser(inspect.getsource(f).strip())
        self._parameters_signature = parsed_function._parameters_signature
        self._return_signature = parsed_function._return_signature
        self._symbol_table = parsed_function._symbol_table
        self._logic_network = parsed_function._logic_network
        self._truth_table = None
        self._num_input_bits = self._logic_network.num_pis()
        self._num_output_bits = self._logic_network.num_pos()

    def _format_simulation_result(self, sim_result):
        i = 0
        result = list()
        for type_, size in self._return_signature:
            tmp = sim_result[i:i+size]
            result.append(type_(size, tmp[::-1]))
            i += size
        if len(result) == 1:
            return result[0]
        return tuple(result)

    def num_inputs(self):
        return len(self._parameters_signature)

    def num_outputs(self):
        return len(self._return_signature)

    def num_input_bits(self):
        return self._num_input_bits

    def num_output_bits(self):
        return self._num_output_bits

    def simulate(self, *argv):
        if len(argv) != self.num_inputs():
            raise TypeError("[BoolFunction] The function requires "
                            f"{self.num_inputs()}. It's signature is: "
                            f"{self._parameters_signature}")
        input_str = str()
        for i, arg in enumerate(argv):
            arg_type = (type(arg), len(arg))
            if arg_type != self._parameters_signature[i]:
                raise TypeError("[BoolFunction] Wrong argument type. "
                                f"Argument {i} expected: {self._parameters_signature[i]}, "
                                f"got: {arg_type}")
            arg_str = str(arg)
            input_str += arg_str[::-1]
        
        # If the truth table was already computed, we just need to look for the
        # result of this particular input
        if self._truth_table != None:
            position = int(input_str[::-1], base=2)
            sim_result = ''.join([str(int(tt[position])) for tt in self._truth_table])
        else:
            input_vector = [bool(int(i)) for i in input_str]
            sim_result = classical.simulate(self._logic_network, input_vector)
            sim_result = ''.join([str(int(i)) for i in sim_result])

        return self._format_simulation_result(sim_result)

    def simulate_all(self):
        if self._truth_table == None:
            self._truth_table = classical.simulate(self._logic_network)

        result = list()
        for position in range(2 ** self._logic_network.num_pis()):
            sim_result = ''.join([str(int(tt[position])) for tt in self._truth_table])
            result.append(self._format_simulation_result(sim_result))

        return result

    def logic_network(self):
        return self._logic_network

    def truth_table(self, output_bit : int):
        if not isinstance(output_bit, int):
            raise TypeError("[BoolFunction] parameter output must be a integer")
        if self._truth_table == None:
            self.simulate_all()
        return self._truth_table[output_bit]

    @classmethod
    def from_expression(cls, expression: str):
        """Create a BooleanFunction from an expression string.

        Returns:
            A BooleanFunction for the expression
        """
        if not isinstance(expression, str):
            raise TypeError("[BoolFunction] expression must be a string")
        from .expression_parser import ExpressionParser
        bfunc_instance = cls.__new__(cls)
        parsed_expression = ExpressionParser(expression)
        bfunc_instance._parameters_signature = parsed_expression._parameters_signature
        bfunc_instance._return_signature = parsed_expression._return_signature
        bfunc_instance._symbol_table = parsed_expression._symbol_table
        bfunc_instance._logic_network = parsed_expression._logic_network
        bfunc_instance._truth_table = None
        bfunc_instance._num_input_bits = len(bfunc_instance._parameters_signature)
        bfunc_instance._num_output_bits = 1
        return bfunc_instance

    @classmethod
    def from_function(cls, function):
        """Create a BooleanFunction from a python function.

        Returns:
            A BooleanFunction for the python function
        """
        return BoolFunction(function)

    @classmethod
    def from_truth_table(cls, tt: str):
        """Create a BooleanFunction from an truth table string.

        Returns:
            A BooleanFunction for the truth table
        """
        if not isinstance(tt, str):
            raise TypeError("[BoolFunction] the truth table must be a bit string")
        if not ((len(tt) & (len(tt) - 1) == 0) and len(tt) != 0):
            raise TypeError("[BoolFunction] length must be a power of two")
        from .._tweedledum.classical import TruthTable, create_from_binary_string
        bfunc_instance = cls.__new__(cls)
        bfunc_instance._logic_network = None
        bfunc_instance._truth_table = [TruthTable(int(math.log(len(tt), 2)))]
        create_from_binary_string(bfunc_instance._truth_table[0], tt)
        bfunc_instance._parameters_signature = list()
        for _ in range(bfunc_instance._truth_table[0].num_vars()):
            bfunc_instance._parameters_signature.append((type(BitVec(1)), 1))
        bfunc_instance._return_signature = [(type(BitVec(1)), 1)]
        bfunc_instance._num_input_bits = len(bfunc_instance._parameters_signature)
        bfunc_instance._num_output_bits = 1

        return bfunc_instance

    @classmethod
    def from_aiger_file(cls, filename: str):
        """Create a BooleanFunction from an AIGER file.

        Returns:
            A BooleanFunction for the input file
        """
        bfunc_instance = cls.__new__(cls)
        bfunc_instance._logic_network = classical.read_aiger(filename)
        bfunc_instance._parameters_signature = []
        for _ in range(bfunc_instance._logic_network.num_pis()):
            bfunc_instance._parameters_signature.append((type(BitVec(1)), 1))
        bfunc_instance._return_signature = [(type(BitVec(1)), 1)]
        bfunc_instance._num_input_bits = bfunc_instance._logic_network.num_pis()
        bfunc_instance._num_output_bits = bfunc_instance._logic_network.num_pos()
        return bfunc_instance

    @classmethod
    def from_dimacs_file(cls, filename: str):
        """Create a BooleanFunction from the string in the DIMACS format.
        Args:
            dimacs: The string in the DIMACS format.

        Returns:
            A quantum circuit (BooleanFunction) for the input string
        """
        from .._tweedledum.classical import read_dimacs
        bfunc_instance = cls.__new__(cls)
        bfunc_instance._logic_network = read_dimacs(filename)
        bfunc_instance._parameters_signature = list()
        for _ in range(bfunc_instance._logic_network.num_pis()):
            bfunc_instance._parameters_signature.append((type(BitVec(1)), 1))
        bfunc_instance._return_signature = [(type(BitVec(1)), 1)]
        bfunc_instance._num_input_bits = bfunc_instance._logic_network.num_pis()
        bfunc_instance._num_output_bits = bfunc_instance._logic_network.num_pos()
        return bfunc_instance

    @classmethod
    def from_verilog_file(cls, filename: str):
        """Create a BooleanFunction from Verilog file.

        Returns:
            A BooleanFunction for the input file
        """
        bfunc_instance = cls.__new__(cls)
        bfunc_instance._logic_network = classical.read_verilog(filename)
        bfunc_instance._parameters_signature = []
        for _ in range(bfunc_instance._logic_network.num_pis()):
            bfunc_instance._parameters_signature.append((type(BitVec(1)), 1))
        bfunc_instance._return_signature = [(type(BitVec(1)), 1)]
        bfunc_instance._num_input_bits = bfunc_instance._logic_network.num_pis()
        bfunc_instance._num_output_bits = bfunc_instance._logic_network.num_pos()
        return bfunc_instance
