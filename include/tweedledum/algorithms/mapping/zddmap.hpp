/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tweedledum {

#pragma region device data structure(to be moved)
struct device_t {
	std::vector<std::pair<uint8_t, uint8_t>> edges;
	uint8_t num_vertices;
};
#pragma endregion

#pragma region ZDD package
class zdd_base {
private:
	struct node_t {
		node_t(uint32_t var, uint32_t lo, uint32_t hi)
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

	enum op_t {
		zdd_union,
		zdd_intersection,
		zdd_difference,
		zdd_join,
		zdd_nonsupersets,
		zdd_edivide,
		zdd_sym,
		zdd_choose
	};

public:
	using node = uint32_t; // std::vector<node_t>::size_type;

	explicit zdd_base(uint32_t num_vars, uint32_t log_num_objs = 16)
	    : unique_table(num_vars)
	{
		nodes.reserve(1u << log_num_objs);

		nodes.emplace_back(num_vars, 0, 0);
		nodes.emplace_back(num_vars, 1, 1);

		for (auto v = 0u; v < num_vars; ++v) {
			ref(unique(v, 0, 1));
		}
	}

	node bot() const
	{
		return 0u;
	}

	node top() const
	{
		return 1u;
	}

	node elementary(uint32_t var) const
	{
		return var + 2u;
	}

	/*! \brief Build and store tautology functions
	 *
	 * This function needs to be called before any other node is created,
	 * right after the constructor.
	 */
	void build_tautologies()
	{
		assert(nodes.size() == unique_table.size() + 2u);

		auto last = top();
		for (int v = unique_table.size() - 1; v >= 0; --v) {
			last = unique(v, last, last);
			assert(last == 2 * unique_table.size() + 1u - v);
		}
		ref(last);
	}

	node tautology(uint32_t var = 0) const
	{
		if (var == unique_table.size())
			return 1;
		return 2 * unique_table.size() + 1u - var;
	}

	node union_(node f, node g)
	{
		if (f == 0)
			return g;
		if (g == 0)
			return f;
		if (f == g)
			return f;
		if (f > g)
			std::swap(f, g);

		const auto it = compute_table.find({f, g, zdd_union});
		if (it != compute_table.end()) {
			assert(!nodes[it->second].dead);
			return it->second;
		}

		auto const& F = nodes[f];
		auto const& G = nodes[g];

		node r_lo, r_hi;

		if (F.var < G.var) {
			r_lo = union_(F.lo, g);
			r_hi = F.hi;
		} else if (F.var > G.var) {
			r_lo = union_(f, G.lo);
			r_hi = G.hi;
		} else {
			r_lo = union_(F.lo, G.lo);
			r_hi = union_(F.hi, G.hi);
		}

		const auto var = std::min(F.var, G.var);
		return compute_table[{f, g, zdd_union}] = unique(var, r_lo, r_hi);
	}

	node intersection(node f, node g)
	{
		if (f == 0)
			return 0;
		if (g == 0)
			return 0;
		if (f == g)
			return f;
		if (f > g)
			std::swap(f, g);

		auto const& F = nodes[f];
		auto const& G = nodes[g];

		if (F.var < G.var) {
			return intersection(F.lo, g);
		} else if (F.var > G.var) {
			return intersection(f, G.lo);
		}

		const auto it = compute_table.find({f, g, zdd_intersection});
		if (it != compute_table.end()) {
			assert(!nodes[it->second].dead);
			return it->second;
		}

		const auto r_lo = intersection(F.lo, G.lo);
		const auto r_hi = intersection(F.hi, G.hi);
		return compute_table[{f, g, zdd_intersection}] = unique(F.var, r_lo, r_hi);
	}

	node difference(node f, node g)
	{
		if (f == 0)
			return 0;
		if (f == g)
			return 0;
		if (g == 0)
			return f;

		auto const& F = nodes[f];
		auto const& G = nodes[g];

		if (G.var < F.var)
			return difference(f, G.lo);

		const auto it = compute_table.find({f, g, zdd_difference});
		if (it != compute_table.end()) {
			assert(!nodes[it->second].dead);
			return it->second;
		}

		node r_lo, r_hi;
		if (F.var == G.var) {
			r_lo = difference(F.lo, G.lo);
			r_hi = difference(F.hi, G.hi);
		} else {
			r_lo = difference(F.lo, g);
			r_hi = F.hi;
		}
		return compute_table[{f, g, zdd_difference}] = unique(F.var, r_lo, r_hi);
	}

	node join(node f, node g)
	{
		if (f == 0)
			return 0;
		if (g == 0)
			return 0;
		if (f == 1)
			return g;
		if (g == 1)
			return f;
		if (f > g)
			std::swap(f, g);

		const auto it = compute_table.find({f, g, zdd_join});
		if (it != compute_table.end()) {
			assert(!nodes[it->second].dead);
			return it->second;
		}

		auto const& F = nodes[f];
		auto const& G = nodes[g];

		node r_lo, r_hi;

		if (F.var < G.var) {
			r_lo = join(F.lo, g);
			r_hi = join(F.hi, g);
		} else if (F.var > G.var) {
			r_lo = join(f, G.lo);
			r_hi = join(f, G.hi);
		} else {
			r_lo = join(F.lo, G.lo);
			const auto r_lh = join(F.lo, G.hi);
			const auto r_hl = join(F.hi, G.lo);
			const auto r_hh = join(F.hi, G.hi);
			r_hi = union_(r_lh, union_(r_hl, r_hh));
		}

		const auto var = std::min(F.var, G.var);
		return compute_table[{f, g, zdd_join}] = unique(var, r_lo, r_hi);
	}

	node nonsupersets(node f, node g)
	{
		if (g == 0)
			return f;
		if (f == 0)
			return 0;
		if (g == 1)
			return 0;
		if (f == g)
			return 0;

		auto const& F = nodes[f];
		auto const& G = nodes[g];

		if (F.var > G.var)
			return nonsupersets(f, G.lo);

		const auto it = compute_table.find({f, g, zdd_nonsupersets});
		if (it != compute_table.end()) {
			assert(!nodes[it->second].dead);
			return it->second;
		}

		node r_lo, r_hi;
		if (F.var < G.var) {
			r_lo = nonsupersets(F.lo, g);
			r_hi = nonsupersets(F.hi, g);
		} else {
			r_hi = intersection(nonsupersets(F.hi, G.lo), nonsupersets(F.hi, G.hi));
			r_lo = nonsupersets(F.lo, G.lo);
		}

		return compute_table[{f, g, zdd_nonsupersets}] = unique(F.var, r_lo, r_hi);
	}

	node edivide(node f, node g)
	{
		auto const& F = nodes[f];
		auto const& G = nodes[g];

		if (F.var == G.var) {
			return F.hi;
		}

		if (F.var > G.var) {
			return 0;
		}

		const auto it = compute_table.find({f, g, zdd_edivide});
		if (it != compute_table.end()) {
			assert(!nodes[it->second].dead);
			return it->second;
		}

		node r_lo = edivide(F.lo, g);
		node r_hi = edivide(F.hi, g);
		return compute_table[{f, g, zdd_edivide}] = unique(F.var, r_lo, r_hi);
	}

	node sym(node f, uint32_t v, uint32_t k)
	{
		while (nodes[f].var < v) {
			f = nodes[f].lo;
		}

		if (f <= 1)
			return k > 0 ? 0 : tautology(v);

		const auto it = compute3_table.find({f, v, k, zdd_sym});
		if (it != compute3_table.end()) {
			assert(!nodes[it->second].dead);
			return it->second;
		}

		auto const& F = nodes[f];
		auto r = sym(f, F.var + 1, k);
		if (k > 0) {
			auto q = sym(F.lo, F.var + 1, k - 1);
			r = unique(F.var, r, q);
		}

		auto var = F.var;
		while (var > v) {
			r = unique(--var, r, r);
		}
		return compute3_table[{f, v, k, zdd_sym}] = r;
	}

	node choose(node f, uint32_t k)
	{
		if (k == 1)
			return f;
		if (f <= 1)
			return k > 0 ? 0 : 1;

		const auto it = compute_table.find({f, k, zdd_choose});
		if (it != compute_table.end()) {
			assert(!nodes[it->second].dead);
			return it->second;
		}

		auto const& F = nodes[f];
		auto r = choose(F.lo, k);
		if (k > 0) {
			auto q = choose(F.lo, k - 1);
			r = unique(F.var, r, q);
		}

		return compute_table[{f, k, zdd_choose}] = r;
	}

public:
	uint64_t count_sets(node f) const
	{
		if (f <= 1)
			return f;

		std::unordered_map<node, uint64_t> visited;
		return count_sets_rec(f, visited);
	}

	uint64_t count_nodes(node f) const
	{
		if (f <= 1)
			return 0;

		std::unordered_set<node> visited;
		count_nodes_rec(f, visited);
		return visited.size();
	}

private:
	uint64_t count_sets_rec(node f, std::unordered_map<node, uint64_t>& visited) const
	{
		if (f <= 1)
			return f;
		const auto it = visited.find(f);
		if (it != visited.end()) {
			return it->second;
		}

		auto const& F = nodes[f];
		return visited[f] = count_sets_rec(F.lo, visited) + count_sets_rec(F.hi, visited);
	}

	void count_nodes_rec(node f, std::unordered_set<node>& visited) const
	{
		if (f <= 1 || visited.count(f))
			return;
		visited.insert(f);
		auto const& F = nodes[f];
		count_nodes_rec(F.lo, visited);
		count_nodes_rec(F.hi, visited);
	}

public:
	void ref(node f)
	{
		if (f > 1) {
			nodes[f].ref++;
		}
	}

	void deref(node f)
	{
		if (f > 1 && nodes[f].ref > 0) {
			nodes[f].ref--;
		}
	}

	std::size_t num_nodes() const
	{
		return nodes.size() - 2 - free.size();
	}

	void garbage_collect()
	{
		std::vector<node> to_delete;
		/* skip terminals and elementary nodes */
		for (auto it = nodes.begin() + unique_table.size() + 2; it != nodes.end(); ++it) {
			if (it->ref == 0 && it->dead == 0) {
				to_delete.push_back(std::distance(nodes.begin(), it));
			}
		}
		for (auto f : to_delete) {
			kill_node(f);
			auto& n = nodes[f];
			garbage_collect_rec(n.lo);
			garbage_collect_rec(n.hi);
		}

		/* remove n from compute table */
		for (auto it = compute_table.begin(); it != compute_table.end();) {
			if (nodes[it->second].dead || nodes[std::get<0>(it->first)].dead
			    || nodes[std::get<1>(it->first)].dead) {
				it = compute_table.erase(it);
			} else {
				++it;
			}
		}

		compute3_table.clear(); /* TODO: selective delete? */
	}

private:
	void garbage_collect_rec(node f)
	{
		if (f <= 1)
			return;
		auto& n = nodes[f];
		if (n.ref == 0 || n.dead == 1)
			return;
		if (--(n.ref) == 0) {
			kill_node(f);
			garbage_collect_rec(n.lo);
			garbage_collect_rec(n.hi);
		}
	}

	void kill_node(node f)
	{
		free.push(f);
		auto& n = nodes[f];
		n.dead = 1;

		/* remove n from unique table */
		const auto it = unique_table[n.var].find({(node) n.lo, (node) n.hi});
		assert(it != unique_table[n.var].end());
		assert(it->second == f);
		unique_table[n.var].erase(it);
	}

public:
	struct identity_format {
		constexpr uint32_t operator()(uint32_t v) const noexcept
		{
			return v;
		}
	};

	template<class Formatter = identity_format>
	void print_sets(node f, Formatter&& fmt = Formatter())
	{
		std::vector<uint32_t> set;
		print_sets_rec(f, set, fmt);
	}

	template<class Formatter = identity_format>
	void write_dot(std::ostream& os, Formatter&& fmt = Formatter())
	{
		os << "digraph {\n";
		os << "0[shape=rectangle,label=⊥];\n";
		os << "1[shape=rectangle,label=⊤];\n";

		for (auto const& t : unique_table) {
			std::stringstream rank;
			for (auto const& [_, n] : t) {
				auto const& N = nodes[n];
				if (N.dead)
					continue;
				os << n << "[shape=ellipse,label=\"" << fmt(N.var) << "\"];\n";
				os << n << " -> " << N.lo << "[style=dashed]\n";
				os << n << " -> " << N.hi << "\n";
				rank << ";" << n;
			}
			os << "{rank=same" << rank.str() << "}\n";
		}

		os << "}\n";
	}

private:
	template<class Formatter>
	void print_sets_rec(node f, std::vector<uint32_t>& set, Formatter&& fmt)
	{
		if (f == 1) {
			for (auto v : set) {
				std::cout << fmt(v) << " ";
			}
			std::cout << "\n";
		} else if (f != 0) {
			print_sets_rec(nodes[f].lo, set, fmt);
			auto set1 = set;
			set1.push_back(nodes[f].var);
			print_sets_rec(nodes[f].hi, set1, fmt);
		}
	}

public:
	void debug()
	{
		std::cout << "    i     VAR    LO    HI   REF  DEAD\n";
		int i{0};
		for (auto const& n : nodes) {
			std::cout << std::setw(5) << i++ << " : " << std::setw(5) << n.var << " "
			          << std::setw(5) << n.lo << " " << std::setw(5) << n.hi << " "
			          << std::setw(5) << n.ref << " " << std::setw(5) << n.dead << "\n";
		}
		summary();
	}

	void summary()
	{
		std::cout << "live nodes = " << num_nodes() << "   dead nodes = " << free.size()
		          << "\n";
	}

private: /* hash functions */
	struct unique_table_hash {
		std::size_t operator()(std::pair<uint32_t, uint32_t> const& p) const
		{
			return 12582917 * p.first + 4256249 * p.second;
		}
	};

	struct compute_table_hash {
		std::size_t operator()(std::tuple<uint32_t, uint32_t, op_t> const& p) const
		{
			return 12582917 * std::get<0>(p) + 4256249 * std::get<1>(p)
			       + 741457 * static_cast<uint32_t>(std::get<2>(p));
		}
	};

	struct compute3_table_hash {
		std::size_t operator()(std::tuple<uint32_t, uint32_t, uint32_t, op_t> const& p) const
		{
			return 18803 * std::get<0>(p) + 53777 * std::get<1>(p)
			       + 61231 * std::get<2>(p)
			       + 3571 * static_cast<uint32_t>(std::get<3>(p));
		}
	};

private:
	node unique(uint32_t var, node lo, node hi)
	{
		/* ZDD reduction rule */
		if (hi == 0) {
			return lo;
		}

		assert(nodes[lo].var > var);
		assert(nodes[hi].var > var);

		/* unique table lookup */
		const auto it = unique_table[var].find({lo, hi});
		if (it != unique_table[var].end()) {
			assert(!nodes[it->second].dead);
			return it->second;
		}

		/* create new node */
		node r;

		if (!free.empty()) {
			r = free.top();
			free.pop();
			nodes[r].ref = nodes[r].dead = 0;
			nodes[r] = {var, lo, hi};
		} else if (nodes.size() < nodes.capacity()) {
			r = nodes.size();
			nodes.emplace_back(var, lo, hi);
		} else {
			std::cerr << "[e] no more space for new nodes available\n";
			exit(1);
		}

		/* increase ref counts */
		if (lo > 1) {
			nodes[lo].ref++;
		}
		if (hi > 1) {
			nodes[hi].ref++;
		}

		return unique_table[var][{lo, hi}] = r;
	}

private:
	std::vector<node_t> nodes;
	std::stack<node> free;
	std::vector<std::unordered_map<std::pair<uint32_t, uint32_t>, node, unique_table_hash>> unique_table;
	std::unordered_map<std::tuple<uint32_t, uint32_t, op_t>, node, compute_table_hash> compute_table;
	std::unordered_map<std::tuple<uint32_t, uint32_t, uint32_t, op_t>, node, compute3_table_hash>
	    compute3_table;
};
#pragma endregion

namespace detail {

template<typename Ntk>
class find_maximal_partitions_impl {
public:
	find_maximal_partitions_impl(Ntk const& circ, device_t const& arch)
	    : circ_(circ)
	    , arch_(arch)
	    , zdd_(circ.num_qubits() * arch.num_vertices, 19)
	    , from_(circ.num_qubits())
	    , to_(arch.num_vertices)
			, edge_perm_(arch.num_vertices, 0)
	    , fmt_(circ.num_qubits())
	{
		std::iota(edge_perm_.begin(), edge_perm_.end(), 0);
	}

	void run()
	{
		// zdd_.build_tautologies();
		init_from();
		init_to();
		init_valid();
		zdd_.garbage_collect();
		init_bad();
		//for (auto& t : to_)
		//	zdd_.deref(t);
		zdd_.garbage_collect();

		std::vector<zdd_base::node> mappings;
		uint32_t c, t;
		auto m = zdd_.bot();
		
        //counts the gates
        uint32_t ctr = 0;
        //vector that holds gate number that swap is needed for
        std::vector<uint32_t> index_of_swap;
        //vector that holds the qubits implemented swaps
        std::vector<std::vector<uint32_t>> swapped_qubits;
        
        
        
        
        
		circ_.foreach_cgate([&](auto const& n) {
			if (n.gate.is_double_qubit()) {
				n.gate.foreach_control([&](auto _c) { c = _c; });
				n.gate.foreach_target([&](auto _t) { t = _t; });

				// std::cout << c << " " << t << "\n";


				//std::cout << "add gate " << ctr << "\n";

				if (m == zdd_.bot())
                {
					/* first gate */
					m = map(c, t);
				}
                else
                {
					auto m_next = map(c, t);
					if (auto mp = zdd_.nonsupersets(zdd_.join(m, m_next), bad_); mp == zdd_.bot())
                    {

                        
                        std::vector<uint32_t> new_mappings_cnt(circ_.num_qubits(),0);
                        
                        for(uint32_t i = 0; i < circ_.num_qubits(); i++)
                        {
                            std::swap(edge_perm_[i], edge_perm_[(i+1)%circ_.num_qubits()]);
                            zdd_.deref(valid_);
                            init_valid();
                            //std::cout << "valid after:\n";
                            //zdd_.print_sets(valid_, fmt_);
                            
                            auto m_next = map(c, t);
                            if (auto mp = zdd_.nonsupersets(zdd_.join(m, m_next), bad_); mp == zdd_.bot())
                            {
                                //cannot extend map
                                new_mappings_cnt[i] = 0;
                                //unswap physical qubits for next iteration
                                std::swap(edge_perm_[i], edge_perm_[(i+1)%circ_.num_qubits()]);
                                zdd_.deref(valid_);
                                init_valid();
                                continue;
                            
                                
                            }
                            else
                            {
                                //can extend map. determine number of mappings and move on to next possible swap
                                //make sure to swap qubits back before next iteration
                                new_mappings_cnt[i] = zdd_.count_sets(mp);
                                std::swap(edge_perm_[i], edge_perm_[(i+1)%circ_.num_qubits()]);
                                zdd_.deref(valid_);
                                init_valid();
                                continue;

                                
                            }
                            
                        }
                        uint32_t max_index = std::max_element(new_mappings_cnt.begin(), new_mappings_cnt.end())-new_mappings_cnt.begin();
                        
                        
                        uint32_t max_mappings = new_mappings_cnt[max_index];
                        
//                        for (auto const& thing : new_mappings_cnt)
//                        {
//                            std::cout << thing << "\n";
//                        }
//                        std::cout <<"max index: "<< max_index  << "\n";
                        
                        if (max_mappings == 0)
                        {
                            std::cout << "A SWAP operation could not be found. Map cannot extend. Exiting...\n";
                            std::cout << "Metrics before exit:\n";
                            std::cout << "\nTotal SWAPs: " << swapped_qubits.size() << "\n";
                            for(uint32_t i = 0; i<swapped_qubits.size(); i++ )
                            {
                                std::cout << "Swap at gate: " <<index_of_swap[i] <<" | Physical qubits swapped: " << std::string(1, 'A' + swapped_qubits[i][0])<< " " <<std::string(1, 'A' + swapped_qubits[i][1])<< "\n";
                                
                            }
                            std::exit(0);
                            //return;
                        }
                        else
                        {
                  
                            std::swap(edge_perm_[max_index], edge_perm_[(max_index+1)%circ_.num_qubits()]);
                            zdd_.deref(valid_);
                            init_valid();
                            
                            auto m_next = map(c, t);
                            //std::cout << c << " " << t << "\n";
                            mp = zdd_.nonsupersets(zdd_.join(m, m_next), bad_);
                            m = mp;
                            index_of_swap.push_back(ctr);
                            
                            std::vector<uint32_t> one_swap;
                            one_swap.push_back(max_index);
                            one_swap.push_back((max_index+1)%circ_.num_qubits());
                            swapped_qubits.push_back(one_swap);
                        }

                        
					}
                    else
                    {
						m = mp;
					}
				}
				++ctr;
			}
		});
		zdd_.ref(m);
		mappings.push_back(m);
        
        std::cout << "\nTotal SWAPs: " << swapped_qubits.size() << "\n";
        for(uint32_t i = 0; i<swapped_qubits.size(); i++ )
        {
            //std::cout << "Swap at gate: " <<index_of_swap[i] <<" | Physical qubits swapped: " << swapped_qubits[i][0]<< " " <<
            //swapped_qubits[i][1] << "\n";
            std::cout << "Swap at gate: " <<index_of_swap[i] <<" | Physical qubits swapped: " << std::string(1, 'A' + swapped_qubits[i][0])<< " " <<std::string(1, 'A' + swapped_qubits[i][1])<< "\n";
            
        }

		uint32_t total{0};
        std::cout<< "\n";
		for (auto const& map : mappings) {
			std::cout << "found mapping with " << zdd_.count_sets(map)
			          << " mappings using " << zdd_.count_nodes(map) << " nodes.\n";
			total += zdd_.count_sets(map);
      
            //below prints the found mappings in partition
            std::cout << "\nfound sets: \n";
            zdd_.print_sets(map, fmt_);
            std::cout << "\n";
            
			zdd_.deref(map);
		}
		zdd_.summary();
		std::cout << "Total mappings: " << total << "\n";

		zdd_.deref(valid_);
		zdd_.deref(bad_);
		for (auto& f : from_)
			zdd_.deref(f);
		zdd_.garbage_collect();
		// zdd_.debug();
	}

private:
	auto index(uint32_t v, uint32_t p) const
	{
		// return p * circ_.num_qubits() + v;
		return v * arch_.num_vertices + p;
	}

	void init_from()
	{
		for (auto v = 0u; v < circ_.num_qubits(); ++v) {
			auto set = zdd_.bot();
			for (int p = arch_.num_vertices - 1; p >= 0; --p) {
				set = zdd_.union_(set, zdd_.elementary(index(v, p)));
			}
			from_[v] = set;
			zdd_.ref(set);
		}
	}

	void init_to()
	{
		for (auto p = 0u; p < arch_.num_vertices; ++p) {
			auto set = zdd_.bot();
			for (int v = circ_.num_qubits() - 1; v >= 0; --v) {
				set = zdd_.union_(set, zdd_.elementary(index(v, p)));
			}
			to_[p] = set;
			zdd_.ref(set);
		}
	}

	void init_valid()
	{
		valid_ = zdd_.bot();
		for (auto const& [p, q] : arch_.edges) {
			valid_ = zdd_.union_(valid_, zdd_.join(to_[edge_perm_[p]], to_[edge_perm_[q]]));
		}
		zdd_.ref(valid_);
	}

	void init_bad()
	{
		bad_ = zdd_.bot();
		for (int v = circ_.num_qubits() - 1; v >= 0; --v) {
			bad_ = zdd_.union_(bad_, zdd_.choose(from_[v], 2));
		}
		for (int p = arch_.num_vertices - 1; p >= 0; --p) {
			bad_ = zdd_.union_(bad_, zdd_.choose(to_[p], 2));
		}
		zdd_.ref(bad_);
	}

	zdd_base::node map(uint32_t c, uint32_t t)
	{
		return zdd_.intersection(zdd_.join(from_[c], from_[t]), valid_);
	}

private:
	Ntk const& circ_;
	device_t const& arch_;

	zdd_base zdd_;
	std::vector<zdd_base::node> from_, to_;
	zdd_base::node valid_, bad_;
	std::vector<uint32_t> edge_perm_;

	struct set_formatter_t {
		set_formatter_t(uint32_t n)
		    : n(n)
		{}
		std::string operator()(uint32_t v) const
		{
			//return std::string(1, 'a' + (v % n)) + std::string(1, 'A' + (v / n));
			return std::string(1, 'a' + (v / n)) + std::string(1, 'A' + (v % n));
		}

	private:
		uint32_t n;
	} fmt_;
};

} // namespace detail

template<typename Ntk>
void find_maximal_partitions(Ntk const& circ, device_t const& arch)
{
#if 0
	zdd_base zdd_map(arch.edges.size());
	zdd_map.build_tautologies();

	struct edge_formatter_t {
		edge_formatter_t(std::vector<std::pair<uint8_t, uint8_t>> const& edges)
		    : edges(edges)
		{}

		std::string operator()(uint32_t v) const
		{
			return std::string(1, 'A' + edges[v].first)
			       + std::string(1, 'A' + edges[v].second) + "(" + std::to_string(v) + ")";
		}

	private:
		std::vector<std::pair<uint8_t, uint8_t>> const& edges;
	} fmt(arch.edges);

	std::vector<std::vector<uint8_t>> incidents(arch.num_vertices);
	for (auto i = 0u; i < arch.edges.size(); ++i) {
		incidents[arch.edges[i].first].push_back(i);
		incidents[arch.edges[i].second].push_back(i);
	}

	std::vector<zdd_base::node> from;
	for (auto& i : incidents) {
		std::sort(i.rbegin(), i.rend());
		auto set = zdd_map.bot();
		for (auto o : i) {
			set = zdd_map.union_(set, zdd_map.elementary(o));
		}
		from.push_back(set);
		std::cout << "Sets = \n";
		zdd_map.print_sets(from.back(), fmt);
	}

  auto set = zdd_map.bot();
  for (auto const& f : from) {
    std::cout << "CHOOSE\n";
    zdd_map.print_sets(zdd_map.choose(f, 2), fmt);
    set = zdd_map.union_(set, zdd_map.choose(f, 2));
  }

  std::cout << "A!\n";
  zdd_map.print_sets(set, fmt);

  auto legal = zdd_map.nonsupersets(zdd_map.tautology(), set);
  std::cout << "B!\n";
  zdd_map.print_sets(legal, fmt);

	// zdd_map.print_sets(zdd_map.tautology());
	return;
#endif

	detail::find_maximal_partitions_impl<Ntk> impl(circ, arch);
	impl.run();

	//zdd_base z(4);

	//z.debug();
}

} // namespace tweedledum
