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

struct DiagramShape {
    uint32_t left_col;
    uint32_t right_col;
    uint32_t top_row;
    uint32_t bot_row;
    std::string_view label;
    std::vector<uint32_t> target_rows;
    std::vector<uint32_t> control_rows;
    std::vector<WireRef> cbits;

    virtual uint32_t width() const = 0;
    virtual void draw(std::vector<std::u32string>& lines) const = 0;

    void draw_cbits(std::vector<std::u32string>& lines) const
    {
        uint32_t const mid_col = (left_col + right_col) / 2;
        uint32_t const cbit_line = lines.size() - 2;
        for (WireRef const wire : cbits) {
            std::u32string const wire_label = fmt::format(U"{:>2}", wire.uid());
            lines.at(cbit_line).at(mid_col) = wire.is_complemented() ? U'◯' : U'●';
            for (uint32_t i = bot_row + 1; i < cbit_line; ++i) {
                merge_chars(lines.at(i).at(mid_col), U'║');
            }
            lines.at(bot_row).at(mid_col) = U'╥';
            std::copy(wire_label.begin(), wire_label.end(), 
                      lines.at(cbit_line + 1).begin() + (mid_col - 1));
        }
    }
};

struct Box : public DiagramShape {
    virtual uint32_t width() const override
    {
        // The width is the label legnth + two box chars + two spaces + 
        return label.size() + 4 + (!control_rows.empty());
    }

    virtual void draw(std::vector<std::u32string>& lines) const override
    {
        draw_box(lines);
        draw_targets(lines);
        draw_controls(lines);
        if (!cbits.empty()) {
            draw_cbits(lines);
        }
        draw_label(lines);
    }

    void draw_box(std::vector<std::u32string>& lines) const
    {
        for (uint32_t i = left_col + 1; i < right_col; ++i) {
            lines.at(top_row).at(i) = U'─';
            lines.at(bot_row).at(i) = U'─';
        }
        for (uint32_t i = top_row + 1; i < bot_row; ++i) {
            auto left = lines.at(i).begin() + left_col;
            auto right = lines.at(i).begin() + right_col;
            *left = U'│';
            *right = U'│';
            std::fill(++left, right, U' ');
        }
        merge_chars(lines.at(top_row).at(left_col), U'╭');
        merge_chars(lines.at(bot_row).at(left_col), U'╰');
        merge_chars(lines.at(top_row).at(right_col), U'╮');
        merge_chars(lines.at(bot_row).at(right_col), U'╯');
    }

    void draw_targets(std::vector<std::u32string>& lines) const
    {
        for (uint32_t const line : target_rows) {
            lines.at(line).at(left_col) = U'┤';
            lines.at(line).at(right_col) = U'├';
        }
    }

    virtual void draw_controls(std::vector<std::u32string>& lines) const
    {
        for (uint32_t const control : control_rows) {
            uint32_t line = (control >> 1);
            lines.at(line).at(left_col) = U'┤';
            lines.at(line).at(left_col + 1) = control & 1 ? U'◯' : U'●';
            lines.at(line).at(right_col) = U'├';
        }
    }

    virtual void draw_label(std::vector<std::u32string>& lines) const
    {
        // Draw label
        uint32_t const mid_row = (top_row + bot_row)/2;
        std::copy(label.begin(), label.end(), 
                  lines.at(mid_row).begin() + left_col + 2 + (!control_rows.empty()));
    }
};

struct ControlledBox : public Box {
    virtual uint32_t width() const override
    {
        // The width is the label legnth + two box chars + two spaces + 
        return label.size() + 4;
    }

    virtual void draw_controls(std::vector<std::u32string>& lines) const override
    {
        uint32_t const mid_col = (left_col + right_col) / 2;
        for (uint32_t const control: control_rows) {
            uint32_t const line = (control >> 1);
            lines.at(line).at(mid_col) = control & 1 ? U'◯' : U'●';
            if (line < top_row) {
                for (uint32_t i = line + 1; i < top_row; ++i) {
                    merge_chars(lines.at(i).at(mid_col), U'│');
                }
                lines.at(top_row).at(mid_col) = U'┴';
            } else {
                for (uint32_t i = top_row + 1; i < line; ++i) {
                    merge_chars(lines.at(i).at(mid_col), U'│');
                }
                lines.at(bot_row).at(mid_col) = U'┬';
            }
        }
    }

    virtual void draw_label(std::vector<std::u32string>& lines) const override
    {
        // Draw label
        uint32_t const mid_row = (top_row + bot_row)/2;
        std::copy(label.begin(), label.end(),
                  lines.at(mid_row).begin() + left_col + 2);
    }
};

struct DiagramSwap : public DiagramShape {
    virtual uint32_t width() const override
    {
        return 3u;
    }

