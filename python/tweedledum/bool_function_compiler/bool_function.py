#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import inspect
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

    def simulate(self, *argv):
        if len(argv) != len(self._parameters_signature):
            raise TypeError("[BoolFunction] The function requires "
                            f"{len(self._parameters_signature)}. It's signature is: "
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

    def simulate_all(self):
        if self._truth_table == None:
            self._truth_table = classical.simulate(self._logic_network)

        result = list()
        for position in range(2 ** self._logic_network.num_pis()):
            sim_result = ''.join([str(int(tt[position])) for tt in self._truth_table])
            result.append(self._format_simulation_result(sim_result))

        return result

    @classmethod
    def from_aiger_file(cls, filename: str, name=None):
        """Create a BooleanFunction from an AIGER file.

        Returns:
            A BooleanFunction for the input file
        """
        bool_func_instance = cls.__new__(cls)
        bool_func_instance._logic_network = classical.read_aiger(filename)
        bool_func_instance._parameters_signature = []
        for _ in range(bool_func_instance._logic_network.num_pis()):
            bool_func_instance._parameters_signature.append((type(BitVec(1)), 1))
        bool_func_instance._return_signature = [(type(BitVec(1)), 1)]
        return bool_func_instance

    @classmethod
    def from_verilog_file(cls, filename: str, name=None):
        """Create a BooleanFunction from Verilog file.

        Returns:
            A BooleanFunction for the input file
        """
        bool_func_instance = cls.__new__(cls)
        bool_func_instance._logic_network = classical.read_verilog(filename)
        bool_func_instance._parameters_signature = []
        for _ in range(bool_func_instance._logic_network.num_pis()):
            bool_func_instance._parameters_signature.append((type(BitVec(1)), 1))
        bool_func_instance._return_signature = [(type(BitVec(1)), 1)]
        return bool_func_instance