/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <array>
#include <cassert>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <vector>

namespace tweedledum::xag_synth_detail {

class HighLevelXAGBuilder;

class HighLevelXAG {
public:
    class Node {
    public:
        using Ref = uint32_t;
        Node()
            : begin_fanin1_(0), begin_fanin01_(0), level_(0), last_level_(0),
                num_ref_(0)
        {}

        Node(std::vector<Ref> const& fanin)
            : fanin_(fanin), begin_fanin1_(fanin.size()),
                begin_fanin01_(fanin.size()),
                level_(std::numeric_limits<uint32_t>::max()), last_level_(0), num_ref_(0)
        {}

        Node(std::vector<Ref> const& fanin, uint32_t begin_fanin1,
            uint32_t begin_fanin01,
            std::array<bool, 2> const& is_negated)
            : fanin_(fanin), begin_fanin1_(begin_fanin1),
                begin_fanin01_(begin_fanin01), is_negated_(is_negated),
                level_(std::numeric_limits<uint32_t>::max()), last_level_(0),num_ref_(0)
        {}

        uint32_t level() const
        {
            return level_;
        }

        uint32_t last_level() const
        {
            return last_level_;
        }

        bool is_input() const
        {
            return fanin_.size() == 0;
        }

        bool is_parity() const
        {
            return fanin_.size() == begin_fanin1_;
        }

        bool is_parity_and() const
        {
            return !is_parity();
        }

        bool is_negated(uint32_t input) const
        {
            assert(input < 2u);
            return is_negated_.at(input);
        }

        uint32_t num_ref() const
        {
            return num_ref_;
        }

        void incr_references(uint32_t n)
        {
            num_ref_ += n;
        }

        void decr_references()
        {
            num_ref_ -= 1;
        }

        auto begin()
        {
            return fanin_.begin();
        }

        auto end()
        {
            return fanin_.end();
        }

        auto begin() const
        {
            return fanin_.cbegin();
        }

        auto end() const
        {
            return fanin_.cend();
        }

        auto cbegin() const
        {
            return fanin_.cbegin();
        }

        auto cend() const
        {
            return fanin_.cend();
        }

        auto crbegin() const
        {
            return fanin_.crbegin();
        }

        auto crend() const
        {
            return fanin_.crend();
        }

        auto cbegin_in0() const
        {
            return fanin_.cbegin();
        }

        auto cend_in0() const
        {
            return fanin_.cbegin() + begin_fanin1_;
        }

        auto cbegin_in1() const
        {
            return fanin_.cbegin() + begin_fanin1_;
        }

        auto cend_in1() const
        {
            return fanin_.cbegin() + begin_fanin01_;
        }

        auto cbegin_in01() const
        {
            return fanin_.cbegin() + begin_fanin01_;
        }

        auto cend_in01() const
        {
            return fanin_.cend();
        }

        void last_level(uint32_t level)
        {
            last_level_ = level;
        }

    private:
        void level(uint32_t level)
        {
            level_ = level;
        }

        friend class HighLevelXAGBuilder;

        // I use the same fanin vector to store 3 vectors.
        std::vector<Ref> fanin_;
        uint32_t begin_fanin1_;
        uint32_t begin_fanin01_;
        std::array<bool, 2> is_negated_;
        uint32_t level_;
        uint32_t last_level_;
        uint32_t num_ref_;
    };
    using NodeRef = Node::Ref;
    using OutputRef = std::pair<uint32_t, bool>;

    HighLevelXAG()
        : num_inputs_(0), num_levels_(0)
    {
        nodes_.reserve(1024);
        // Create the constant node
        nodes_.emplace_back();
    }

    uint32_t size() const
    {
        return nodes_.size();
    }

    uint32_t num_inputs() const
    {
        return num_inputs_;
    }

    uint32_t num_levels() const
    {
        return num_levels_;
    }

    NodeRef create_pi()
    {
        nodes_.emplace_back();
        num_inputs_ += 1;
        return nodes_.size() - 1;
    }

    NodeRef create_xor(std::vector<NodeRef> const& fanin)
    {
        nodes_.emplace_back(fanin);
        return nodes_.size() - 1;
    }

    Node& get_node(NodeRef node_ref)
    {
        return nodes_.at(node_ref);
    }

    Node const& get_node(NodeRef node_ref) const
    {
        return nodes_.at(node_ref);
    }

