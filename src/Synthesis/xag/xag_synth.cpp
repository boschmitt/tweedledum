/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/xag_synth.h"
#include "HighLevelXAG.h"
#include "tweedledum/Operators/Extension/Parity.h"
#include "tweedledum/Operators/Standard/Rx.h"
#include "tweedledum/Operators/Standard/X.h"

#include <cassert>

namespace tweedledum {

namespace {

struct Config {
    Config(nlohmann::json const& config)
    {}
};
} // namespace

#pragma region Implementation details
namespace xag_synth_detail {

// Synthesizer functor
class Synthesizer {
public:
    Synthesizer() = default;

    void operator()(mockturtle::xag_network const& xag, Circuit& circuit,
      std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits);

private:
    void pre_process(HighLevelXAG& hl_xag);
    bool try_compute(
      Circuit& circuit, HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref);
    void try_cleanup_inputs(
      Circuit& circuit, HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref);

    void cleanup(
      Circuit& circuit, HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref);

    Qubit request_ancilla(
      Circuit& circuit, HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref);
    void release_ancilla(Circuit& circuit, Qubit qubit);
    void add_parity(Circuit& circuit, std::vector<Qubit> const& qubits);
    void compute_node(Circuit& circuit, Qubit target, HighLevelXAG& hl_xag,
      HighLevelXAG::NodeRef ref);
    void cleanup_node(Circuit& circuit, Qubit target, HighLevelXAG& hl_xag,
      HighLevelXAG::NodeRef ref);

    std::vector<Qubit> qubits_;
    std::vector<Cbit> cbits_;
    // NodeRef -> Qubit, tells in which qubit a node was computed
    std::vector<Qubit> to_qubit_;
    // NodeRef -> Cleanup method
    std::vector<uint8_t> cleanup_;

