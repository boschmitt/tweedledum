/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Utils/Visualization/string_utf8.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace tweedledum {

struct StringBuilder {

    StringBuilder(Circuit const& circuit)
        : max_num_glyphs_(0u)
        , info_(circuit.num_wires(), 0)
        , num_glyphs_(circuit.num_wires(), 0)
        , lines_(3 * circuit.num_wires())
    {
        circuit.foreach_wire([&](Wire const& wire) {
            max_num_glyphs_ = std::max(utf8_strlen(wire.name), max_num_glyphs_);
        });
        circuit.foreach_wire([&](WireRef i, Wire const& wire) {
            lines_[(3 * i)] += fmt::format("{:>{}}", "", max_num_glyphs_ + 6u);
            if (wire.kind == Wire::Kind::quantum) {
                lines_[(3 * i) + 1] += fmt::format("{:>{}} : ───", wire.name, max_num_glyphs_);
                info_.at(i) |= 2;
            } else {
                lines_[(3 * i) + 1] += fmt::format("{:>{}} : ═══", wire.name, max_num_glyphs_);
            }
            lines_[(3 * i) + 2] += fmt::format("{:>{}}", "", max_num_glyphs_ + 6u);
            num_glyphs_.at(i) = max_num_glyphs_ + 6u;
        });
        max_num_glyphs_ += 6u;
    }

    std::string finish()
    {
        finish_column();
        std::string result;
        std::for_each(lines_.rbegin(), lines_.rend(), 
        [&result] (std::string const& line) {
            result += line + "\n";
        });
        return result;
    }
    
    uint32_t utf8_strlen(std::string_view str)
    {
        uint32_t num_glyphs = 0u;
        uint32_t end = str.length();
        for (uint32_t i = 0; i < end; i++, num_glyphs++) {
            uint8_t c = static_cast<uint8_t>(str.at(i));
            if (c <= 127u) {
                continue;
            } else if ((c & 0xE0) == 0xC0) {
                i += 1;
            } else if ((c & 0xF0) == 0xE0) {
                i += 2;
            } else if ((c & 0xF8) == 0xF0) {
                i += 3;
            } else {
                return 0u;
            }
        }
        return num_glyphs;
    }

    void finish_column()
    {
        for (auto i = 0u; i < info_.size(); ++i) {
            assert(num_glyphs_.at(i) <= max_num_glyphs_);
            uint32_t pad = max_num_glyphs_ - num_glyphs_.at(i);
            if (pad) {
                lines_[(3 * i)] += fmt::format("{:>{}}", "", pad);
                if (info_.at(i) & 2u) {
                    lines_[(3 * i) + 1] += fmt::format("{:─>{}}", "", pad);
                } else {
                    lines_[(3 * i) + 1] += fmt::format("{:═>{}}", "", pad);
                }
                lines_[(3 * i) + 2] += fmt::format("{:>{}}", "", pad);
            }
            num_glyphs_.at(i) = max_num_glyphs_;
            info_.at(i) &= ~1u;
        }
    }

    bool does_need_new_column(uint32_t from, uint32_t to)
    {
        for (uint32_t i = from; i <= to; ++i) {
            if (info_[i] & 1) {
                return true;
            }
        }
        return false;
    }

    bool is_overlap(uint32_t a_min, uint32_t a_max, uint32_t b_min, uint32_t b_max)
    {
        return (a_min > b_min && a_min < b_max) || (a_max > b_min && a_max < b_max);
    }

    std::pair<uint32_t, uint32_t> minmax(std::vector<WireRef> const& wires)
    {
        uint32_t min = std::numeric_limits<uint32_t>::max();
        uint32_t max = 0;
        for (WireRef ref : wires) {
            min = std::min(min, ref.uid());
            max = std::max(max, ref.uid());
        }
        return {min, max};
    }

    void add_op(std::string_view name, std::vector<WireRef> const& controls, std::vector<WireRef> const& targets)
    {
        assert(!targets.empty());
        uint32_t const width = utf8_strlen(name) + 2u;
        auto const [c_min, c_max] = minmax(controls);
        auto const [t_min,t_max] = minmax(targets);
        // Check if I need a new column
        uint32_t const min = std::min(c_min, t_min);
        uint32_t const max = std::max(c_max, t_max);
        if (does_need_new_column(min, max)) {
            finish_column();
        }
        // If they overlap
        if (is_overlap(c_min, c_max, t_min, t_max)) {
            return;
        }
        // Otherwise
        for (WireRef c : controls) {
            uint32_t const line = c.uid();
            info_[line] |= 1;
            lines_[(3 * line)] += fmt::format(" {:^{}} ", line == min ? " " : "│", width);
            lines_[(3 * line) + 1] += fmt::format("─{:─^{}}─", c.is_complemented() ? "◯" : "●", width);
            lines_[(3 * line) + 2] += fmt::format(" {:^{}} ", line == max ? " " : "│", width);
            num_glyphs_.at(line) += 2u + width;
        }
        uint32_t const bot = (3 * t_min);
        uint32_t const top = (3 * t_max) + 2;
        uint32_t const mid = (bot + top)/2;
        lines_[top] += fmt::format("┌{:─^{}}┐", t_max == max ? "─" : "┴", width);
        for (WireRef t : targets) {
            uint32_t const l = (3 * t) + 1;
            lines_.at(l) += fmt::format("┤{:^{}}├", l == mid? name : "", width);
        }
        lines_[bot] += fmt::format("└{:─^{}}┘", t_min == min ? "─" : "┬", width);
        for (uint32_t i = bot + 1; i < top; ++i) {
            if (utf8_strlen(lines_.at(top)) == utf8_strlen(lines_.at(i))) {
                continue;
            }
            lines_[i] += fmt::format("│{:^{}}│", i == mid? name : "", width);
        }
        for (uint32_t i = t_min; i <= t_max; ++i) {
            info_[i] |= 1;
            num_glyphs_.at(i) += 2u + width;
        }

        for (auto i = min + 1; i < max; ++i) {
            if (info_[i] & 1) {
                continue;
            }
            info_[i] |= 1;
            lines_[(3 * i)] += fmt::format(" {:^{}} ", "│", width);
            if (info_[i] & 2) {
                lines_[(3 * i) + 1] += fmt::format("─{:─^{}}─", "┼", width);
            } else {
                lines_[(3 * i) + 1] += fmt::format("═{:═^{}}═", "╪", width);
            }
            lines_[(3 * i) + 2] += fmt::format(" {:^{}} ", "│", width);
            num_glyphs_.at(i) += 2u + width;
        }
        max_num_glyphs_ = std::max(max_num_glyphs_, num_glyphs_.at(t_max));
    }

    uint32_t max_num_glyphs_;
    std::vector<uint8_t> info_;
    std::vector<uint32_t> num_glyphs_;
    std::vector<std::string> lines_;
};

std::string to_string_utf8(Circuit const& circuit)
{
    StringBuilder builder(circuit);
    circuit.foreach_instruction([&](Instruction const& inst) {
        std::string_view kind = inst.kind();
        auto pos = kind.find_first_of(".");
        if (pos ==  std::string_view::npos) {
            pos = 0;
        } else {
            ++pos;
        }
        std::string name(kind.begin() + pos, kind.end());
        std::vector<WireRef> controls;
        inst.foreach_control([&controls](WireRef control) {
            controls.push_back(control);
        });
        std::vector<WireRef> targets;
        inst.foreach_target([&targets](WireRef target) {
            targets.push_back(target);
        });
        builder.add_op(name, controls, targets);
    });
    return builder.finish();
}

void print(Circuit const& circuit)
{
    fmt::print("{}", to_string_utf8(circuit));
}

} // namespace tweedledum