/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Operators/Standard/Measure.h"
#include "tweedledum/Operators/Standard/Swap.h"
#include "tweedledum/Utils/Visualization/string_utf8.h"

#include <algorithm>
#include <codecvt>
#include <limits>
#include <utility>

namespace tweedledum {

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
        case U'╭':
        case U'╮':
            c0 = U'┬';
            return;

        case U'╰':
        case U'╯':
            c0 = U'┴';
            return;

        default:
            break;
        }
    }
    if (c0 == U'╭' && c1 == U'╰') {
        c0 = U'├';
        return;
    } else if (c0 == U'╮' && c1 == U'╯') {
        c0 = U'┤';
        return;
    }
}

struct DiagramOp {
    uint32_t left_col;
    uint32_t right_col;
    uint32_t num_targets;
    std::vector<uint32_t> wires;
    std::vector<WireRef> cbits;

    DiagramOp(std::vector<uint32_t> const& wires, uint32_t num_targets)
        : num_targets(num_targets), wires(wires)
    {}

    uint32_t num_controls() const
    {
        return wires.size() - num_targets;
    }

    virtual uint32_t width() const = 0;

    virtual void draw(std::vector<std::u32string>& lines) const = 0;
};

struct Box : public DiagramOp {
    uint32_t box_top;
    uint32_t box_mid;
    uint32_t box_bot;
    std::string label;

    Box(std::vector<uint32_t> const& wires, uint32_t num_targets, std::string_view label) 
        : DiagramOp(wires, num_targets), label(label)
    {
        auto const [min, max] = std::minmax_element(wires.begin(), wires.end());
        box_top = (2 * (*min >> 1));
        box_bot = (2 * (*max >> 1)) + 2;
        box_mid = (box_top + box_bot) / 2;
    }

    virtual uint32_t width() const override
    {
        return label.size() + 2 + (num_controls() > 0);
    }

    virtual void draw(std::vector<std::u32string>& lines) const override
    {
        draw_box(lines);
        draw_targets(lines);
        draw_controls(lines);
        draw_cbits(lines);
        draw_label(lines);
    }

    void draw_box(std::vector<std::u32string>& lines) const
    {
        // Draw top and bottom
        for (uint32_t i = left_col + 1; i < right_col; ++i) {
            lines.at(box_top).at(i) = U'─';
            lines.at(box_bot).at(i) = U'─';
        }
        // Draw sides
        for (uint32_t i = box_top + 1; i < box_bot; ++i) {
            auto left = lines.at(i).begin() + left_col;
            auto right = lines.at(i).begin() + right_col;
            *left = U'│';
            *right = U'│';
            std::fill(++left, right, U' ');
        }
        // Draw corners
        merge_chars(lines.at(box_top).at(left_col), U'╭');
        merge_chars(lines.at(box_bot).at(left_col), U'╰');
        merge_chars(lines.at(box_top).at(right_col), U'╮');
        merge_chars(lines.at(box_bot).at(right_col), U'╯');
    }

    void draw_targets(std::vector<std::u32string>& lines) const
    {
        std::for_each(wires.begin(), wires.begin() + num_targets,
        [&](uint32_t wire) {
            uint32_t const line = (2 * (wire >> 1)) + 1;
            lines.at(line).at(left_col) = U'┤';
            lines.at(line).at(right_col) = U'├';
        });
    }

    virtual void draw_controls(std::vector<std::u32string>& lines) const
    {
        std::for_each(wires.begin() + num_targets, wires.end(),
        [&](uint32_t wire) {
            uint32_t const line = (2 * (wire >> 1)) + 1;
            lines.at(line).at(left_col) = U'┤';
            lines.at(line).at(left_col + 1) = wire & 1 ? U'◯' : U'●';
            lines.at(line).at(right_col) = U'├';
        });
    }

    virtual void draw_cbits(std::vector<std::u32string>& lines) const
    {
        uint32_t const mid_col = (left_col + right_col) / 2;
        uint32_t const cbit_line = lines.size() - 2;
        for (WireRef const wire : cbits) {
            std::u32string const wire_label = fmt::format(U"{:>2}", wire.uid());
            lines.at(cbit_line).at(mid_col) = wire.is_complemented() ? U'◯' : U'●';
            for (uint32_t i = box_bot + 1; i < cbit_line; ++i) {
                merge_chars(lines.at(i).at(mid_col), U'║');
            }
            lines.at(box_bot).at(mid_col) = U'╥';
            std::copy(wire_label.begin(), wire_label.end(), 
                      lines.at(cbit_line + 1).begin() + (mid_col - 1));
        }
    }

    virtual void draw_label(std::vector<std::u32string>& lines) const
    {
        uint32_t const label_start = left_col + 1 + (num_controls() > 0);
        std::copy(label.begin(), label.end(), 
                  lines.at(box_mid).begin() + label_start);
    }
};

struct ControlledBox : public Box {

