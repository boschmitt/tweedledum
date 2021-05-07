/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Utils/Visualization/string_utf8.h"
#include "tweedledum/Operators/Standard/Measure.h"
#include "tweedledum/Operators/Standard/Swap.h"

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
        std::swap(c0, c1);
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
    enum class Style : uint8_t
    {
        NotCompact,
        Compact,
    };

    class Wire {
    public:
        Wire(uint32_t uid, uint32_t is_complemented)
            : uid_(uid)
            , is_complemented_(is_complemented)
        {}

        uint32_t is_complemented() const
        {
            return is_complemented_;
        }

        operator uint32_t() const
        {
            return uid_;
        }

    private:
        uint32_t uid_ : 31;
        uint32_t is_complemented_ : 1;
    };

    class Operator {
    public:
        Operator(std::vector<Wire> const& wires, uint32_t num_targets,
          uint32_t num_controls)
            : wires_(wires)
            , num_targets_(num_targets)
            , num_controls_(num_controls)
        {}

        virtual ~Operator() = default;

        uint32_t num_controls() const
        {
            return num_controls_;
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
        std::vector<Wire> const wires_;
        uint32_t const num_targets_;
        uint32_t const num_controls_;
        uint32_t left_col_;
        uint32_t right_col_;
    };

    Diagram(uint32_t num_qubits, uint32_t num_cbits,
      bool inverted_qubits = false, Style style = Style::Compact)
        : style_(style)
        , inverted_qubits_(inverted_qubits)
        , num_qubits_(num_qubits)
        , num_cbits_(style_ == Style::NotCompact ? num_cbits : (num_cbits > 1))
        , height_((2 * (num_qubits_ + num_cbits_) + 1))
    {}

    uint32_t num_wires() const
    {
        return num_qubits_ + (num_cbits_ > 0);
    }

    uint32_t num_qubits() const
    {
        return num_qubits_;
    }

    uint32_t height() const
    {
        return height_;
    }

    void width(uint32_t width)
    {
        width_ = width;
        rows_.resize(height_, std::u32string(width_, U' '));
        for (uint32_t i = 0; i < num_qubits_; ++i) {
            std::u32string& row = rows_.at((2 * i) + 1);
            std::fill(row.begin(), row.end(), U'─');
        }
        for (uint32_t i = num_qubits_; i < num_qubits_ + num_cbits_; ++i) {
            std::u32string& row = rows_.at((2 * i) + 1);
            std::fill(row.begin(), row.end(), U'═');
        }
    }

    Wire to_dwire(Qubit qubit) const
    {
        uint32_t const uid =
          inverted_qubits_ ? qubit.uid() : (num_qubits_ - 1) - qubit.uid();
        return Wire(uid, (qubit.polarity() == Qubit::Polarity::negative));
    }

    Wire to_dwire(Cbit cbit) const
    {
        uint32_t const uid = cbit.uid() + num_qubits_;
        return Wire(uid, (cbit.polarity() == Cbit::Polarity::negative));
    }

    uint32_t to_row(Wire wire) const
    {
        if (wire < num_qubits_) {
            return (2u * wire) + 1u;
        } else {
            if (style_ == Style::NotCompact) {
                return (2 * wire) + 1;
            }
            return rows_.size() - 2;
        }
    }

    auto& at(uint32_t row, uint32_t col)
    {
        assert(row < height_);
        assert(col < width_);
        return rows_.at(row).at(col);
    }

    std::u32string& row(uint32_t row)
    {
        assert(row < height_);
        return rows_.at(row);
    }

private:
    Style const style_;
    bool const inverted_qubits_;
    uint32_t const num_qubits_;
    uint32_t const num_cbits_;
    uint32_t height_;
    uint32_t width_;
    std::vector<std::u32string> rows_;
};

class Box : public Diagram::Operator {
public:
    using Wire = Diagram::Wire;

    Box(std::string_view label, std::vector<Wire> const& dwires,
      uint32_t num_targets, uint32_t num_controls)
        : Operator(dwires, num_targets, num_controls)
        , label(label)
    {}

    virtual uint32_t width() const override
    {
        return label.size() + 2u + (num_controls() > 0);
    }

