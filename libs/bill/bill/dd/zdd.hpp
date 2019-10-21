/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../utils/hash.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace bill {

/*! \brief A zero-suppressed decision diagram (ZDD).
 *
 *  NOTE: This is a simple implementation. I would advise against its use when high-performance
 *        is a requirement.
 * 
 *  Limitations:
 *  	- Maximum number of varibales `N_max` is: 4095 [(1 << 12) - 1]
 *  	- The number of variables `N` must be known at instantiation time. 
 * 
 * Variables are numbered from `0` to `N - 1`.
 */
class zdd_base {
#pragma region Types and constructors
private:
	struct node_type {
		node_type(uint32_t var, uint32_t lo, uint32_t hi)
		    : var(var)
		    , ref(0)
		    , dead(0)
		    , lo(lo)
		    , hi(hi)
		{}

		uint64_t var : 12;
		uint64_t ref : 11;
		uint64_t dead : 1;
		uint64_t lo : 20;
		uint64_t hi : 20;
	};

	enum operations : uint32_t {
		zdd_choose,
		zdd_difference,
		zdd_edivide,
		zdd_intersection,
		zdd_join,
		zdd_nonsupersets,
		zdd_union,
		num_operations
	};

public:
	using node_index = uint32_t;

	/* \!brief Creates a new ZDD base.
	 * 
	 * \param num_vars Number of variables (maximum = 4095)
	 * \param log_num_objs Log number of nodes to pre-allocate (default: 16)
	 */
	explicit zdd_base(uint32_t num_vars, uint32_t log_num_objs = 16)
	    : unique_tables_(num_vars)
	    , built_tautologies(false)
	    , num_variables(num_vars)
	    , num_cache_lookups(0)
	    , num_cache_misses(0)
	{
		assert(num_variables <= 4095);
		nodes_.reserve(1u << log_num_objs);
		nodes_.emplace_back(num_vars, 0, 0);
		nodes_.emplace_back(num_vars, 1, 1);
		for (auto var = 0u; var < num_vars; ++var) {
			ref(unique(var, 0, 1));
		}
	}
#pragma endregion

#pragma region ZDD base properties
public:
	/*! \brief Return the number of active nodes. */
	std::size_t num_nodes() const
	{
		return nodes_.size() - 2 - free_nodes_.size();
	}
#pragma endregion

#pragma region ZDD base operations
private:
	node_index unique(uint32_t var, node_index lo, node_index hi)
	{
		/* ZDD reduction rule */
		if (hi == 0) {
			return lo;
		}
		assert(nodes_.at(lo).var > var);
		assert(nodes_.at(hi).var > var);

		/* unique table lookup */
		const auto it = unique_tables_[var].find({lo, hi});
		if (it != unique_tables_[var].end()) {
			assert(!nodes_.at(it->second).dead);
			return it->second;
		}

		/* create new node */
		node_index new_node_index;
		if (!free_nodes_.empty()) {
			new_node_index = free_nodes_.top();
			free_nodes_.pop();
			nodes_.at(new_node_index).ref = 0;
			nodes_.at(new_node_index).dead = 0;
			nodes_.at(new_node_index).var = var;
			nodes_.at(new_node_index).lo = lo;
			nodes_.at(new_node_index).hi = hi;
		} else if (nodes_.size() < nodes_.capacity()) {
			new_node_index = nodes_.size();
			nodes_.emplace_back(var, lo, hi);
		} else {
			std::cerr << "[e] no more space for new nodes available\n";
			exit(1);
		}

		/* increase ref counts */
		ref(lo);
		ref(hi);
		return unique_tables_[var][{lo, hi}] = new_node_index;
	}

	void garbage_collect_rec(node_index index)
	{
		if (index <= 1) {
			return;
		}
		node_type& node = nodes_.at(index);
		if (node.ref == 0 || node.dead == 1) {
			return;
		}
		if (--(node.ref) == 0) {
			kill_node(index);
			garbage_collect_rec(node.lo);
			garbage_collect_rec(node.hi);
		}
	}

	void kill_node(node_index index)
	{
		free_nodes_.push(index);
		node_type& node = nodes_.at(index);
		node.dead = 1;

		/* Remove node from unique table */
		node_index lo = node.lo;
		node_index hi = node.hi;
		const auto it = unique_tables_[node.var].find({lo, hi});
		assert(it != unique_tables_[node.var].end());
		assert(it->second == index);
		unique_tables_[node.var].erase(it);
	}
public:
	/*! \brief Returns the node index corresponding to the empty family (aka, node FALSE) */
	node_index bottom() const
	{
		return 0u;
	}

	/*! \brief Returns the node index corresponding to the unit family (ake, node TRUE) */
	node_index top() const
	{
		return 1u;
	}

	/*! \brief Returns the node-id corresponding to the elementary family `{{var}}` */
	node_index elementary(uint32_t var) const
	{
		assert(var < num_variables);
		return var + 2u;
	}

	/*! \brief Build and store tautology functions
	 *
	 * This function needs to be called before any other node is created,
	 * right after the constructor.
	 */
	void build_tautologies()
	{
		assert(nodes_.size() == unique_tables_.size() + 2u);
		node_index last = top();
		for (int v = unique_tables_.size() - 1; v >= 0; --v) {
			last = unique(v, last, last);
			assert(last == 2 * unique_tables_.size() + 1u - v);
		}
		ref(last);
		built_tautologies = true;
	}

	/*! \brief Increase the reference count of a node. */
	void ref(node_index index)
	{
		if (index > 1) {
			nodes_.at(index).ref++;
		}
	}

	/*! \brief Decrease the reference count of a node. */
	void deref(node_index index)
	{
		if (index > 1 && nodes_.at(index).ref > 0) {
			nodes_.at(index).ref--;
		}
	}

	/*! \brief Remove nodes that are not referenced. */
	void garbage_collect()
	{
		std::vector<node_index> to_delete;
		/* Skip terminals and elementary nodes */
		for (auto it = nodes_.begin() + unique_tables_.size() + 2; it != nodes_.end(); ++it) {
			if (it->ref == 0 && it->dead == 0) {
				to_delete.push_back(std::distance(nodes_.begin(), it));
			}
		}
		for (auto index : to_delete) {
			kill_node(index);
			node_type const& node = nodes_.at(index);
			garbage_collect_rec(node.lo);
			garbage_collect_rec(node.hi);
		}

		/* Remove node from compute table */
		for (auto& table : computed_tables_) {
			for (auto it = table.begin(); it != table.end();) {
				if (nodes_[it->second].dead || nodes_[std::get<0>(it->first)].dead
				    || nodes_[std::get<1>(it->first)].dead) {
					it = table.erase(it);
				} else {
					++it;
				}
			}
		}
	}
#pragma endregion

#pragma region ZDD Operations
public:
	/* \!brief Computes the family of all ``k``-combinations of a ZDD.  */
	node_index choose(node_index index_f, uint32_t k)
	{
		if (k == 1) {
			return index_f;
		}
		if (index_f <= 1) {
			return k > 0 ? 0 : 1;
		}

		// Unique table lookup
		++num_cache_lookups;
		const auto it = computed_tables_[operations::zdd_choose].find({index_f, k});
		if (it != computed_tables_[operations::zdd_choose].end()) {
			assert(!nodes_.at(it->second).dead);
			return it->second;
		}
		++num_cache_misses;

		node_type const& node_f = nodes_.at(index_f);
		node_index result = choose(node_f.lo, k);
		if (k > 0) {
			auto q = choose(node_f.lo, k - 1);
			result = unique(node_f.var, result, q);
		}
		return computed_tables_[operations::zdd_choose][{index_f, k}] = result;
	}

	/* \!brief Computes the difference of two ZDDs (`f / g`)
	 *  Keep in mind that `f / g` is different from `g / f` !
	 */
	node_index difference(node_index index_f, node_index index_g)
	{
		if (index_f == 0) {
			return 0;
		}
		if (index_f == index_g) {
			return 0;
		}
		if (index_g == 0) {
			return index_f;
		}

		node_type const& node_f = nodes_.at(index_f);
		node_type const& node_g = nodes_.at(index_g);
		if (node_g.var < node_f.var) {
			return difference(index_f, node_g.lo);
		}

		// Unique table lookup
		++num_cache_lookups;
		const auto it = computed_tables_[operations::zdd_difference].find({index_f, index_g});
		if (it != computed_tables_[operations::zdd_difference].end()) {
			assert(!nodes_.at(it->second).dead);
			return it->second;
		}
		++num_cache_misses;

		node_index r_lo;
		node_index r_hi;
		if (node_f.var == node_g.var) {
			r_lo = difference(node_f.lo, node_g.lo);
			r_hi = difference(node_f.hi, node_g.hi);
		} else {
			r_lo = difference(node_f.lo, index_g);
			r_hi = node_f.hi;
		}
		node_index index_new = unique(node_f.var, r_lo, r_hi);
		return computed_tables_[operations::zdd_difference][{index_f, index_g}] = index_new;
	}

	/* \!brief Computes the intersection of two ZDDs */
	node_index intersection(node_index index_f, node_index index_g)
	{
		if (index_f == 0) {
			return 0;
		}
		if (index_g == 0) {
			return 0;
		}
		if (index_f == index_g) {
			return index_f;
		}
		if (index_f > index_g) {
			std::swap(index_f, index_g);
		}

		node_type const& node_f = nodes_.at(index_f);
		node_type const& node_g = nodes_.at(index_g);
		if (node_f.var < node_g.var) {
			return intersection(node_f.lo, index_g);
		} else if (node_f.var > node_g.var) {
			return intersection(index_f, node_g.lo);
		}

		// Unique table lookup
		++num_cache_lookups;
		const auto it = computed_tables_[operations::zdd_intersection].find({index_f, index_g});
		if (it != computed_tables_[operations::zdd_intersection].end()) {
			assert(!nodes_.at(it->second).dead);
			return it->second;
		}
		++num_cache_misses;

		node_index const r_lo = intersection(node_f.lo, node_g.lo);
		node_index const r_hi = intersection(node_f.hi, node_g.hi);
		node_index index_new = unique(node_f.var, r_lo, r_hi);
		return computed_tables_[operations::zdd_intersection][{index_f, index_g}] = index_new;
	}

	/* \!brief Computes the join of two ZDDs */
	node_index join(node_index index_f, node_index index_g)
	{
		if (index_f == 0) {
			return 0;
		}
		if (index_g == 0) {
			return 0;
		}
		if (index_f == 1) {
			return index_g;
		}
		if (index_g == 1) {
			return index_f;
		}
		if (index_f > index_g) {
			std::swap(index_f, index_g);
		}

		// Unique table lookup
		++num_cache_lookups;
		const auto it = computed_tables_[operations::zdd_join].find({index_f, index_g});
		if (it != computed_tables_[operations::zdd_join].end()) {
			assert(!nodes_.at(it->second).dead);
			return it->second;
		}
		++num_cache_misses;

		node_type const& node_f = nodes_.at(index_f);
		node_type const& node_g = nodes_.at(index_g);
		node_index r_lo;
		node_index r_hi;
		if (node_f.var < node_g.var) {
			r_lo = join(node_f.lo, index_g);
			r_hi = join(node_f.hi, index_g);
		} else if (node_f.var > node_g.var) {
			r_lo = join(index_f, node_g.lo);
			r_hi = join(index_f, node_g.hi);
		} else {
			r_lo = join(node_f.lo, node_f.lo);
			node_index const r_lh = join(node_f.lo, node_g.hi);
			node_index const r_hl = join(node_f.hi, node_g.lo);
			node_index const r_hh = join(node_f.hi, node_g.hi);
			r_hi = union_(r_lh, union_(r_hl, r_hh));
		}
		const auto var = std::min(node_f.var, node_g.var);
		node_index index_new = unique(var, r_lo, r_hi);
		return computed_tables_[operations::zdd_join][{index_f, index_g}] = index_new;
	}

	/* \!brief Computes the nonsupersets of two ZDDs */
	node_index nonsupersets(node_index index_f, node_index index_g)
	{
		if (index_f == 0) {
			return 0;
		}
		if (index_g == 0) {
			return index_f;
		}
		if (index_g == 1) {
			return 0;
		}
		if (index_f == index_g) {
			return 0;
		}

		node_type const& node_f = nodes_.at(index_f);
		node_type const& node_g = nodes_.at(index_g);
		if (node_f.var > node_g.var) {
			return nonsupersets(index_f, node_g.lo);
		}

		// Unique table lookup
		++num_cache_lookups;
		const auto it = computed_tables_[operations::zdd_nonsupersets].find({index_f, index_g});
		if (it != computed_tables_[operations::zdd_nonsupersets].end()) {
			assert(!nodes_.at(it->second).dead);
			return it->second;
		}
		++num_cache_misses;

		node_index r_lo;
		node_index r_hi;
		if (node_f.var < node_g.var) {
			r_lo = nonsupersets(node_f.lo, index_g);
			r_hi = nonsupersets(node_f.hi, index_g);
		} else {
			r_hi = intersection(nonsupersets(node_f.hi, node_g.lo),
			                    nonsupersets(node_f.hi, node_g.hi));
			r_lo = nonsupersets(node_f.lo, node_g.lo);
		}
		node_index index_new = unique(node_f.var, r_lo, r_hi);
		return computed_tables_[operations::zdd_nonsupersets][{index_f, index_g}] = index_new;
	}

	/* \!brief Return the tautology function ``f(var) = true`` 
	 *  Important: the function ``build_tautologies`` must have been called.
	 */
	node_index tautology(uint32_t var = 0) const
	{
		assert(built_tautologies);
		if (var == unique_tables_.size()) {
			return top();
		}
		return 2 * unique_tables_.size() + 1u - var;
	}

	/* \!brief Computes the union of two ZDDs */
	node_index union_(node_index index_f, node_index index_g)
	{
		if (index_f == 0) {
			return index_g;
		}
		if (index_g == 0) {
			return index_f;
		}
		if (index_f == index_g) {
			return index_f;
		}
		if (index_f > index_g) {
			std::swap(index_f, index_g);
		}

		// Unique table lookup
		++num_cache_lookups;
		const auto it = computed_tables_[operations::zdd_union].find({index_f, index_g});
		if (it != computed_tables_[operations::zdd_union].end()) {
			assert(!nodes_.at(it->second).dead);
			return it->second;
		}
		++num_cache_misses;

		node_type const& node_f = nodes_.at(index_f);
		node_type const& node_g = nodes_.at(index_g);
		node_index r_lo;
		node_index r_hi;
		if (node_f.var < node_g.var) {
			r_lo = union_(node_f.lo, index_g);
			r_hi = node_f.hi;
		} else if (node_f.var > node_g.var) {
			r_lo = union_(index_f, node_g.lo);
			r_hi = node_g.hi;
		} else {
			r_lo = union_(node_f.lo, node_g.lo);
			r_hi = union_(node_f.hi, node_g.hi);
		}
		const auto var = std::min(node_f.var, node_g.var);
		node_index index_new = unique(var, r_lo, r_hi);
		return computed_tables_[operations::zdd_union][{index_f, index_g}] = index_new;
	}
#pragma endregion

#pragma region ZDD iterators
private:
	template<class Fn>
	bool foreach_set_rec(node_index index, std::vector<uint32_t>& set, Fn&& fn) const
	{
		if (index == 1u) {
			return fn(set);
		}
		if (index != 0u) {
			if (!foreach_set_rec(nodes_.at(index).lo, set, fn)) {
				return false;
			}
			auto new_set = set;
			new_set.push_back(nodes_.at(index).var);
			if (!foreach_set_rec(nodes_.at(index).hi, new_set, fn)) {
				return false;
			}
		}
		return true;
	}

public:
	template<class Fn>
	void foreach_set(node_index index, Fn&& fn) const
	{
		std::vector<uint32_t> set;
		foreach_set_rec(index, set, fn);
	}
#pragma endregion

#pragma region ZDD properties
private:
	void count_nodes_rec(node_index index, std::unordered_set<node_index>& visited) const
	{
		if (index <= 1 || visited.count(index)) {
			return;
		}
		visited.insert(index);
		node_type const& node = nodes_.at(index);
		count_nodes_rec(node.lo, visited);
		count_nodes_rec(node.hi, visited);
	}

	uint64_t count_sets_rec(node_index index, std::unordered_map<node_index, uint64_t>& visited) const
	{
		if (index <= 1) {
			return index;
		}
		const auto it = visited.find(index);
		if (it != visited.end()) {
			return it->second;
		}
		node_type const& node = nodes_.at(index);
		return visited[index] = count_sets_rec(node.lo, visited)
		                      + count_sets_rec(node.hi, visited);
	}

public:
	/* \!brief Return the number of nodes in a ZDD. */
	uint64_t count_nodes(node_index index_root) const
	{
		if (index_root <= 1) {
			return 0;
		}
		std::unordered_set<node_index> visited;
		count_nodes_rec(index_root, visited);
		return visited.size();
	}

	/* \!brief Return the number of sets in a ZDD. */
	uint64_t count_sets(node_index index_root) const
	{
		if (index_root <= 1) {
			return index_root;
		}
		std::unordered_map<node_index, uint64_t> visited;
		return count_sets_rec(index_root, visited);
	}

	std::vector<std::vector<uint32_t>> sets_as_vectors(node_index index) const
	{
		std::vector<std::vector<uint32_t>> sets_vectors;
		foreach_set(index, [&](auto const& set){
			sets_vectors.emplace_back(set);
			return true;
		});
		return sets_vectors;
	}
#pragma endregion

#pragma region Debug
public:
	void print_debug(std::ostream& os = std::cout) const
	{
		os << "ZDD nodes:\n";
		os << "    i     VAR    LO    HI   REF\n";
		uint32_t i = 0u;
		for (node_type const& node : nodes_) {
			os << fmt::format("{:5} : {:5} {:5} {:5} {:5}\n", i++, node.var, node.lo,
			                  node.hi, node.ref);
		}
	}

	void print_sets(node_index index, std::ostream& os = std::cout) const
	{
		foreach_set(index, [&](auto const& set){
			os << fmt::format("{{ {} }}\n", fmt::join(set, ", "));
			return true;
		});
	}
#pragma endregion

private:
	using children_type = std::pair<node_index, node_index>;
	using unique_table_type = std::unordered_map<children_type, node_index>;

	std::vector<node_type> nodes_;
	std::stack<node_index> free_nodes_;
	std::vector<unique_table_type> unique_tables_;
	std::array<unique_table_type, operations::num_operations> computed_tables_;

	bool built_tautologies = false;

	// Stats
	uint32_t num_variables;
	uint32_t num_cache_lookups;
	uint32_t num_cache_misses;
};

} // namespace bill
