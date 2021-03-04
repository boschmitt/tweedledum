/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Utils/Visualization/string_utf8.h"

#include <algorithm>
#include <codecvt>
#include <limits>
#include <utility>

namespace tweedledum {

// Width is given by the label!
struct Box {
    uint32_t left_x;
    uint32_t right_x;
    uint32_t top_y;
    uint32_t bot_y;
    bool overlap;
    std::string_view label;
    std::vector<uint32_t> targets;
    std::vector<std::pair<uint32_t, uint32_t>> controls;
    std::vector<WireRef> cbits;
};

inline void merge_chars(char32_t& c0, char32_t c1)
{ 
    if (c0 == U' ') {
        c0 = c1;
        return;
    }
    if (c1 == U'│') {
        switch (c0) {
        case U'◯':
        case U'●':
        case U'┼':
        case U'╪':
            return;
                    
        case U'─':
            c0 = U'┼';
            return;
                    
        case U'═':
            c0 = U'╪';
            return;

        default:
            c0 = U'│';
            return;
        }
    }

    if (c1 == U'║') {
        switch (c0) {
        case U'─':
            c0 = U'╫';
            return;
                    
        case U'═':
            c0 = U'╬';
            return;

        default:
            c0 = U'║';
            return;
        }
    }
    if (c0 > c1) {
        std::swap (c0, c1);
    }
    if (c0 == U'─') {
        switch (c1) {
        case U'┌':
        case U'┐':
            c0 = U'┬';
            return;

        case U'└':
        case U'┘':
            c0 = U'┴';
            return;

        default:
            break;
        }
    }
    if (c0 == U'┌' && c1 == U'└') {
        c0 = U'├';
        return;
    } else if (c0 == U'┐' && c1 == U'┘') {
        c0 = U'┤';
        return;
    }
}

std::string to_string_utf8(Circuit const& circuit)
{
    constexpr uint32_t padding = 4u;
    using Layer = std::vector<InstRef>;
    
    if (circuit.num_wires() == 0) {
        return "";
    }
    // Create a mapping between circuit wires and diagram wires
    uint32_t const num_dwire = circuit.num_qubits() + (circuit.num_cbits() > 0);
    std::vector<uint32_t> wire_to_dwire(circuit.num_wires(), num_dwire - 1);
    uint32_t curr_dwire = 0;
    circuit.foreach_wire([&](WireRef wire) {
        if (wire.kind() == Wire::Kind::classical) {
            return;
        }
        wire_to_dwire.at(wire) = curr_dwire++;
    });
    // Separete the intructions in layers.  Each layer must contain gates that
    // can be drawn in the same diagram layer.  For example, if I have a
    // CX(0, 2) and a X(1), then I cannot draw those gates on the same layer
    // in the circuit diagram
    std::vector<Box> boxes(circuit.size());
    std::vector<Layer> layers;
    std::vector<uint32_t> layer_width;
    std::vector<int> wire_layer(num_dwire, -1);
    circuit.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        Box& box = boxes.at(ref);
        box.label = inst.name();
        inst.foreach_target([&](WireRef wire) {
            uint32_t const dwire = wire_to_dwire.at(wire);
            box.targets.push_back(dwire);
        });
        std::sort(box.targets.begin(), box.targets.end());
        uint32_t const min_target = box.targets.front();
        uint32_t const max_target = box.targets.back();
        uint32_t min = min_target;
        uint32_t max = max_target;
        box.overlap = (inst.num_cbits() > 0) && (inst.num_controls() > 0);
        inst.foreach_control([&](WireRef wire) {
            auto const [control, _] = box.controls.emplace_back(wire_to_dwire.at(wire), wire.is_complemented());
            if (control > min_target && control < max_target) {
                box.overlap = true;
            }
            if (control > max) {
                max = control;
            } else if (control < min) {
                min = control;
            }
        });
        inst.foreach_cbit([&](WireRef wire) {
            box.cbits.push_back(wire);
        });
        box.top_y = 2 * (min_target);
        box.bot_y = 2 * (max_target) + 2;
        if (box.overlap) {
            box.top_y = 2 * (min);
            box.bot_y = 2 * (max) + 2;
        }

        int layer = -1;
        for (uint32_t i = min; i <= max; ++i) {
            layer = std::max(layer, wire_layer.at(i));
        }
        layer += 1;
        if (layer == layers.size()) {
            layers.emplace_back();
            layer_width.push_back(0);
        }
        layers.at(layer).emplace_back(ref);
        for (uint32_t i = min; i <= max; ++i) {
            wire_layer.at(i) = layer;
        }
        uint32_t const width = inst.name().size() + padding + (2 * box.overlap);
        layer_width.at(layer) = std::max(layer_width.at(layer), width);
        // TODO: Edge cases: Swap, Measure
    });

    // Compute diagram width
    uint32_t curr_width = 0u;
    for (uint32_t layer = 0u; layer < layers.size(); ++layer) {
        for (InstRef const ref : layers.at(layer)) {
            Box& box = boxes.at(ref);
            box.left_x = curr_width + (layer_width.at(layer) - (box.label.size() + padding))/2u;
            box.right_x = box.left_x + box.label.size() + padding + box.overlap - 1;
        }
        curr_width += layer_width.at(layer);
    }

    // Initialize the diagram with the wires 
    uint32_t const num_lines = (2 * num_dwire) + 1;
    std::vector<std::u32string> lines(num_lines, std::u32string(curr_width, U' '));
    for (uint32_t i = 0; i < num_dwire - 1; ++i) {
        std::u32string& line = lines.at((2 * i) + 1);
        std::fill(line.begin(), line.end(), U'─');
    }
    if (circuit.num_cbits() > 0) {
        std::u32string& line = lines.at((2 * (num_dwire - 1)) + 1);
        std::fill(line.begin(), line.end(), U'═');
    } else {
        std::u32string& line = lines.at((2 * (num_dwire - 1)) + 1);
        std::fill(line.begin(), line.end(), U'─');
    }

    // Draw boxes
    for (Box const& box : boxes) {
        // Draw box
        for (uint32_t i = box.left_x + 1; i < box.right_x; ++i) {
            lines.at(box.top_y).at(i) = U'─';
            lines.at(box.bot_y).at(i) = U'─';
        }
        for (uint32_t i = box.top_y + 1; i < box.bot_y; ++i) {
            auto left = lines.at(i).begin() + box.left_x;
            auto right = lines.at(i).begin() + box.right_x;
            *left = U'│';
            *right = U'│';
            std::fill(++left, right, U' ');
        }
        merge_chars(lines.at(box.top_y).at(box.left_x), U'┌');
        merge_chars(lines.at(box.bot_y).at(box.left_x), U'└');
        merge_chars(lines.at(box.top_y).at(box.right_x), U'┐');
        merge_chars(lines.at(box.bot_y).at(box.right_x), U'┘');
        // Connect targets
        for (auto const wire : box.targets) {
            uint32_t const line = (2u * wire) + 1;
            lines.at(line).at(box.left_x) = U'┤';
            lines.at(line).at(box.right_x) = U'├';
        }
        // Connect controls
        uint32_t const mid_x = box.left_x + ((box.label.size() + padding) / 2u);
        if (box.overlap) {
            for (auto const& [wire, is_complemented] : box.controls) {
                uint32_t const line = (2u * wire) + 1;
                lines.at(line).at(box.left_x) = U'┤';
                lines.at(line).at(box.left_x + 1) = is_complemented ? U'◯' : U'●';
                lines.at(line).at(box.right_x) = U'├';
            }
        } else {
            for (auto const& [wire, is_complemented] : box.controls) {
                uint32_t const line = (2u * wire) + 1;
                lines.at(line).at(mid_x) = is_complemented ? U'◯' : U'●';
                if (line < box.top_y) {
                    for (uint32_t i = line + 1; i < box.top_y; ++i) {
                        merge_chars(lines.at(i).at(mid_x), U'│');
                    }
                    lines.at(box.top_y).at(mid_x) = U'┴';
                } else {
                    for (uint32_t i = box.top_y + 1; i < line; ++i) {
                        merge_chars(lines.at(i).at(mid_x), U'│');
                    }
                    lines.at(box.bot_y).at(mid_x) = U'┬';
                }
            }
        }
        // Connect classical bits
        uint32_t const cbit_line = (2u * (num_dwire - 1) + 1);
        for (WireRef const wire : box.cbits) {
            std::u32string const wire_label = fmt::format(U"{:>2}", wire.uid());
            lines.at(cbit_line).at(mid_x) = wire.is_complemented() ? U'◯' : U'●';
            for (uint32_t i = box.bot_y + 1; i < cbit_line; ++i) {
                merge_chars(lines.at(i).at(mid_x), U'║');
            }
            lines.at(box.bot_y).at(mid_x) = U'╥';
            std::copy(wire_label.begin(), wire_label.end(), 
                  lines.at(cbit_line + 1).begin() + (mid_x - 1));
        }
        
        // Draw label
        uint32_t const mid_y = (box.top_y + box.bot_y)/2;
        std::copy(box.label.begin(), box.label.end(), 
                  lines.at(mid_y).begin() + box.left_x + 2 + box.overlap);
    }

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    std::string str;
    str.reserve(curr_width * num_lines * 4);
    for (auto const& line : lines) {
        str += conv.to_bytes(line) + '\n';
    }
    return str;
}

void print(Circuit const& circuit)
{
    fmt::print("{}", to_string_utf8(circuit));
}

} // namespace tweedledum