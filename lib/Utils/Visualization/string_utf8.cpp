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

class Diagram {
public:
    Diagram(uint32_t num_qubits, uint32_t num_cbits, uint32_t num_cols)
        : num_rows_((2 * (num_qubits + num_cbits)) + 1)
        , num_cols_(num_cols)
        , rows_(num_rows_, std::u32string(num_cols, U' '))
    {
        for (uint32_t i = 0; i < num_qubits; ++i) {
            std::u32string& row = rows_.at((2 * i) + 1);
            std::fill(row.begin(), row.end(), U'─');
        }
        for (uint32_t i = num_qubits; i < num_qubits + num_cbits; ++i) {
            std::u32string& row = rows_.at((2 * i) + 1);
            std::fill(row.begin(), row.end(), U'═');
        }
    }

    uint32_t wire_to_row(Qubit qubit) const
    {
        return (2u * (qubit)) + 1u;
    }

    uint32_t wire_to_row(Cbit cbit) const
    {
        return rows_.size() - 2;
    }

    auto& at(uint32_t row, uint32_t col)
    {
        assert(row < num_rows_);
        assert(col < num_cols_);
        return rows_.at(row).at(col);
    }

    std::u32string& row(uint32_t row)
    {
        assert(row < num_rows_);
        return rows_.at(row);
    }

private:
    uint32_t const num_rows_;
    uint32_t const num_cols_;
    std::vector<std::u32string> rows_;
};

class DiagramOp {
public:
    DiagramOp(uint32_t num_targets, std::vector<Qubit> const& qubits, 
        std::vector<Cbit> const& cbits)
        : num_targets_(num_targets), qubits_(qubits), cbits_(cbits)
    {}

    uint32_t num_controls() const
    {
        return qubits_.size() - num_targets();
    }

    uint32_t num_targets() const
    {
        return num_targets_;
    }

    void set_cols(uint32_t left_col)
    {
        left_col_ = left_col;
        right_col_ = left_col_ + width() - 1;
    }

    virtual uint32_t width() const = 0;

    virtual void draw(Diagram& diagram) = 0;

protected:
    uint32_t left_col_;
    uint32_t right_col_;
    uint32_t num_targets_;
    std::vector<Qubit> qubits_;
    std::vector<Cbit> cbits_;
};

class Box : public DiagramOp {
public:

    Box(uint32_t num_targets, std::vector<Qubit> const& qubits, 
        std::vector<Cbit> const& cbits, std::string_view label) 
        : DiagramOp(num_targets, qubits, cbits), label(label)
    { }

    virtual uint32_t width() const override
    {
        return label.size() + 2u + (num_controls() > 0);
    }

    virtual void draw(Diagram& diagram) override
    {
        auto const [min, max] =
            std::minmax_element(qubits_.begin(), qubits_.end());
        set_vertical_positions(diagram, *min, *max);
        draw_box(diagram);
        draw_targets(diagram);
        draw_controls(diagram);
        draw_cbits(diagram);
        draw_label(diagram);
    }

protected:
    void set_vertical_positions(Diagram const& diagram, Qubit top, Qubit bot) 
    {
        box_top = diagram.wire_to_row(top) - 1u;
        box_bot = diagram.wire_to_row(bot) + 1u;
        box_mid = (box_top + box_bot) / 2u;
    }

    void draw_box(Diagram& diagram) const
    {
        // Draw top and bottom
        for (uint32_t i = left_col_ + 1; i < right_col_; ++i) {
            diagram.at(box_top, i) = U'─';
            diagram.at(box_bot, i) = U'─';
        }
        // Draw sides
        for (uint32_t i = box_top + 1; i < box_bot; ++i) {
            auto left = diagram.row(i).begin() + left_col_;
            auto right = diagram.row(i).begin() + right_col_;
            *left = U'│';
            *right = U'│';
            std::fill(left + 1, right, U' ');
        }
        // Draw corners
        merge_chars(diagram.at(box_top, left_col_), U'╭');
        merge_chars(diagram.at(box_bot, left_col_), U'╰');
        merge_chars(diagram.at(box_top, right_col_), U'╮');
        merge_chars(diagram.at(box_bot, right_col_), U'╯');
    }

    void draw_targets(Diagram& diagram) const
    {
        std::for_each(qubits_.begin(), qubits_.begin() + num_targets(),
        [&](Qubit qubit) {
            uint32_t const row = diagram.wire_to_row(qubit);
            diagram.at(row, left_col_) = U'┤';
            diagram.at(row, right_col_) = U'├';
        });
    }

