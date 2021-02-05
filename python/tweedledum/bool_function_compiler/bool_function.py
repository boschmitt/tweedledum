#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import inspect
import re
import string
import types

from .BitVec import BitVec
from .Parser import Parser
from ..libPyTweedledum import classical

class BoolFunction(object):
    """Class to represent a Boolean function
    """

    def __init__(self, f):
        if not isinstance(f, types.FunctionType):
            raise TypeError("[BoolFunction] Constructor requires a function")
        parsed_function = Parser(inspect.getsource(f).strip())
        self._parameters_signature = parsed_function._parameters_signature
        self._return_signature = parsed_function._return_signature
        self._symbol_table = parsed_function._symbol_table
        self._logic_network = parsed_function._logic_network
        self._truth_table = None

    def _expr_to_source(self, expr):
        args = list(filter(None, [arg.strip() for arg in re.split(r' and | or |not |~|\&|\||\^|[()]', expr)]))
        args = sorted(set(args), key=args.index)
        source = f"def f(" + ", ".join(args) + " : BitVec(1)) -> BitVec(1):\n"
        source += f"    return {expr}"
        return source

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
            sim_result = Classical.simulate(self._logic_network, input_vector)
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
            self._truth_table = Classical.simulate(self._logic_network)

        result = list()
        for position in range(2 ** self._logic_network.num_pis()):
            sim_result = ''.join([str(int(tt[position])) for tt in self._truth_table])
            result.append(self._format_simulation_result(sim_result))

        return result

    def num_ones(self):
        if not self._truth_table:
            self._truth_table = Classical.xag_simulate(self._logic_network)
        return [Classical.count_ones(tt) for tt in self._truth_table]

    def print_tt(self, fancy = False):
        if not self._truth_table:
            self._truth_table = Classical.xag_simulate(self._logic_network)
        if not fancy:
            for idx, tt in enumerate(self._truth_table):
                print("[{}] : {}".format(idx, tt))
            return

        # Fancy print
        # reversing the strings helps:
        tt_strings = [str(tt)[::-1]  for tt in self._truth_table]
        for i in range(self._logic_network.num_pis() - 1, -1, -1):
            print("{:^3}".format(string.ascii_lowercase[i]), end="")
        print(" | ", end="")
        for i in range(self._logic_network.num_pos() - 1, -1, -1):
            print("{:^3}".format(i), end="")
        print("")
        table_width = (self._logic_network.num_pis() + self._logic_network.num_pos()) * 3 + 3
        print("{:-^{}}".format('', table_width))
        for i in range(0, (1 << self._logic_network.num_pis())):
            i_bool = "{:0{}b}".format(i, self._logic_network.num_pis())
            for j in range(0, len(i_bool)):
                print("{:^3}".format(i_bool[j]), end="")
            print(" | ", end="")
            for j in range(self._logic_network.num_pos() - 1, -1, -1):
                print("{:^3}".format(tt_strings[j][i]), end="")
            print("")