    void reference(uint32_t node_ref, uint32_t n = 1u)
    {
        nodes_.at(node_ref).incr_references(n);
    }

    void dereference(uint32_t node_ref)
    {
        nodes_.at(node_ref).decr_references();
    }

    auto begin()
    {
        return nodes_.begin() + num_inputs_ + 1;
    }

    auto end()
    {
        return nodes_.end();
    }

    auto rbegin()
    {
        return nodes_.rbegin();
    }

    auto rend()
    {
        return nodes_.rend() - num_inputs_ - 1;
    }

    auto cbegin_outputs() const
    {
        return outputs_.cbegin();
    }

    auto cend_outputs() const
    {
        return outputs_.end();
    }

private:
    std::vector<Node> nodes_;
    std::vector<OutputRef> outputs_;
    uint32_t num_inputs_;
    uint32_t num_levels_;

    friend class HighLevelXAGBuilder;
};

class HighLevelXAGBuilder {
    using NodeRef = HighLevelXAG::NodeRef;
    // Mockturtle types
    using xag_network = mockturtle::xag_network;
    using xag_node = typename xag_network::node;
    using xag_signal = typename xag_network::signal;
    using xag_ltfi = std::vector<xag_signal>;
    using ltfi_map = mockturtle::node_map<xag_ltfi, xag_network>;

public:
    HighLevelXAGBuilder(xag_network const& xag)
        : xag_(xag), node_ltfi_(xag)
    {
        compute_ltfi();
    }

    // static HighLevelXAG build(xag_network const& xag);

    HighLevelXAG operator()();

private:
    void mark_xor_drivers();
    void compute_ltfi();

    void create_constant(HighLevelXAG& hl_xag);
    NodeRef create_pi(HighLevelXAG& hl_xag);
    void create_po(HighLevelXAG& hl_xag, NodeRef node_ref, bool is_negated);
    NodeRef create_parity(
        HighLevelXAG& hl_xag, std::vector<NodeRef> const& fanin, uint32_t level);
    NodeRef create_parity(HighLevelXAG& hl_xag, std::vector<uint32_t>& fanin0,
        std::vector<uint32_t> const& fanin1,
        std::vector<uint32_t> const& fanin01,
        std::array<bool, 2> const& is_negated, uint32_t level);

    NodeRef to_node_ref(xag_signal signal) const;

    NodeRef handle_xor(
        HighLevelXAG& hl_xag, xag_ltfi const& ltfi0, xag_ltfi const& ltfi1);

    NodeRef handle_and(HighLevelXAG& hl_xag, xag_ltfi const& ltfi0,
        xag_ltfi const& ltfi1, std::array<bool, 2> is_negated);

