/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/a_star_swap_synth.h"
#include "tweedledum/Operators/Standard/Swap.h"
#include "tweedledum/Utils/Hash.h"

namespace tweedledum {

namespace {

class AStarSwapper {
private:
    using Map = std::vector<uint32_t>;
    using Swap = std::pair<uint32_t, uint32_t>;

    struct Node {
        Map mapping;
        uint32_t swap;
        uint32_t previous;
        uint32_t g;
        uint32_t h;
        bool closed;

        Node(Map const& mapping_, uint32_t edge, uint32_t prev, uint32_t g_,
          uint32_t h_)
            : mapping(mapping_)
            , swap(edge)
            , previous(prev)
            , g(g_)
            , h(h_)
            , closed(false)
        {}
    };

public:
    explicit AStarSwapper(Device const& device)
        : device_(device)
    {}

    std::vector<Swap> run(
      Map const& init_mapping, Map const& final_mapping, bool admissable = true)
    {
        std::vector<Node> nodes;
        std::vector<uint32_t> open_nodes;
        std::vector<uint32_t> closed_nodes;
        std::unordered_map<Map, uint32_t, Hash<Map>> mappings;

        mappings.emplace(init_mapping, 0);
        nodes.emplace_back(init_mapping, 0, 0, 0, 0);
        open_nodes.emplace_back(0);
        while (!open_nodes.empty()) {
            Node node = nodes.at(open_nodes.back());
            closed_nodes.push_back(open_nodes.back());
            nodes.at(open_nodes.back()).closed = true;
            if (node.mapping == final_mapping) {
                break;
            }
            open_nodes.pop_back();
            assert(node.mapping.size() == init_mapping.size());
            for (uint32_t i = 0; i < device_.num_edges(); ++i) {
                Map new_mapping = node.mapping;
                std::swap(new_mapping.at(device_.edge(i).first),
                  new_mapping.at(device_.edge(i).second));

                // Try to add new mapping to the mappings database
                auto [it, was_added] =
                  mappings.emplace(new_mapping, nodes.size());
                Node& new_node = was_added ? nodes.emplace_back(new_mapping, i,
                                   closed_nodes.back(), node.g + 1, 0)
                                           : nodes.at(it->second);

                if (was_added) {
                    // If a new node was created, need to add to the list of
                    // nodes
                    open_nodes.push_back(nodes.size() - 1);
                } else if (new_node.g <= node.g + 1) {
                    // Do not update a node if its new cost exeeds the previous
                    // one
                    continue;
                } else if (new_node.closed) {
                    // If new cost is smaller and the node was already closed,
                    // re-open it!
                    new_node = nodes.emplace_back(
                      new_mapping, i, closed_nodes.back(), node.g + 1, 0);
                    open_nodes.push_back(nodes.size() - 1);
                    mappings.at(new_mapping) = nodes.size() - 1;
                }
                for (auto k = 0ull; k < final_mapping.size(); ++k) {
                    if (new_node.mapping[k] != final_mapping[k]) {
                        auto it = std::find(final_mapping.begin(),
                          final_mapping.end(), new_node.mapping[k]);
                        auto idx = std::distance(final_mapping.begin(), it);
                        new_node.h += device_.distance(k, idx);
                    }
                }
                if (admissable) {
                    new_node.h = std::ceil(new_node.h / 2.0);
                }
            }
            auto min_it = std::min_element(open_nodes.begin(), open_nodes.end(),
              [&](auto a_idx, auto b_idx) {
                  auto& a = nodes[a_idx];
                  auto& b = nodes[b_idx];
                  return (a.g + a.h) < (b.g + b.h);
              });
            std::swap(*min_it, open_nodes.back());
        }

        // Reconstruct sequence of swaps
        std::vector<Swap> swaps;
        auto& node = nodes.at(closed_nodes.back());
        while (node.previous) {
            swaps.emplace_back(device_.edge(node.swap));
            node = nodes.at(node.previous);
        }
        if (closed_nodes.size() > 1) {
            swaps.emplace_back(device_.edge(node.swap));
        }
        std::reverse(swaps.begin(), swaps.end());
        return swaps;
    }

private:
    Device const& device_;
};

} // namespace

Circuit a_star_swap_synth(Device const& device,
  std::vector<uint32_t> const& init_cfg, std::vector<uint32_t> const& final_cfg,
  nlohmann::json const& config)
{
    using Swap = std::pair<uint32_t, uint32_t>;
    std::vector<Swap> swaps;
    AStarSwapper swapper(device);
    // if (params.method == swap_network_params::methods::non_admissible) {
    //     return swapper.run(init_mapping, final_mapping, false);
    // }
    swaps = swapper.run(init_cfg, final_cfg);
    Circuit circuit;
    for (uint32_t i = 0u; i < device.num_qubits(); ++i) {
        circuit.create_qubit();
    }
    for (auto [x, y] : swaps) {
        circuit.apply_operator(Op::Swap(), {Qubit(x), Qubit(y)});
    }
    return circuit;
}

} // namespace tweedledum