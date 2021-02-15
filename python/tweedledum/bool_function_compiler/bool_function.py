#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
from typing import Union, List
import functools 
import inspect
import math
import string
import types

from .bitvec import BitVec
from .function_parser import FunctionParser
from .._tweedledum.classical import TruthTable, create_from_binary_string
from .._tweedledum import classical

class BoolFunction(object):
    """Class to represent a Boolean function

    Formally, a Boolean function is a mapping :math:`f : {0, 1}^n \to {0, 1}^m`,
    where :math:`n`(:math:`m`) is the number of inputs(outputs).  There are 
    many ways to represent/specify a Boolean function.  Here, we use two:

    Truth tables:  They are are an explicit and _not compact_ function
        representation.  Basically a truth table is an exhaustive mapping from
        input binary bit-strings of length :math:`n` to corresponding output
        bit-strings of length :math:`m`. Hence they do not scale well. They are,
        however, tremendously useful to represent and manipulate small functions.

    Logic network (Xor-And graph, XAG): A logic network is modeled by a directed
        acyclic graph where nodes represent primary inputs and outputs, as well
        as local functions.  The nodes representing local function as called 
        gates.  In our case, we limit our local functions to be either a 2-input
        AND or a 2-input XOR---a structure known as XAG.  Therefore, a XAG is a
        2-regular non-homogeneous logic network.

    Under the hood both representations are implemented in C++.
    """
    def __init__(self, f):
        if not isinstance(f, types.FunctionType):
            raise TypeError("Constructor requires a function")
        parsed_function = FunctionParser(inspect.getsource(f).strip())
        self._parameters_signature = parsed_function._parameters_signature
        self._return_signature = parsed_function._return_signature
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
            raise RuntimeError(f"The function requires {self.num_inputs()}. "
                               f"It's signature is: {self._parameters_signature}")
        input_str = str()
        for i, arg in enumerate(argv):
            arg_type = (type(arg), len(arg))
            if arg_type != self._parameters_signature[i]:
                raise TypeError(f"Wrong argument type. Argument {i} "
                                f"expected: {self._parameters_signature[i]}, "
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
            raise TypeError("Parameter output must be an integer")
        if self._truth_table == None:
            self.simulate_all()
        return self._truth_table[output_bit]

    @classmethod
    def from_expression(cls, expression: str):
        """
        Create a BooleanFunction from an arbitrary logical expression.

        A logical expression is composed of logical operators & (AND), | (OR),
        ~ (NOT), and ^ (XOR), as well as symbols for variables. For example,
        'a & b' and '~(~(x0 | x1) ^ (~x0 & ~x1))' are both valid string 
        representation of logical expressions.

        Logic expressions are parsed and stored, initially, as a LogicNetwork,
        which is a XOR-AND graph (XAG). 

        Args:
            expression: The string of the desired logical expression, such as
                        'a & b' or '~(~(x0 | x1) ^ (~x0 & ~x1))' 

        Returns:
            A BooleanFunction that uses a XAG to represent the logical expression
        """
        from .expression_parser import ExpressionParser

        function = cls.__new__(cls)
        parsed_expression = ExpressionParser(expression)
        function._parameters_signature = parsed_expression._parameters_signature
        function._return_signature = parsed_expression._return_signature
        function._logic_network = parsed_expression._logic_network
        function._truth_table = None
        function._num_input_bits = len(function._parameters_signature)
        function._num_output_bits = 1
        return function

    @classmethod
    def from_function(cls, function):
        """Create a BooleanFunction from a python function.

        Returns:
            A BooleanFunction for the python function
        """
        return BoolFunction(function)

    @classmethod
    def from_truth_table(cls, tts: Union[str, List[str]]):
        """Create a BooleanFunction from an truth table string.

        Truth tables are a common way of specifying boolean functions. They are
        are an explicit and _not compact_ function representation---basically a 
        truth table is an exhaustive mapping from input binary bit-strings of 
        length :math:`n` to corresponding output bit-strings of length :math:`m`.
        However, they are tremendously useful to represent and manipulate small
        functions.  For example, the following is a simple truth table that
        corresponds to the `AND` of two variables:

        =====  =====  =============
           Inputs        Output
        ------------  -------------
          a      b      f(a, b)
        =====  =====  =============
          0      0         0
          0      1         0
          1      0         0
          1      1         1
        =====  =====  =============

        In this case :math:`n=2`, and :math:`m=1`.  Note that, for compactness,
        the input bit-strings are omitted because they can be easily derived
        for any given :math:`n`.  Hence to completely specify a truth table, 
        we only need a Length-2 :sup:`n` bit-string for each of the :math:`m`
        outputs.  In the above example, a single bit-string `'1000'` is enough.
        The most-significant bit corresponds to the :math:`f(1, 1)` and the 
        least-significant bit corresponds to :math:`f(0, 0)`.

        Args:
            tt: A single binary string or a list of binary strings representing
                the a single- or multi-output truth table.

        Returns:
            A BooleanFunction that uses truth tables to represent the function
        """
        if isinstance(tts, str):
            tts = [tts]

        # Check that the input binary stings have the same length and that this 
        # length is a power of 2
        if not ((len(tts[0]) & (len(tts[0]) - 1) == 0) and len(tts[0]) != 0):
            raise RuntimeError("Length of all binary string must be a power of 2")
        for tt in tts[1:]:
            if len(tt) != len(tts[0]):
                raise RuntimeError("Length of all binary string must equal")
        
        n_inputs = int(math.log(len(tts[0]), 2))
        n_outputs = len(tts)

        function = cls.__new__(cls)
        function._logic_network = None
        function._truth_table = [TruthTable(n_inputs) for _ in range(n_outputs)]
        for i, tt in enumerate(tts):
            create_from_binary_string(function._truth_table[i], tt)
        function._parameters_signature = [(type(BitVec(1)), 1)] * n_inputs
        function._return_signature = [(type(BitVec(1)), 1)] * n_outputs
        function._num_input_bits = n_inputs
        function._num_output_bits = n_outputs
        return function

    @classmethod
    def from_aiger_file(cls, path: str):
        """Create a BooleanFunction from an AIGER file.

        AIGER is a binary format to store Boolean functions that are represented 
        as And-Inverter Graphs (AIGs).  An AIG is a 2-regular homogeneous logic
        network, in which each gate function is the 2-input AND. Inverters are 
        modelled using the complementation flags.

        The logic network used in `BoolFunction` is _not_ an AIG.  We use a XAG
        instead.  A XAG is a 2-regular non-homogeneous logic network, in which
        each gate function is either a 2-input AND or a 2-input XOR.  Inverters 
        are modelled using the complementation flags, but they are not strictly
        necessary.  Thus this function reads an AIG from a AIGER file and 
        transforms it into a XAG.

        Args:
            path: Path to the AIGER file (*.aig)

        Returns:
            A BooleanFunction that uses a XAG to represent the function
        """
        function = cls.__new__(cls)
        function._logic_network = classical.read_aiger(path)
        function._truth_table = None
        n_inputs = function._logic_network.num_pis()
        n_outputs = function._logic_network.num_pos()
        function._parameters_signature = [(type(BitVec(1)), 1)] * n_inputs
        function._return_signature = [(type(BitVec(1)), 1)] * n_outputs
        function._num_input_bits = n_inputs
        function._num_output_bits = n_outputs
        return function

    @classmethod
    def from_dimacs_file(cls, path: str):
        """Create a BooleanFunction from a DIMACS CNF file.

        The `DIMACS CNF format 
        <http://www.satcompetition.org/2009/format-benchmarks2009.html>`__, is 
        the standard format for specifying SATisfiability (SAT) problem 
        instances in `Conjunctive Normal Form (CNF) 
        <https://en.wikipedia.org/wiki/Conjunctive_normal_form>`__, which is a 
        two-level representation of a Boolean function, in which the inner
        operator is OR and the outter operator is AND.  Thus a CNF is a 
        conjunction of one or more clauses, where a clause is a disjunction of
        one or more literals.
        
        Args:
            path: Path to the DIMACS file (*.cnf)

        Returns:
            A BooleanFunction that uses a XAG to represent the function
        """
        function = cls.__new__(cls)
        function._logic_network = classical.read_dimacs(path)
        function._truth_table = None
        n_inputs = function._logic_network.num_pis()
        n_outputs = function._logic_network.num_pos()
        function._parameters_signature = [(type(BitVec(1)), 1)] * n_inputs
        function._return_signature = [(type(BitVec(1)), 1)] * n_outputs
        function._num_input_bits = n_inputs
        function._num_output_bits = n_outputs
        return function

    @classmethod
    def from_verilog_file(cls, path: str):
        """Create a BooleanFunction from Verilog file.

        /!\ This function can only parse a _very_ small and simple subset of 
        /!\ the Verilog language.

        Verilog is a hardware description language (HDL) used to model
        electronic systems.  Here we are interested on its capability to model
        purely combinational circuits.  

        Args:
            path: Path to the Verilog file (*.v)

        Returns:
            A BooleanFunction that uses a XAG to represent the function
        """
        function = cls.__new__(cls)
        function._logic_network = classical.read_verilog(path)
        function._truth_table = None
        n_inputs = function._logic_network.num_pis()
        n_outputs = function._logic_network.num_pos()
        function._parameters_signature = [(type(BitVec(1)), 1)] * n_inputs
        function._return_signature = [(type(BitVec(1)), 1)] * n_outputs
        function._num_input_bits = n_inputs
        function._num_output_bits = n_outputs
        return function