    xag_network const& xag_;
    ltfi_map node_ltfi_;
    std::vector<uint32_t> asap_level_;
};

void HighLevelXAGBuilder::mark_xor_drivers()
{
    xag_.clear_values();
    xag_.foreach_po([&](xag_signal const& signal) {
        xag_node const& node = xag_.get_node(signal);
        if (!xag_.is_xor(node)) {
            return;
        }
        xag_.set_value(node, 1u);
    });
}

void HighLevelXAGBuilder::compute_ltfi()
{
    // First, I need to mark the XOR nodes which drive an output, so they
    // can get their own LTFI variable.
    mark_xor_drivers();
    // Compute the LTFI for the inputs
    xag_.foreach_pi([&](xag_node const& node) {
        assert(!xag_.is_constant(node));
        node_ltfi_[node].emplace_back(xag_.make_signal(node));
    });
    // Compute the LTFI for all the gates
    xag_.foreach_gate([&](xag_node const& node) {
        std::array<xag_ltfi const*, 2> fanin_ltfi;
        xag_.foreach_fanin(
            node, [&](xag_signal const signal, uint32_t i) {
                fanin_ltfi.at(i) = &(node_ltfi_[signal]);
            });
        // If this node is a AND gate or a XOR which drives an output,
        // then its LTFI is just itself
        if (xag_.is_and(node) || xag_.value(node)) {
            node_ltfi_[node].emplace_back(xag_.make_signal(node));
            return;
        }
        // The node is an internal XOR
        std::set_symmetric_difference(fanin_ltfi.at(0)->cbegin(),
            fanin_ltfi.at(0)->cend(), fanin_ltfi.at(1)->cbegin(),
            fanin_ltfi.at(1)->cend(),
            std::back_inserter(node_ltfi_[node]));
        // Make sure the LTFI is not empty!
        assert(node_ltfi_[node].size());
    });
}

void HighLevelXAGBuilder::create_constant(HighLevelXAG& hl_xag)
{
    asap_level_.emplace_back(0);
}

HighLevelXAG::NodeRef HighLevelXAGBuilder::create_pi(HighLevelXAG& hl_xag)
{
    asap_level_.emplace_back(0);
    hl_xag.nodes_.emplace_back();
    hl_xag.num_inputs_ += 1;
    return hl_xag.nodes_.size() - 1;
}

void HighLevelXAGBuilder::create_po(HighLevelXAG& hl_xag, NodeRef node_ref, bool is_negated)
{
    hl_xag.outputs_.emplace_back(node_ref, is_negated);
}

HighLevelXAG::NodeRef HighLevelXAGBuilder::create_parity(
    HighLevelXAG& hl_xag, std::vector<NodeRef> const& fanin, uint32_t level)
{
    hl_xag.nodes_.emplace_back(fanin);
    asap_level_.emplace_back(level);
    return hl_xag.nodes_.size() - 1;
}

HighLevelXAG::NodeRef HighLevelXAGBuilder::create_parity(HighLevelXAG& hl_xag,
    std::vector<uint32_t>& fanin0, std::vector<uint32_t> const& fanin1,
    std::vector<uint32_t> const& fanin01, std::array<bool, 2> const& is_negated,
    uint32_t level)
{
    assert(fanin0.size() && (fanin01.size() || fanin1.size()));
    uint32_t begin1 = fanin0.size();
    std::copy(fanin1.begin(), fanin1.end(), std::back_inserter(fanin0));
    uint32_t begin01 = fanin0.size();
    std::copy(fanin01.begin(), fanin01.end(), std::back_inserter(fanin0));
    hl_xag.nodes_.emplace_back(fanin0, begin1, begin01, is_negated);
    asap_level_.emplace_back(level);
    return hl_xag.nodes_.size() - 1;
}

uint32_t HighLevelXAGBuilder::to_node_ref(xag_signal signal) const
{
    return xag_.value(xag_.get_node(signal));
}

uint32_t HighLevelXAGBuilder::handle_xor(
    HighLevelXAG& hl_xag, xag_ltfi const& ltfi0, xag_ltfi const& ltfi1)
{
    std::vector<uint32_t> fanin;
    auto first0 = ltfi0.cbegin();
    auto last0 = ltfi0.cend();
    auto first1 = ltfi1.cbegin();
    auto last1 = ltfi1.cend();
    uint32_t level = 0;
    NodeRef fanin_ref;
    while (first0 != last0) {
        if (first1 == last1) {
            while (first0 != last0) {
                fanin_ref = to_node_ref(*first0);
                fanin.emplace_back(fanin_ref);
                hl_xag.reference(fanin_ref);
                level = std::max(level, asap_level_.at(fanin_ref));
                ++first0;
            }
            goto handle_xor_end;
        }
        if (*first0 < *first1) {
            fanin_ref = to_node_ref(*first0);
            fanin.emplace_back(fanin_ref);
            hl_xag.reference(fanin_ref);
            level = std::max(level, asap_level_.at(fanin_ref));
            ++first0;
        } else {
            if (*first1 < *first0) {
                fanin_ref = to_node_ref(*first1);
                fanin.emplace_back(fanin_ref);
                hl_xag.reference(fanin_ref);
                level = std::max(level, asap_level_.at(fanin_ref));
            } else {
                ++first0;
            }
            ++first1;
        }
    }
    while (first1 != last1) {
        fanin_ref = to_node_ref(*first1);
        fanin.emplace_back(fanin_ref);
        hl_xag.reference(fanin_ref);
        level = std::max(level, asap_level_.at(fanin_ref));
        ++first1;
    }
handle_xor_end:
    return create_parity(hl_xag, fanin, ++level);
}

uint32_t HighLevelXAGBuilder::handle_and(HighLevelXAG& hl_xag, xag_ltfi const& ltfi0,
    xag_ltfi const& ltfi1, std::array<bool, 2> is_negated)
{
    std::vector<uint32_t> fanin0;
    std::vector<uint32_t> fanin1;
    std::vector<uint32_t> fanin01;
    auto first0 = ltfi0.cbegin();
    auto last0 = ltfi0.cend();
    auto first1 = ltfi1.cbegin();
    auto last1 = ltfi1.cend();
    uint32_t level = 0;
    NodeRef fanin_ref;
    while (first0 != last0 && first1 != last1) {
        if (*first0 == *first1) {
            fanin_ref = to_node_ref(*first0);
            fanin01.emplace_back(fanin_ref);
            hl_xag.reference(fanin_ref, 2);
            level = std::max(level, asap_level_.at(fanin_ref));
            ++first0;
            ++first1;
        } else if (*first0 < *first1) {
            fanin_ref = to_node_ref(*first0);
            fanin0.emplace_back(fanin_ref);
            hl_xag.reference(fanin_ref, 2);
            level = std::max(level, asap_level_.at(fanin_ref));
            ++first0;
        } else {
            fanin_ref = to_node_ref(*first1);
            fanin1.emplace_back(fanin_ref);
            hl_xag.reference(fanin_ref, 2);
            level = std::max(level, asap_level_.at(fanin_ref));
            ++first1;
        }
    }
    while (first0 != last0) {
        fanin_ref = to_node_ref(*first0);
        fanin0.emplace_back(fanin_ref);
        hl_xag.reference(fanin_ref, 2);
        level = std::max(level, asap_level_.at(fanin_ref));;
        ++first0;
    }
    while (first1 != last1) {
        fanin_ref = to_node_ref(*first1);
        fanin1.emplace_back(fanin_ref);
        hl_xag.reference(fanin_ref, 2);
        level = std::max(level, asap_level_.at(fanin_ref));
        ++first1;
    }
    if (fanin0.size() < fanin1.size()) {
        std::swap(is_negated[0], is_negated[1]);
        return create_parity(hl_xag, fanin1, fanin0, fanin01, is_negated, ++level);
    }
    return create_parity(hl_xag, fanin0, fanin1, fanin01, is_negated, ++level);
}

HighLevelXAG HighLevelXAGBuilder::operator()()
{
    HighLevelXAG hl_xag;
    create_constant(hl_xag);
    xag_.foreach_pi([&](xag_node const& node) {
        assert(!xag_.is_constant(node));
        xag_.set_value(node, create_pi(hl_xag));
        return;
    });
    xag_.foreach_gate([&](xag_node const& node) {
        if (xag_.is_xor(node) && xag_.value(node) == 0) {
            // non-driver XOR node, ignore it!
            return;
        }
        std::array<xag_ltfi const*, 2> fanin_ltfi;
        std::array<bool, 2> is_negated;
        xag_.foreach_fanin(
            node, [&](xag_signal const signal, uint32_t i) {
                fanin_ltfi.at(i) = &(node_ltfi_[signal]);
                is_negated.at(i) = xag_.is_complemented(signal);
            });
        if (xag_.is_xor(node)) {
            uint32_t ref = handle_xor(
                hl_xag, *fanin_ltfi.at(0), *fanin_ltfi.at(1));
            xag_.set_value(node, ref);
            return;
        }
        uint32_t ref = handle_and(
            hl_xag, *fanin_ltfi.at(0), *fanin_ltfi.at(1), is_negated);
        xag_.set_value(node, ref);
    });
    uint32_t max_level = *std::max_element(asap_level_.begin(), asap_level_.end());
    // Create pointers to outputs and set the level of all nodes
    xag_.foreach_po([&](xag_signal const& signal) {
        uint32_t node_ref = to_node_ref(signal);
        HighLevelXAG::Node& node = hl_xag.get_node(node_ref);
        create_po(hl_xag, node_ref, xag_.is_complemented(signal));
        node.level(max_level);
        node.last_level(max_level);
    });
    std::for_each(hl_xag.rbegin(), hl_xag.rend(),
    [&](HighLevelXAG::Node& node) {
        uint32_t level = node.level() - 1;
        for (NodeRef input_ref : node) {
            HighLevelXAG::Node& input = hl_xag.get_node(input_ref);
            input.level(std::min(level, input.level()));
            input.last_level(std::max(node.level(), input.last_level()));
        }
    });
    hl_xag.num_levels_ = max_level + 1; // +1 to account for the 0th level 
    return hl_xag;
}

HighLevelXAG to_pag(mockturtle::xag_network const& xag)
{
    HighLevelXAGBuilder builder(xag);
    return builder();
}

} // namespace tweedledum::xag_synth_detail