    ControlledBox(std::vector<uint32_t> const& wires, uint32_t num_targets, std::string_view label) 
        : Box(wires, num_targets, label)
    {
        auto const [min, max] = std::minmax_element(wires.begin(), wires.begin() + num_targets);
        box_top = (2 * (*min >> 1));
        box_bot = (2 * (*max >> 1)) + 2;
        box_mid = (box_top + box_bot) / 2;
    }

    virtual uint32_t width() const override
    {
        return label.size() + 2;
    }

    virtual void draw_controls(std::vector<std::u32string>& lines) const override
    {
        uint32_t const mid_col = (left_col + right_col) / 2;
        std::for_each(wires.begin() + num_targets, wires.end(),
        [&](uint32_t wire) {
            uint32_t const line = (2 * (wire >> 1)) + 1;
            lines.at(line).at(mid_col) = wire & 1 ? U'◯' : U'●';
            if (line < box_top) {
                for (uint32_t i = line + 1; i < box_top; ++i) {
                    merge_chars(lines.at(i).at(mid_col), U'│');
                }
                lines.at(box_top).at(mid_col) = U'┴';
            } else {
                for (uint32_t i = box_top + 1; i < line; ++i) {
                    merge_chars(lines.at(i).at(mid_col), U'│');
                }
                lines.at(box_bot).at(mid_col) = U'┬';
            }
        });
    }

    virtual void draw_label(std::vector<std::u32string>& lines) const override
    {
        std::copy(label.begin(), label.end(),
                  lines.at(box_mid).begin() + left_col + 1);
    }
};

struct MeasureBox : public Box {
    MeasureBox(std::vector<uint32_t> const& wires) 
        : Box(wires, 1, "m")
    { }

    virtual void draw_cbits(std::vector<std::u32string>& lines) const
    {
        uint32_t const mid_col = (left_col + right_col) / 2;
        uint32_t const cbit_line = lines.size() - 2;
        lines.at(box_bot).at(mid_col) = U'╥';
        for (uint32_t i = box_bot + 1; i < cbit_line; ++i) {
            merge_chars(lines.at(i).at(mid_col), U'║');
        }
        lines.at(cbit_line).at(left_col + 1) = U'V';
        std::u32string const wire_label = fmt::format(U"{:>2}", cbits.at(0).uid());
        std::copy(wire_label.begin(), wire_label.end(), 
                  lines.at(cbit_line + 1).begin() + (mid_col - 1));
    }
};

struct DiagramSwap : public DiagramOp {

    DiagramSwap(std::vector<uint32_t> const& wires, uint32_t num_targets)
        : DiagramOp(wires, num_targets)
    {}
    virtual uint32_t width() const override
    {
        return 3u;
    }

    virtual void draw(std::vector<std::u32string>& lines) const override
    {
        uint32_t const mid_col = left_col + 1;
        uint32_t const target_row0 = (2 * (wires.at(0) >> 1)) + 1;
        uint32_t const target_row1 = (2 * (wires.at(1) >> 1)) + 1;
        lines.at(target_row0).at(mid_col) = U'╳';
        for (uint32_t i = target_row0 + 1; i < target_row1; ++i) {
            merge_chars(lines.at(i).at(mid_col), U'│');
        }
        lines.at(target_row1).at(mid_col) = U'╳';
        draw_controls(lines);
    }

    void draw_controls(std::vector<std::u32string>& lines) const
    {
        uint32_t const mid_col = left_col + 1;
        uint32_t const target_row0 = (2 * (wires.at(0) >> 1)) + 1;
        uint32_t const target_row1 = (2 * (wires.at(1) >> 1)) + 1;
        std::for_each(wires.begin() + num_targets, wires.end(),
        [&](uint32_t wire) {
            uint32_t const line = (2 * (wire >> 1)) + 1;
            lines.at(line).at(mid_col) = wire & 1 ? U'◯' : U'●';
            if (line < target_row0) {
                for (uint32_t i = line + 1; i < target_row0; ++i) {
                    merge_chars(lines.at(i).at(mid_col), U'│');
                }
            } else {
                for (uint32_t i = target_row1 + 1; i < line; ++i) {
                    merge_chars(lines.at(i).at(mid_col), U'│');
                }
            }
        });
    }
};

