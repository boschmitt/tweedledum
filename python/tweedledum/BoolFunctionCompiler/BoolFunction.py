#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import inspect
import re
import string
import types

from .Parser import Parser
from ..libPyTweedledum import Classical

class BoolFunction(object):
    """Class to represent a Boolean function


    Attributes
    ----------
    name_ : str
        Function name
    signature_ : list
        Defines the input and output types of the function
    symbol_table_ : dict(str : Signal)
        A table that maps inputs and outputs of the function to their respective
        signals in the LogicNetwork
    logic_network_ : LogicNetwork
        A representation of the function as a XAG (XOR-AND Graph)
    truth_table_ : list(TruthTable)
        A representation of the function as a truth table. Note that if the
        function has multiple outputs, then each output will be represented by
        one truth table
    """

    def __init__(self, f):
        if not isinstance(f, types.FunctionType) and not isinstance(f, str):
            raise TypeError("[BoolFunction] Constructor requires a function or "
                            "a string (expression)")
        if isinstance(f, str):
            source = self._expr_to_source(f)
            self.name_ = "f"
        else:
            source = inspect.getsource(f).strip()
            self.name_ = f.__name__
        parsed_function = Parser(source)
        self.signature_ = parsed_function.signature_
        self.symbol_table_ = parsed_function.symbol_table_
        self.logic_network_ = parsed_function.logic_network_
        self.truth_table_ = None

    def _expr_to_source(self, expr):
        args = list(filter(None, [arg.strip() for arg in re.split(r' and | or |not |~|\&|\||\^|[()]', expr)]))
        args = sorted(set(args), key=args.index)
        source = f"def f(" + ", ".join(args) + " : BitVec(1)) -> BitVec(1):\n"
        source += f"    return {expr}"
        return source

    def simulate(self, *argv):
        if len(argv) != len(self.signature_):
            raise TypeError("[BoolFunction] The function requires "
                            f"{len(self.signature_)}. It's signature is: "
                            f"{self.signature_}")
        input_str = str()
        for i, arg in enumerate(argv):
            arg_type = [type(arg), len(arg)]
            if arg_type != self.signature_[i]:
                raise TypeError("[BoolFunction] Wrong argument type. "
                                f"Argument {i} expected: {self.signature_[i]}, "
                                f"got: {arg_type}")
            arg_str = str(arg)
            input_str += arg_str[::-1]
        if self.truth_table_ != None:
            position = int(input_str)
            return [tt[position] for tt in self.truth_table_]
        input_vector = [bool(int(i)) for i in input_str]
        return Classical.simulate(self.logic_network_, input_vector)

    def simulate_all(self):
        if self.truth_table_ == None:
            self.truth_table_ = Classical.simulate(self.logic_network_)
        return self.truth_table_

    def num_ones(self):
        if not self.truth_table_:
            self.truth_table_ = Classical.xag_simulate(self.logic_network_)
        return [Classical.count_ones(tt) for tt in self.truth_table_]

    def print_tt(self, fancy = False):
        if not self.truth_table_:
            self.truth_table_ = Classical.xag_simulate(self.logic_network_)
        if not fancy:
            for idx, tt in enumerate(self.truth_table_):
                print("[{}] : {}".format(idx, tt))
            return

        # Fancy print
        # reversing the strings helps:
        tt_strings = [str(tt)[::-1]  for tt in self.truth_table_]
        for i in range(self.logic_network_.num_pis() - 1, -1, -1):
            print("{:^3}".format(string.ascii_lowercase[i]), end="")
        print(" | ", end="")
        for i in range(self.logic_network_.num_pos() - 1, -1, -1):
            print("{:^3}".format(i), end="")
        print("")
        table_width = (self.logic_network_.num_pis() + self.logic_network_.num_pos()) * 3 + 3
        print("{:-^{}}".format('', table_width))
        for i in range(0, (1 << self.logic_network_.num_pis())):
            i_bool = "{:0{}b}".format(i, self.logic_network_.num_pis())
            for j in range(0, len(i_bool)):
                print("{:^3}".format(i_bool[j]), end="")
            print(" | ", end="")
            for j in range(self.logic_network_.num_pos() - 1, -1, -1):
                print("{:^3}".format(tt_strings[j][i]), end="")
            print("")
