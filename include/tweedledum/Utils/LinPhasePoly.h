/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <iterator>
#include <vector>

namespace tweedledum {

// Linear Phase Polynomial (LinerPP)
class LinPhasePoly {
public:
    using Parity = std::vector<uint32_t>;
    using PhaseParity = std::pair<Parity, double>;

    LinPhasePoly() = default;

    uint32_t size() const
    {
        return terms_.size();
    }

    auto begin() const
    {
        return terms_.cbegin();
    }

    auto end() const
    {
        return terms_.cend();
    }

    void add_term(uint32_t parity, double const angle)
    {
        add_term(convert(parity), angle);
    }

    void add_term(Parity const& parity, double const angle)
    {
        auto it = lower_bound(terms_.begin(), terms_.end(), parity);
        if (it != terms_.end() && it->first == parity) {
            it->second += angle;
            return;
        }
        terms_.emplace(it, parity, angle);
    }

    double extract_phase(uint32_t parity)
    {
        return extract_phase(convert(parity));
    }

    double extract_phase(Parity const& parity)
    {
        auto it = lower_bound(terms_.begin(), terms_.end(), parity);
        if (it == terms_.end() || it->first != parity) {
            return 0;
        }
        double const angle = it->second;
        terms_.erase(it);
        return angle;
    }

private:
    using Iterator = typename std::vector<PhaseParity>::iterator;
    using DistType = typename std::iterator_traits<Iterator>::difference_type;

    std::vector<uint32_t> convert(uint32_t parity) const
    {
        std::vector<uint32_t> esop;
        for (uint32_t i = 1; parity; ++i) {
            if (parity & 1) {
                esop.push_back((i << 1));
            }
            parity >>= 1;
        }
        return esop;
    }

    Iterator lower_bound(Iterator first, Iterator last, Parity const& parity)
    {
        DistType len = std::distance(first, last);
        while (len > 0) {
            if (first->first == parity) {
                break;
            }
            DistType half = len >> 1;
            Iterator middle = first;
            std::advance(middle, half);
            if (middle->first < parity) {
                first = ++middle;
                len -= half + 1;
            } else {
                len = half;
            }
        }
        return first;
    }

    std::vector<PhaseParity> terms_;
};

} // namespace tweedledum