std::string to_string_utf8(Circuit const& circuit, uint32_t const max_rows)
{
    using Layer = std::vector<InstRef>;
    if (circuit.num_wires() == 0) {
        return "";
    }
    // Create a mapping between circuit wires and diagram wires
    uint32_t const num_dwire = circuit.num_qubits() + (circuit.num_cbits() > 0);
    std::vector<uint32_t> wire_to_dwire(circuit.num_wires(), num_dwire - 1);
    uint32_t curr_dwire = 0;
    std::size_t prefix_size = 0;
    std::vector<std::string> prefix((2 * num_dwire) + 1, "");
    circuit.foreach_wire([&](WireRef wref, Wire const& wire) {
        if (wref.kind() == Wire::Kind::classical) {
            return;
        }
        wire_to_dwire.at(wref) = curr_dwire;
        prefix.at((2 * curr_dwire++) + 1) = wire.name + " : ";
        prefix_size = std::max(prefix_size, wire.name.size() + 3);
    });
    // Separete the intructions in layers.  Each layer must contain gates that
    // can be drawn in the same diagram layer.  For example, if I have a
    // CX(0, 2) and a X(1), then I cannot draw those gates on the same layer
    // in the circuit diagram
    std::vector<std::unique_ptr<DiagramOp>> boxes;
    std::vector<Layer> layers;
    std::vector<uint32_t> layer_width;
    std::vector<int> wire_layer(num_dwire, -1);
    circuit.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        std::vector<uint32_t> diagram_wires;
        std::vector<WireRef> cbits;
        inst.foreach_target([&](WireRef wire) {
            diagram_wires.push_back(wire_to_dwire.at(wire) << 1);
        });
        std::sort(diagram_wires.begin(), diagram_wires.end());
        uint32_t const min_target = diagram_wires.front();
        uint32_t const max_target = diagram_wires.back();
        uint32_t min_dwire = min_target;
        uint32_t max_dwire = max_target;
        bool overlap = false;
        inst.foreach_control([&](WireRef wire) {
            uint32_t const control = (wire_to_dwire.at(wire) << 1);
            diagram_wires.emplace_back(control | wire.is_complemented());
            if (control > min_target && control < max_target) {
                overlap = true;
            }
            min_dwire = std::min(control, min_dwire);
            max_dwire = std::max(control, max_dwire);
        });
        min_dwire >>= 1;
        max_dwire >>= 1;
        if (inst.num_cbits() > 0) {
            inst.foreach_cbit([&](WireRef wire) {
                cbits.push_back(wire);
            });
            max_dwire = wire_layer.size() - 1;
        }
        uint32_t padding = 1;
        std::string_view name = inst.name();
        std::string label = fmt::format("{: ^{}}", name, name.size() + (2 * padding));
        std::unique_ptr<DiagramOp> shape = nullptr;
        if (overlap) {
            shape = std::make_unique<Box>(diagram_wires, inst.num_targets(), inst.name());
        } else if (inst.is_a<Op::Swap>()) {
            shape = std::make_unique<DiagramSwap>(diagram_wires, inst.num_targets());
        } else if (inst.is_a<Op::Measure>()) {
            shape = std::make_unique<MeasureBox>(diagram_wires);
        } else {
            shape = std::make_unique<ControlledBox>(diagram_wires, inst.num_targets(), label);
        }
        shape->cbits = cbits;
        
        int layer = -1;
        for (uint32_t i = min_dwire; i <= max_dwire; ++i) {
            layer = std::max(layer, wire_layer.at(i));
        }
        layer += 1;
        if (layer == layers.size()) {
            layers.emplace_back();
            layer_width.push_back(0);
        }
        layers.at(layer).emplace_back(ref);
        for (uint32_t i = min_dwire; i <= max_dwire; ++i) {
            wire_layer.at(i) = layer;
        }
        layer_width.at(layer) = std::max(layer_width.at(layer), shape->width());
        boxes.push_back(std::move(shape));
    });

    // Compute diagram width
    uint32_t curr_width = 0u;
    uint32_t acc_width = prefix_size;
    std::vector<uint32_t> cutting_point;
    for (uint32_t layer = 0u; layer < layers.size(); ++layer) {
        for (InstRef const ref : layers.at(layer)) {
            auto const& box = boxes.at(ref);
            box->left_col = curr_width + (layer_width.at(layer) - box->width())/2u;
            box->right_col = box->left_col + box->width() - 1;
        }
        if ((acc_width + layer_width.at(layer)) >= (max_rows - 1)) {
            cutting_point.push_back(curr_width);
            acc_width = 0u;
        }
        curr_width += layer_width.at(layer);
        acc_width += layer_width.at(layer);
    }
    cutting_point.push_back(curr_width);

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
    for (auto const& box : boxes) {
        box->draw(lines);
    }

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    std::string str;
    str.reserve(curr_width * num_lines * 4);

    uint32_t start = 0;
    for (uint32_t i = 0; i < cutting_point.size(); ++i) {
        if (i > 0) {
            str += fmt::format("\n{:#^{}}\n\n", "", max_rows);
        }
        for (uint32_t line = 0; line < lines.size(); ++line) {
            auto being = lines.at(line).data() + start;
            auto end = lines.at(line).data() + cutting_point.at(i);
            if (i == 0) {
                str += fmt::format("{: >{}}", prefix.at(line), prefix_size);
            }
            str += conv.to_bytes(being, end);
            if (i + 1 < cutting_point.size()) {
                str += conv.to_bytes(U'»');
            }
            str += '\n';
        }
        start = cutting_point.at(i);
    }
    return str;
}

void print(Circuit const& circuit, uint32_t const max_rows)
{
    fmt::print("{}", to_string_utf8(circuit, max_rows));
}

} // namespace tweedledum