    virtual void draw(std::vector<std::u32string>& lines) const override
    {
        uint32_t const mid_col = left_col + 1;
        lines.at(top_row + 1).at(mid_col) = U'╳';
        for (uint32_t i = top_row + 2; i < bot_row - 1; ++i) {
            merge_chars(lines.at(i).at(mid_col), U'│');
        }
        lines.at(bot_row - 1).at(mid_col) = U'╳';
        draw_controls(lines);
        if (!cbits.empty()) {
            draw_cbits(lines);
        }
    }

    void draw_controls(std::vector<std::u32string>& lines) const
    {
        uint32_t const mid_col = left_col + 1;
        for (uint32_t const control : control_rows) {
            uint32_t const line = (control >> 1);
            lines.at(line).at(mid_col) = control & 1 ? U'◯' : U'●';
            if (line <= top_row) {
                for (uint32_t i = line + 1; i <= top_row; ++i) {
                    merge_chars(lines.at(i).at(mid_col), U'│');
                }
            } else {
                for (uint32_t i = bot_row; i < line; ++i) {
                    merge_chars(lines.at(i).at(mid_col), U'│');
                }
            }
        }
    }
};

struct DiagramMeasure : public DiagramShape {
    virtual uint32_t width() const override
    {
        return 3u;
    }

    virtual void draw(std::vector<std::u32string>& lines) const override
    {
        uint32_t const mid_col = left_col + 1;
        merge_chars(lines.at(top_row).at(left_col), U'╭');
        merge_chars(lines.at(bot_row).at(left_col), U'╰');
        merge_chars(lines.at(top_row).at(right_col), U'╮');
        merge_chars(lines.at(bot_row).at(right_col), U'╯');
        lines.at(target_rows.at(0)).at(left_col) = U'┤';
        lines.at(target_rows.at(0)).at(mid_col) = U'm';
        lines.at(target_rows.at(0)).at(right_col) = U'├';
        merge_chars(lines.at(top_row).at(mid_col), U'─');
        lines.at(bot_row).at(mid_col) = U'╥';

        uint32_t const cbit_line = lines.size() - 2;
        for (uint32_t i = bot_row + 1; i < cbit_line; ++i) {
            merge_chars(lines.at(i).at(mid_col), U'║');
        }
        lines.at(cbit_line).at(left_col + 1) = U'V';
        std::u32string const wire_label = fmt::format(U"{:>2}", cbits.at(0).uid());
        std::copy(wire_label.begin(), wire_label.end(), 
                  lines.at(cbit_line + 1).begin() + (mid_col - 1));
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
    std::vector<std::unique_ptr<DiagramShape>> boxes;
    std::vector<Layer> layers;
    std::vector<uint32_t> layer_width;
    std::vector<int> wire_layer(num_dwire, -1);
    circuit.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        std::vector<uint32_t> target_rows;
        inst.foreach_target([&](WireRef wire) {
            target_rows.push_back((2 * wire_to_dwire.at(wire)) + 1);
        });
        std::sort(target_rows.begin(), target_rows.end());
        uint32_t const min_target = ((target_rows.front() - 1) >> 1);
        uint32_t const max_target = ((target_rows.back() - 1) >> 1);
        uint32_t min = min_target;
        uint32_t max = max_target;

        bool overlap = (inst.num_cbits() > 0) && (inst.num_controls() > 0);
        std::vector<uint32_t> control_rows;
        inst.foreach_control([&](WireRef wire) {
            uint32_t const dwire = wire_to_dwire.at(wire);
            uint32_t const line = (2 * wire_to_dwire.at(wire)) + 1;
            control_rows.push_back((line << 1) | wire.is_complemented());
            if (dwire > min_target && dwire < max_target) {
                overlap = true;
            }
            if (dwire > max) {
                max = dwire;
            } else if (dwire < min) {
                min = dwire;
            }
        });
        std::vector<WireRef> cbits;
        inst.foreach_cbit([&](WireRef wire) {
            cbits.push_back(wire);
        });

        std::unique_ptr<DiagramShape> shape = nullptr;
        if (overlap) {
            shape = std::make_unique<Box>();
        } else if (inst.is_a<Op::Swap>()) {
            shape = std::make_unique<DiagramSwap>();
        } else if (inst.is_a<Op::Measure>()) {
            shape = std::make_unique<DiagramMeasure>();
        } else {
            shape = std::make_unique<ControlledBox>();
        }
        shape->label = inst.name();
        shape->cbits = cbits;
        shape->control_rows = control_rows;
        shape->target_rows = target_rows;
        shape->top_row = (2 * min_target);
        shape->bot_row = (2 * max_target) + 2;
        if (overlap) {
            shape->top_row = (2 * min);
            shape->bot_row = (2 * max ) + 2;
        }

        int layer = -1;
        for (uint32_t i = min; i <= max; ++i) {
            layer = std::max(layer, wire_layer.at(i));
        }
        if (!cbits.empty()) {
            layer = std::max(layer, wire_layer.back());
            wire_layer.back() = layer + 1;
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