    // In a out-of-place oracle, output qubits are initialized to |0> state.
    // Thus, these output qubits can be used as ancilla for intermediate
    // computations.  We just have to guarantee that they will be clanned
    // by the time we need them to compute the actual output.
    struct OuputInfo {
        uint32_t last_node;
        uint32_t compute_time;
    };
    std::vector<OuputInfo> qubit_info_;
};

void Synthesizer::pre_process(HighLevelXAG& hl_xag)
{
    uint32_t qubit_idx = 0;
    cleanup_.at(0) = 0; // Don't cleanup the constant!
    // Assing qubits to inputs gates (This is fairly straightforward)
    for (; qubit_idx < hl_xag.num_inputs(); ++qubit_idx) {
        // Note: GateRef == 0 is reserved, hence we sum 1
        uint32_t const gate_ref = qubit_idx + 1;
        to_qubit_.at(gate_ref) = qubits_.at(qubit_idx);
        cleanup_.at(gate_ref) = 0; // Don't cleanup inputs!
    }
    // Assing qubits to gates that drive outputs
    // Note: Here we can save qubits and cleanup gates by identifying
    //       parity_and gates that can be computed directly into an output
    //       qubit
    //
    // Note: This process is greedy (first-come, first-serve)
    //
    // I handle this in two passes:
    // 1st pass: deal with the parity_and drivers
    std::for_each(hl_xag.cbegin_outputs(), hl_xag.cend_outputs(),
      [&](HighLevelXAG::OutputRef ref) {
          HighLevelXAG::NodeRef const node_ref = ref.first;
          // The same gate might drive different outputs.  Here I check if
          // this gate was already assigned a qubit.  If yes, do nothing
          if (to_qubit_.at(node_ref) != Qubit::invalid()) {
              qubit_idx += 1;
              return;
          }
          HighLevelXAG::Node const& node = hl_xag.get_node(node_ref);
          if (!node.is_parity_and()) {
              qubit_idx += 1;
              return;
          }
          for (HighLevelXAG::NodeRef input_ref : node) {
              hl_xag.dereference(input_ref);
          }
          to_qubit_.at(node_ref) = qubits_.at(qubit_idx);
          cleanup_.at(node_ref) = 0;
          qubit_info_.at(qubit_idx).compute_time = node.level();
          bool only_inputs = true;
          for (HighLevelXAG::NodeRef const input_ref : node) {
              HighLevelXAG::Node const& input = hl_xag.get_node(input_ref);
              if (!input.is_input()) {
                  only_inputs = false;
                  break;
              }
          }
          if (only_inputs && node.num_ref() == 0) {
              qubit_info_.at(qubit_idx).compute_time = hl_xag.num_levels();
          }
          qubit_idx += 1;
      });
    // 2nd pass: deal with the parity drivers
    qubit_idx = hl_xag.num_inputs();
    std::for_each(hl_xag.cbegin_outputs(), hl_xag.cend_outputs(),
      [&](HighLevelXAG::OutputRef ref) {
          HighLevelXAG::NodeRef const node_ref = ref.first;
          // The same gate might drive different outputs.  Here I check if
          // this gate was already assigned a qubit.  If yes, do nothing
          if (to_qubit_.at(node_ref) != Qubit::invalid()) {
              qubit_idx += 1;
              return;
          }
          HighLevelXAG::Node const& node = hl_xag.get_node(node_ref);
          if (!node.is_parity()) {
              qubit_idx += 1;
              return;
          }
          to_qubit_.at(node_ref) = qubits_.at(qubit_idx);
          cleanup_.at(node_ref) = 0;
          qubit_info_.at(qubit_idx).compute_time = node.level();
          std::vector<HighLevelXAG::NodeRef> ands;
          bool only_inputs = true;

          for (HighLevelXAG::NodeRef const input_ref : node) {
              HighLevelXAG::Node const& input = hl_xag.get_node(input_ref);
              if (to_qubit_.at(input_ref) != Qubit::invalid()) {
                  if (!input.is_input()) {
                      only_inputs = false;
                  }
                  continue;
              }
              only_inputs = false;
              if (!input.is_parity_and()) {
                  continue;
              }
              ands.push_back(input_ref);
          }
          if (only_inputs && node.num_ref() == 0) {
              qubit_info_.at(qubit_idx).compute_time = hl_xag.num_levels();
          }
          for (HighLevelXAG::NodeRef const input_ref : ands) {
              HighLevelXAG::Node const& input = hl_xag.get_node(input_ref);
              if ((input.num_ref() == 1 || ands.size() == 1)
                  && input.last_level() <= node.level())
              {
                  to_qubit_.at(input_ref) = qubits_.at(qubit_idx);
                  cleanup_.at(input_ref) = 0;
                  qubit_info_.at(qubit_idx).compute_time = std::min(
                    input.level(), qubit_info_.at(qubit_idx).compute_time);
                  hl_xag.dereference(input_ref);
                  for (HighLevelXAG::NodeRef a : input) {
                      hl_xag.dereference(a);
                  }
              }
          }
          qubit_idx += 1;
      });
    HighLevelXAG::NodeRef node_ref = hl_xag.size() - 1;
    std::for_each(
      hl_xag.rbegin(), hl_xag.rend(), [&](HighLevelXAG::Node& node) {
          if (cleanup_.at(node_ref) == 0) {
              node_ref -= 1;
              return;
          }
          for (HighLevelXAG::NodeRef input_ref : node) {
              HighLevelXAG::Node& input = hl_xag.get_node(input_ref);
              if (node.last_level() == input.last_level()) {
                  input.last_level(node.last_level() + 1);
                  continue;
              }
              input.last_level(std::max(node.last_level(), input.last_level()));
          }
          node_ref -= 1;
      });
}

Qubit Synthesizer::request_ancilla(
  Circuit& circuit, HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref)
{
    // HighLevelXAG::Node const& node = hl_xag.get_node(ref);
    // for (uint32_t i = hl_xag.num_inputs(); i < qubits_.size(); ++i) {
    // 	auto& info = qubit_info_.at(i);
    // 	if (info.compute_time <= node.last_level()) {
    // 		continue;
    // 	}
    // 	HighLevelXAG::Node const& node = hl_xag.get_node(info.last_node);
    // 	if (node.num_ref() != 0) {
    // 		continue;
    // 	}
    // 	info.last_node = ref;
    // 	return qubits_.at(i);
    // }
    return circuit.request_ancilla();
}

void Synthesizer::release_ancilla(Circuit& circuit, Qubit qubit)
{
    auto it = std::find(qubits_.crbegin(), qubits_.crend(), qubit);
    if (it != qubits_.crend()) {
        return;
    }
    circuit.release_ancilla(qubit);
}

void Synthesizer::add_parity(Circuit& circuit, std::vector<Qubit> const& qubits)
{
    if (qubits.size() == 1) {
        return;
    }
    circuit.apply_operator(Op::Parity(), qubits, cbits_);
}

void Synthesizer::compute_node(Circuit& circuit, Qubit target,
  HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref)
{
    HighLevelXAG::Node const& node = hl_xag.get_node(ref);
    std::vector<Qubit> in0;
    std::vector<Qubit> in1;
    std::vector<Qubit> in01;

    // fmt::print(">>>>>>>> input\n");
    std::for_each(
      node.cbegin_in0(), node.cend_in0(), [&](HighLevelXAG::NodeRef input_ref) {
          if (to_qubit_.at(input_ref) == target) {
              return;
          }
          in0.push_back(to_qubit_.at(input_ref));
          hl_xag.dereference(input_ref);
          assert(to_qubit_.at(input_ref) != Qubit::invalid());
      });

    if (node.is_parity()) {
        in0.push_back(target);
        add_parity(circuit, in0);
        return;
    }

    std::for_each(
      node.cbegin_in1(), node.cend_in1(), [&](HighLevelXAG::NodeRef input_ref) {
          in1.push_back(to_qubit_.at(input_ref));
          hl_xag.dereference(input_ref);
          assert(to_qubit_.at(input_ref) != Qubit::invalid());
      });

    std::for_each(node.cbegin_in01(), node.cend_in01(),
      [&](HighLevelXAG::NodeRef input_ref) {
          in01.push_back(to_qubit_.at(input_ref));
          hl_xag.dereference(input_ref);
          assert(to_qubit_.at(input_ref) != Qubit::invalid());
      });
    // fmt::print(">>>>>>>> actual: {}, {}, {}\n", in0.size(), in1.size(),
    // in01.size());

    // Compute the inputs to the Toffoli gate (inplace)
    add_parity(circuit, in0);
    if (!in01.empty()) {
        add_parity(circuit, in01);
        in1.push_back(in01.back());
        circuit.apply_operator(Op::X(), {in01.back(), in0.back()}, cbits_);
    }
    add_parity(circuit, in1);
    // fmt::print(">>>>>>>> Tof\n");
    // Compute Toffoli
    Qubit c0 = node.is_negated(0) ? !in0.back() : in0.back();
    Qubit c1 = node.is_negated(1) ? !in1.back() : in1.back();
    if (cleanup_.at(ref) == 1) {
        circuit.apply_operator(Op::Rx(numbers::pi), {c0, c1, target}, cbits_);
    } else {
        circuit.apply_operator(Op::X(), {c0, c1, target}, cbits_);
    }
    // fmt::print(">>>>>>>> C\n");
    // Cleanup the input to the Toffoli gate
    add_parity(circuit, in1);
    if (!in01.empty()) {
        circuit.apply_operator(Op::X(), {in01.back(), in0.back()}, cbits_);
        add_parity(circuit, in01);
    }
    add_parity(circuit, in0);
}

void Synthesizer::cleanup_node(Circuit& circuit, Qubit target,
  HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref)
{
    HighLevelXAG::Node const& node = hl_xag.get_node(ref);
    std::vector<Qubit> in0;
    std::vector<Qubit> in1;
    std::vector<Qubit> in01;

    // fmt::print(">>>>>>>> input\n");
    std::for_each(
      node.cbegin_in0(), node.cend_in0(), [&](HighLevelXAG::NodeRef input_ref) {
          if (to_qubit_.at(input_ref) == target) {
              return;
          }
          in0.push_back(to_qubit_.at(input_ref));
          hl_xag.dereference(input_ref);
          assert(to_qubit_.at(input_ref) != Qubit::invalid());
      });

    if (node.is_parity()) {
        in0.push_back(target);
        add_parity(circuit, in0);
        return;
    }

    std::for_each(
      node.cbegin_in1(), node.cend_in1(), [&](HighLevelXAG::NodeRef input_ref) {
          in1.push_back(to_qubit_.at(input_ref));
          hl_xag.dereference(input_ref);
          assert(to_qubit_.at(input_ref) != Qubit::invalid());
      });

    std::for_each(node.cbegin_in01(), node.cend_in01(),
      [&](HighLevelXAG::NodeRef input_ref) {
          in01.push_back(to_qubit_.at(input_ref));
          hl_xag.dereference(input_ref);
          assert(to_qubit_.at(input_ref) != Qubit::invalid());
      });
    // fmt::print(">>>>>>>> actual: {}, {}, {}\n", in0.size(), in1.size(),
    // in01.size());

    // Compute the inputs to the Toffoli gate (inplace)
    add_parity(circuit, in0);
    if (!in01.empty()) {
        add_parity(circuit, in01);
        in1.push_back(in01.back());
        circuit.apply_operator(Op::X(), {in01.back(), in0.back()}, cbits_);
    }
    add_parity(circuit, in1);
    // fmt::print(">>>>>>>> Tof\n");
    // Compute Toffoli
    Qubit c0 = node.is_negated(0) ? !in0.back() : in0.back();
    Qubit c1 = node.is_negated(1) ? !in1.back() : in1.back();
    circuit.apply_operator(Op::Rx(-numbers::pi), {c0, c1, target}, cbits_);
    // fmt::print(">>>>>>>> C\n");
    // Cleanup the input to the Toffoli gate
    add_parity(circuit, in1);
    if (!in01.empty()) {
        circuit.apply_operator(Op::X(), {in01.back(), in0.back()}, cbits_);
        add_parity(circuit, in01);
    }
    add_parity(circuit, in0);
}

bool Synthesizer::try_compute(
  Circuit& circuit, HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref)
{
    Qubit& qubit = to_qubit_.at(ref);
    if (qubit == Qubit::invalid()) {
        qubit = request_ancilla(circuit, hl_xag, ref);
    } else {
        auto it = std::find(qubits_.cbegin(), qubits_.cend(), qubit);
        assert(it != qubits_.cend());
        uint32_t const idx = std::distance(qubits_.cbegin(), it);
        HighLevelXAG::NodeRef const last_ref = qubit_info_.at(idx).last_node;
        HighLevelXAG::Node const& node = hl_xag.get_node(last_ref);
        if (last_ref != 0u && node.num_ref() != 0) {
            return false;
        }
        qubit_info_.at(idx).last_node = ref;
    }
    assert(qubit != Qubit::invalid());
    // fmt::print(">>>>>> compute\n");
    compute_node(circuit, qubit, hl_xag, ref);
    // fmt::print(">>>>>> done\n");
    return true;
}

void Synthesizer::cleanup(
  Circuit& circuit, HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref)
{
    // fmt::print("cleanup: {}\n", ref);
    cleanup_node(circuit, to_qubit_.at(ref), hl_xag, ref);
    // compute_node(circuit, to_qubit_.at(ref), hl_xag, ref);
    release_ancilla(circuit, to_qubit_.at(ref));
    to_qubit_.at(ref) = Qubit::invalid();
    cleanup_.at(ref) = 0;
}

void Synthesizer::try_cleanup_inputs(
  Circuit& circuit, HighLevelXAG& hl_xag, HighLevelXAG::NodeRef ref)
{
    HighLevelXAG::Node const& node = hl_xag.get_node(ref);
    std::for_each(
      node.crbegin(), node.crend(), [&](HighLevelXAG::NodeRef input_ref) {
          if (cleanup_.at(input_ref) == 0) {
              return;
          }
          HighLevelXAG::Node const& input = hl_xag.get_node(input_ref);
          // Check if the input gate is referenced after the c
          if (input.num_ref() == 0) {
              cleanup(circuit, hl_xag, input_ref);
              try_cleanup_inputs(circuit, hl_xag, input_ref);
          }
      });
}

void Synthesizer::operator()(mockturtle::xag_network const& xag,
  Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits)
{
    HighLevelXAG hl_xag = to_pag(xag);
    qubits_ = qubits;
    cbits_ = cbits;
    to_qubit_.resize(hl_xag.size(), Qubit::invalid());
    qubit_info_.resize(qubits.size(), {0, hl_xag.num_levels()});
    cleanup_.resize(hl_xag.size(), 1); // by default cleanup everything (:

    pre_process(hl_xag);
    // fmt::print("> Done preprocess\n");

    HighLevelXAG::NodeRef node_ref =
      hl_xag.num_inputs() + 1; // inputs + constant
    std::vector<std::vector<HighLevelXAG::NodeRef>> levels(hl_xag.num_levels());
    for (HighLevelXAG::Node const& node : hl_xag) {
        // THIS MEANS there are danglng nodes!
        assert(node.level() != std::numeric_limits<uint32_t>::max());
        levels.at(node.level()).push_back(node_ref);
        node_ref += 1;
    }
    // fmt::print("> Done levels: {}\n", levels.size());

    // Compute steps
    uint32_t lvl = 0;
    for (std::vector<HighLevelXAG::NodeRef> level : levels) {
        std::vector<HighLevelXAG::NodeRef> delayed;
        // fmt::print(">>>> level: {}\n", lvl);
    compute_level:
        for (HighLevelXAG::NodeRef node_ref : level) {
            if (!try_compute(circuit, hl_xag, node_ref)) {
                delayed.push_back(node_ref);
                continue;
            }
            // fmt::print("compute: {}\n", node_ref);
            // Eagerly try to cleanup the inputs of gate that won't
            // need to be cleaned up.  Meaning that this gate is
            // computed in an output qubit
            if (cleanup_.at(node_ref) == 0) {
                try_cleanup_inputs(circuit, hl_xag, node_ref);
            }
        }
        if (!delayed.empty()) {
            assert(delayed.size() != level.size());
            level = delayed;
            delayed.clear();
            // fmt::print(">>>> again\n");
            goto compute_level;
        }
        ++lvl;
    }
    // fmt::print("> Done computing\n");

    //
    uint32_t qubit_idx = hl_xag.num_inputs();
    std::for_each(hl_xag.cbegin_outputs(), hl_xag.cend_outputs(),
      [&](HighLevelXAG::OutputRef ref) {
          HighLevelXAG::NodeRef const node_ref = ref.first;
          if (node_ref == 0 || to_qubit_.at(node_ref) == qubits.at(qubit_idx)) {
              qubit_idx += 1;
              return;
          }
          circuit.apply_operator(
            Op::X(), {to_qubit_.at(node_ref), qubits.at(qubit_idx)}, cbits_);
          qubit_idx += 1;
      });
    qubit_idx = hl_xag.num_inputs();
    std::for_each(hl_xag.cbegin_outputs(), hl_xag.cend_outputs(),
      [&](HighLevelXAG::OutputRef ref) {
          if (ref.second) {
              circuit.apply_operator(Op::X(), {qubits.at(qubit_idx)}, cbits_);
          }
          qubit_idx += 1;
      });
}

} // namespace xag_synth_detail
#pragma endregion

void xag_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, mockturtle::xag_network const& xag,
  nlohmann::json const& config)
{
    xag_synth_detail::Synthesizer xag_synthesize;
    xag_synthesize(xag, circuit, qubits, cbits);
}

Circuit xag_synth(
  mockturtle::xag_network const& xag, nlohmann::json const& config)
{
    Circuit circuit;
    // Config cfg(config);
    uint32_t num_qubits = xag.num_pis() + xag.num_pos();
    std::vector<Qubit> qubits;
    qubits.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    xag_synth(circuit, qubits, {}, xag, config);
    return circuit;
}

} // namespace tweedledum