    virtual void draw_controls(Diagram& diagram) const
    {
        std::for_each(qubits_.begin() + num_targets(), qubits_.end(),
        [&](Qubit qubit) {
            uint32_t const row = diagram.wire_to_row(qubit);
            bool is_complemented = qubit.polarity() == Qubit::Polarity::negative;
            diagram.at(row, left_col_) = U'┤';
            diagram.at(row, left_col_ + 1) = is_complemented ? U'◯' : U'●';
            diagram.at(row, right_col_) = U'├';
        });
    }

    virtual void draw_cbits(Diagram& diagram) const
    {
        uint32_t const mid_col = (left_col_ + right_col_) / 2;
        for (Cbit const cbit : cbits_) {
            uint32_t const row = diagram.wire_to_row(cbit);
            std::u32string const wire_label = fmt::format(U"{:>2}", cbit.uid());
            bool is_complemented = cbit.polarity() == Cbit::Polarity::negative;
            diagram.at(row, mid_col) = is_complemented ? U'◯' : U'●';
            for (uint32_t i = box_bot + 1; i < row; ++i) {
                merge_chars(diagram.at(i, mid_col), U'║');
            }
            diagram.at(box_bot, mid_col) = U'╥';
            std::copy(wire_label.begin(), wire_label.end(), 
                      diagram.row(row + 1).begin() + (mid_col - 1));
        }
    }

    virtual void draw_label(Diagram& diagram) const
    {
        uint32_t const label_start = left_col_ + 1 + (num_controls() > 0);
        std::copy(label.begin(), label.end(), 
                  diagram.row(box_mid).begin() + label_start);
    }

    uint32_t box_top;
    uint32_t box_mid;
    uint32_t box_bot;
    std::string label;
};

class ControlledBox : public Box {
public:
    ControlledBox(uint32_t num_targets, std::vector<Qubit> const& qubits, 
        std::vector<Cbit> const& cbits, std::string_view label) 
        : Box(num_targets, qubits, cbits, label)
    { }

    virtual uint32_t width() const override
    {
        return label.size() + 2;
    }

    virtual void draw(Diagram& diagram) override
    {
        auto const [min, max] =
            std::minmax_element(qubits_.begin(), qubits_.begin() + num_targets());
        set_vertical_positions(diagram, *min, *max);
        draw_box(diagram);
        draw_targets(diagram);
        draw_controls(diagram);
        draw_cbits(diagram);
        draw_label(diagram);
    }

private:
    virtual void draw_controls(Diagram& diagram) const override
    {
        uint32_t const mid_col = (left_col_ + right_col_) / 2;
        std::for_each(qubits_.begin() + num_targets(), qubits_.end(),
        [&](Qubit qubit) {
            uint32_t const row = diagram.wire_to_row(qubit);
            bool is_complemented = qubit.polarity() == Qubit::Polarity::negative;
            diagram.at(row, mid_col) = is_complemented ? U'◯' : U'●';
            if (row < box_top) {
                for (uint32_t i = row + 1; i < box_top; ++i) {
                    merge_chars(diagram.at(i, mid_col), U'│');
                }
                diagram.at(box_top, mid_col) = U'┴';
            } else {
                for (uint32_t i = box_top + 1; i < row; ++i) {
                    merge_chars(diagram.at(i, mid_col), U'│');
                }
                diagram.at(box_bot, mid_col) = U'┬';
            }
        });
    }

    virtual void draw_label(Diagram& diagram) const override
    {
        std::copy(label.begin(), label.end(),
                  diagram.row(box_mid).begin() + left_col_ + 1);
    }
};

class MeasureBox : public Box {
public:
    MeasureBox(std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits) 
        : Box(1u, qubits, cbits, "m")
    { }

private:
    virtual void draw_cbits(Diagram& diagram) const
    {
        uint32_t const mid_col = (left_col_ + right_col_) / 2;
        uint32_t const row = diagram.wire_to_row(cbits_.back());
        diagram.at(box_bot, mid_col) = U'╥';
        for (uint32_t i = box_bot + 1; i < row; ++i) {
            merge_chars(diagram.at(i, mid_col), U'║');
        }
        diagram.at(row, left_col_ + 1) = U'V';
        std::u32string const wire_label = fmt::format(U"{:>2}", cbits_.at(0).uid());
        std::copy(wire_label.begin(), wire_label.end(), 
                  diagram.row(row + 1).begin() + (mid_col - 1));
    }
};

class DiagramSwap : public DiagramOp {
public:
    DiagramSwap(std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits)
        : DiagramOp(2u, qubits, cbits)
    {}

    virtual uint32_t width() const override
    {
        return 3u;
    }