    virtual void draw(Diagram& diagram) override
    {
        auto const [min, max] =
          std::minmax_element(wires_.begin(), wires_.end());
        set_vertical_positions(diagram, *min, *max);
        draw_box(diagram);
        draw_targets(diagram);
        draw_controls(diagram);
        draw_cbits(diagram);
        draw_label(diagram);
    }

protected:
    void set_vertical_positions(Diagram const& diagram, Wire top, Wire bot)
    {
        box_top = diagram.to_row(top) - 1u;
        box_bot = diagram.to_row(bot) + 1u;
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
        std::for_each(
          wires_.begin(), wires_.begin() + num_targets(), [&](Wire wire) {
              uint32_t const row = diagram.to_row(wire);
              diagram.at(row, left_col_) = U'┤';
              diagram.at(row, right_col_) = U'├';
          });
    }

    virtual void draw_controls(Diagram& diagram) const
    {
        auto begin = wires_.begin() + num_targets();
        auto end = begin + num_controls();
        std::for_each(begin, end, [&](Wire wire) {
            uint32_t const row = diagram.to_row(wire);
            diagram.at(row, left_col_) = U'┤';
            diagram.at(row, left_col_ + 1) =
              wire.is_complemented() ? U'◯' : U'●';
            diagram.at(row, right_col_) = U'├';
        });
    }

