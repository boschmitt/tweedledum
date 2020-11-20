#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import string

from .Parser import Parser
from ..libPyTweedledum import Classical

class CFunc(object):
    def __init__(self, type, source, name = None):
        self.name_ = name or 'classical_function'
        parsed_function = Parser(source)
        self.symbol_table_ = parsed_function.symbol_table_
        self.logic_network_ = parsed_function.logic_network_
        self.simulated_tts_ = None

    def num_ones(self):
        if not self.simulated_tts_:
            self.simulated_tts_ = Classical.simulate(self.logic_network_)
        return [Classical.count_ones(tt) for tt in self.simulated_tts_]

    def print_tt(self, fancy = False):
        if not self.simulated_tts_:
            self.simulated_tts_ = Classical.simulate(self.logic_network_)
        if not fancy:
            for idx, tt in enumerate(self.simulated_tts_):
                print("[{}] : {}".format(idx, tt))
            return

        # Fancy print
        # reversing the strings helps:
        tt_strings = [str(tt)[::-1]  for tt in self.simulated_tts_]
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