    virtual void draw(Diagram& diagram) override
    {
        uint32_t const mid_col = left_col_ + 1;
        uint32_t const target_row0 = diagram.wire_to_row(qubits_.at(0));
        uint32_t const target_row1 = diagram.wire_to_row(qubits_.at(1));
        diagram.at(target_row0, mid_col) = U'╳';
        for (uint32_t i = target_row0 + 1; i < target_row1; ++i) {
            merge_chars(diagram.at(i, mid_col), U'│');
        }
        diagram.at(target_row1, mid_col) = U'╳';
        draw_controls(diagram);
    }

private:
    void draw_controls(Diagram& diagram) const
    {
        uint32_t const mid_col = left_col_ + 1;
        uint32_t const target_row0 = diagram.wire_to_row(qubits_.at(0));
        uint32_t const target_row1 = diagram.wire_to_row(qubits_.at(1));
        std::for_each(qubits_.begin() + num_targets(), qubits_.end(),
        [&](Qubit qubit) {
            uint32_t const row = diagram.wire_to_row(qubit);
            bool is_complemented = qubit.polarity() == Qubit::Polarity::negative;
            diagram.at(row, mid_col) = is_complemented ? U'◯' : U'●';
            if (row < target_row0) {
                for (uint32_t i = row + 1; i < target_row0; ++i) {
                    merge_chars(diagram.at(i, mid_col), U'│');
                }
            } else {
                for (uint32_t i = target_row1 + 1; i < row; ++i) {
                    merge_chars(diagram.at(i, mid_col), U'│');
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
    uint32_t const num_dwire = circuit.num_qubits() + (circuit.num_cbits() > 0);
    uint32_t curr_dwire = 0;
    std::size_t prefix_size = 0;
    std::vector<std::string> prefix((2 * num_dwire) + 1, "");
    circuit.foreach_qubit([&](std::string_view name) {
        prefix.at((2 * curr_dwire++) + 1) = fmt::format("{} : ", name);
        prefix_size = std::max(prefix_size, name.size() + 3);
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
        std::vector<Qubit> qubits;
        std::vector<Cbit> cbits;
        inst.foreach_target([&](Qubit target) {
            qubits.push_back(target);
        });
        std::sort(qubits.begin(), qubits.end());
        uint32_t const min_target = qubits.front();
        uint32_t const max_target = qubits.back();
        uint32_t min_dwire = min_target;
        uint32_t max_dwire = max_target;
        bool overlap = false;
        inst.foreach_control([&](Qubit control) {
            qubits.push_back(control);
            if (control > min_target && control < max_target) {
                overlap = true;
            }
            min_dwire = std::min(control.uid(), min_dwire);
            max_dwire = std::max(control.uid(), max_dwire);
        });
        if (inst.num_cbits() > 0) {
            inst.foreach_cbit([&](Cbit cbit) {
                cbits.push_back(cbit);
            });
            max_dwire = wire_layer.size() - 1;
        }
        uint32_t padding = 1;
        std::string_view name = inst.name();
        std::string label = fmt::format("{: ^{}}", name, name.size() + (2 * padding));
        std::unique_ptr<DiagramOp> shape = nullptr;
        if (overlap) {
            shape = std::make_unique<Box>(inst.num_targets(), qubits, cbits, label);
        } else if (inst.is_a<Op::Swap>()) {
            shape = std::make_unique<DiagramSwap>(qubits, cbits);
        } else if (inst.is_a<Op::Measure>()) {
            shape = std::make_unique<MeasureBox>(qubits, cbits);
        } else {
            shape = std::make_unique<ControlledBox>(inst.num_targets(), qubits, cbits, label);
        }
        
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
            auto& box = boxes.at(ref);
            box->set_cols(curr_width + (layer_width.at(layer) - box->width())/2u);
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
    uint32_t const num_rows = (2 * num_dwire) + 1;
    Diagram diagram(circuit.num_qubits(), (circuit.num_cbits() > 0), curr_width);
    // Draw boxes
    for (auto const& box : boxes) {
        box->draw(diagram);
    }

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    std::string str;
    str.reserve(curr_width * num_rows * 4);

    uint32_t start = 0;
    for (uint32_t i = 0; i < cutting_point.size(); ++i) {
        if (i > 0) {
            str += fmt::format("\n{:#^{}}\n\n", "", max_rows);
        }
        for (uint32_t row = 0; row < num_rows; ++row) {
            auto being = diagram.row(row).data() + start;
            auto end = diagram.row(row).data() + cutting_point.at(i);
            if (i == 0) {
                str += fmt::format("{: >{}}", prefix.at(row), prefix_size);
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