    virtual void draw_cbits(Diagram& diagram) const
    {
        uint32_t const mid_col = (left_col_ + right_col_) / 2;
        uint32_t const begin = num_targets() + num_controls();
        std::for_each(wires_.begin() + begin, wires_.end(), [&](Wire wire) {
            std::u32string const wire_label =
              fmt::format(U"{:>2}", wire - diagram.num_qubits());
            uint32_t const row = diagram.to_row(wire);
            diagram.at(row, mid_col) = wire.is_complemented() ? U'◯' : U'●';
            for (uint32_t i = box_bot + 1; i < row; ++i) {
                merge_chars(diagram.at(i, mid_col), U'║');
            }
            diagram.at(box_bot, mid_col) = U'╥';
            std::copy(wire_label.begin(), wire_label.end(),
              diagram.row(row + 1).begin() + (mid_col - 1));
        });
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
    ControlledBox(std::string_view label, std::vector<Wire> const& dwires,
      uint32_t num_targets, uint32_t num_controls)
        : Box(label, dwires, num_targets, num_controls)
    {}

    virtual uint32_t width() const override
    {
        return label.size() + 2;
    }

    virtual void draw(Diagram& diagram) override
    {
        auto const [min, max] =
          std::minmax_element(wires_.begin(), wires_.begin() + num_targets());
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
        auto begin = wires_.begin() + num_targets();
        auto end = begin + num_controls();
        std::for_each(begin, end, [&](Wire wire) {
            uint32_t const row = diagram.to_row(wire);
            diagram.at(row, mid_col) = wire.is_complemented() ? U'◯' : U'●';
            if (row < box_top) {
                for (uint32_t i = row + 1; i < box_top; ++i) {
                    merge_chars(diagram.at(i, mid_col), U'│');
                }
                diagram.at(box_top, mid_col) = U'┴';
            } else {
                for (uint32_t i = box_bot + 1; i < row; ++i) {
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

class MeasureBox : public ControlledBox {
public:
    MeasureBox(std::vector<Wire> const& dwires)
        : ControlledBox("m", dwires, 1u, dwires.size() - 2)
    {}

private:
    virtual void draw_cbits(Diagram& diagram) const
    {
        uint32_t const mid_col = (left_col_ + right_col_) / 2;
        uint32_t const row = diagram.to_row(wires_.back());
        diagram.at(box_bot, mid_col) = U'╥';
        for (uint32_t i = box_bot + 1; i < row; ++i) {
            merge_chars(diagram.at(i, mid_col), U'║');
        }
        diagram.at(row, left_col_ + 1) = U'V';
        std::u32string const wire_label = fmt::format(U"{:>2}", wires_.back());
        std::copy(wire_label.begin(), wire_label.end(),
          diagram.row(row + 1).begin() + (mid_col - 1));
    }
};

class DiagramSwap : public Diagram::Operator {
public:
    using Wire = Diagram::Wire;

    DiagramSwap(std::vector<Wire> const& dwires, uint32_t num_controls)
        : Operator(dwires, 2u, num_controls)
    {}

    virtual uint32_t width() const override
    {
        return 3u;
    }

    virtual void draw(Diagram& diagram) override
    {
        uint32_t const mid_col = left_col_ + 1;
        uint32_t const target_row0 = diagram.to_row(wires_.at(0));
        uint32_t const target_row1 = diagram.to_row(wires_.at(1));
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
        uint32_t const target_row0 = diagram.to_row(wires_.at(0));
        uint32_t const target_row1 = diagram.to_row(wires_.at(1));
        auto begin = wires_.begin() + num_targets();
        auto end = begin + num_controls();
        std::for_each(begin, end, [&](Wire wire) {
            uint32_t const row = diagram.to_row(wire);
            diagram.at(row, mid_col) = wire.is_complemented() ? U'◯' : U'●';
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
    Diagram diagram(circuit.num_qubits(), circuit.num_cbits());
    std::size_t prefix_size = 0;
    std::vector<std::string> prefix(diagram.height(), "");
    circuit.foreach_qubit([&](Qubit qubit, std::string_view name) {
        uint32_t const row = diagram.to_row(diagram.to_dwire(qubit));
        prefix.at(row) = fmt::format("{} : ", name);
        prefix_size = std::max(prefix_size, name.size() + 3);
    });
    // Separete the intructions in layers.  Each layer must contain gates that
    // can be drawn in the same diagram layer.  For example, if I have a
    // CX(0, 2) and a X(1), then I cannot draw those gates on the same layer
    // in the circuit diagram
    std::vector<std::unique_ptr<Diagram::Operator>> boxes;
    std::vector<Layer> layers;
    std::vector<uint32_t> layer_width;
    std::vector<int> wire_layer(diagram.num_wires(), -1);
    circuit.foreach_instruction([&](InstRef ref, Instruction const& inst) {
        std::vector<Diagram::Wire> wires;
        inst.foreach_target(
          [&](Qubit target) { wires.push_back(diagram.to_dwire(target)); });
        std::sort(wires.begin(), wires.end());
        uint32_t const min_target = wires.front();
        uint32_t const max_target = wires.back();
        uint32_t min_dwire = min_target;
        uint32_t max_dwire = max_target;
        bool overlap = false;
        inst.foreach_control([&](Qubit qubit) {
            Diagram::Wire const control = diagram.to_dwire(qubit);
            wires.push_back(control);
            if (control > min_target && control < max_target) {
                overlap = true;
            }
            min_dwire = std::min(static_cast<uint32_t>(control), min_dwire);
            max_dwire = std::max(static_cast<uint32_t>(control), max_dwire);
        });
        if (inst.num_cbits() > 0) {
            inst.foreach_cbit(
              [&](Cbit cbit) { wires.push_back(diagram.to_dwire(cbit)); });
            max_dwire = wire_layer.size() - 1;
        }
        uint32_t padding = 1;
        std::string_view name = inst.name();
        std::string label =
          fmt::format("{: ^{}}", name, name.size() + (2 * padding));
        std::unique_ptr<Diagram::Operator> shape = nullptr;
        if (overlap) {
            shape = std::make_unique<Box>(
              label, wires, inst.num_targets(), inst.num_controls());
        } else if (inst.is_a<Op::Swap>()) {
            shape = std::make_unique<DiagramSwap>(wires, inst.num_controls());
        } else if (inst.is_a<Op::Measure>()) {
            shape = std::make_unique<MeasureBox>(wires);
        } else {
            shape = std::make_unique<ControlledBox>(
              label, wires, inst.num_targets(), inst.num_controls());
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
            box->set_cols(
              curr_width + (layer_width.at(layer) - box->width()) / 2u);
        }
        if ((acc_width + layer_width.at(layer)) >= (max_rows - 1)) {
            cutting_point.push_back(curr_width);
            acc_width = 0u;
        }
        curr_width += layer_width.at(layer);
        acc_width += layer_width.at(layer);
    }
    cutting_point.push_back(curr_width);
    diagram.width(curr_width);
    // Draw boxes
    for (auto const& box : boxes) {
        box->draw(diagram);
    }

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    std::string str;
    str.reserve(curr_width * diagram.height() * 4);

    uint32_t start = 0;
    for (uint32_t i = 0; i < cutting_point.size(); ++i) {
        if (i > 0) {
            str += fmt::format("\n{:#^{}}\n\n", "", max_rows);
        }
        for (uint32_t row = 0; row < diagram.height(); ++row) {
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